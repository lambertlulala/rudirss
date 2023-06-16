#pragma once

#include "WinHttpRequest.h"

#include <vector>
#include <functional>

class HttpGetRequest : public WinHttpRequest<HttpGetRequest>
{
public:
    using FN_ON_READ_COMPLETE = std::function<void(const void*, const char*, DWORD)>;
    using FN_ON_PROCESS_COMPLETE = std::function<void(const void*, bool, DWORD)>;
    using FN_ON_EXCEPTION = std::function<void(const void*, const WINHTTP_ASYNC_RESULT *)>;

protected:
    WinHttpConnection m_connection;
    bool m_result;
    DWORD m_statusCode;
    void* m_userParam;
    FN_ON_READ_COMPLETE m_onReadComplete;
    FN_ON_PROCESS_COMPLETE m_onProcessComplete;
    FN_ON_EXCEPTION m_onException;

    HRESULT GetStatusCode(DWORD& statusCode)
    {
        DWORD statusCodeSize = sizeof(DWORD);
        HRESULT result = S_OK;
        if (!::WinHttpQueryHeaders(m_handle,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &statusCode,
            &statusCodeSize,
            WINHTTP_NO_HEADER_INDEX))
        {
            result = HRESULT_FROM_WIN32(::GetLastError());
        }

        return result;
    }

public:
    HttpGetRequest() : m_result{ false }, m_statusCode{ 0 }, m_userParam{ nullptr }{ }
    HttpGetRequest(const HttpGetRequest& rhs) = delete;
    HttpGetRequest(HttpGetRequest&& rhs) noexcept :
        m_result{ rhs.m_result }, m_statusCode{ rhs.m_statusCode }, m_userParam{ rhs.m_userParam },
        m_onReadComplete{ rhs.m_onReadComplete }, m_onProcessComplete{ rhs.m_onProcessComplete },
        m_onException{ rhs.m_onException }
    {
        Close();
        if (rhs.m_handle)
        {
            m_handle = rhs.m_handle;
            rhs.m_handle = nullptr;
        }

        m_connection.Close();
        if (rhs.m_connection.m_handle)
        {
            m_connection.m_handle = rhs.m_connection.m_handle;
            rhs.m_connection.m_handle = nullptr;
        }
    }

    virtual ~HttpGetRequest() 
    {
        Disconnect();
    }

    virtual void Disconnect()
    {
        if (m_handle)
            WinHttpSetStatusCallback(m_handle, nullptr, 0, 0);

        Close();
        m_connection.Close(); // Close connection handle so that it's reusable next time.
    }

    virtual void OnException(const WINHTTP_ASYNC_RESULT* asyncResult)
    {
        if (m_onException)
            m_onException(m_userParam, asyncResult);
    }

    virtual void OnReadComplete(const void* buffer, DWORD bytesRead)
    {
        if (m_onReadComplete)
            m_onReadComplete(m_userParam, reinterpret_cast<const char*>(buffer), bytesRead);
    }

    virtual void OnProcessComplete()
    {
        if (m_onProcessComplete)
            m_onProcessComplete(m_userParam, m_result, m_statusCode);
    }

    virtual void OnResponseComplete(HRESULT result)
    {
        Disconnect();
        m_result = S_OK == result;
        OnProcessComplete();
    }

    virtual HRESULT OnCallback(DWORD code, const void* info, DWORD length)
    {
        switch (code)
        {
        case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
        {
            if (!::WinHttpReceiveResponse(m_handle, 0))
            {
                return HRESULT_FROM_WIN32(::GetLastError());
            }

            break;
        }

        case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
        {
            m_statusCode = 0;
            auto result = GetStatusCode(m_statusCode);
            if (FAILED(result))
            {
                return result;
            }

            if (!::WinHttpReadData(m_handle, m_buffer.data(), m_buffer.size(), nullptr))
            {
                return HRESULT_FROM_WIN32(::GetLastError());
            }

            break;
        }

        case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
        {
            if (length > 0)
            {
                OnReadComplete(m_buffer.data(), length);

                if (!::WinHttpReadData(m_handle, m_buffer.data(), m_buffer.size(), nullptr))
                {
                    return HRESULT_FROM_WIN32(::GetLastError());
                }
            }
            else
            {
                OnResponseComplete(S_OK);
            }

            break;
        }

        case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
        {
            OnException(static_cast<WINHTTP_ASYNC_RESULT*>(const_cast<void*>(info)));
            return E_FAIL;
        }

        case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
        {
            break;
        }

        default:
            break;
        }

        return S_OK;
    }

    virtual bool Initialize(const std::wstring& source, const WinHttpSession& session)
    {
        HRESULT result = E_FAIL;
        do
        {
            WCHAR host[BUFSIZ]{};
            URL_COMPONENTS urlComponents{};
            urlComponents.dwStructSize = sizeof(urlComponents);
            urlComponents.lpszHostName = host;
            urlComponents.dwHostNameLength = _countof(host);
            urlComponents.dwUrlPathLength = -1;
            if (!WinHttpCrackUrl(source.c_str(), 0, 0, &urlComponents))
            {
                result = HRESULT_FROM_WIN32(::GetLastError());
                break;
            }

            result = m_connection.Initialize(urlComponents.lpszHostName, urlComponents.nPort, session);
            if (FAILED(result))
                break;

            result = WinHttpRequest<HttpGetRequest>::Initialize(urlComponents.lpszUrlPath, nullptr, m_connection,
                nullptr, nullptr, nullptr, INTERNET_SCHEME_HTTPS == urlComponents.nScheme ? WINHTTP_FLAG_SECURE : 0);
            if (FAILED(result))
                break;

        } while (0);

        return SUCCEEDED(result);
    }

    virtual bool SendRequest(const void* userParam, FN_ON_READ_COMPLETE onReadComplete,
        FN_ON_PROCESS_COMPLETE onProcessComplete, FN_ON_EXCEPTION onException)
    {
        m_userParam = const_cast<void*>(userParam);
        m_onReadComplete = onReadComplete;
        m_onProcessComplete = onProcessComplete;
        m_onException = onException;
        return SUCCEEDED(WinHttpRequest<HttpGetRequest>::SendRequest(nullptr, 0, nullptr, 0, 0));
    }
};
