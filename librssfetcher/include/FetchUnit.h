#pragma once

#include "HttpGetRequest.h"

#include <Windows.h>
#include <string>
#include <vector>
#include <functional>

class FetchUnit: public HttpGetRequest
{
public:
    FetchUnit();
    FetchUnit(const FetchUnit& rhs) = delete;
    FetchUnit(FetchUnit&& rhs) noexcept;
    virtual ~FetchUnit();

    using FN_ON_FETCH_COMPLETE = std::function<void(const void *, bool, DWORD, const char *, size_t, const WINHTTP_ASYNC_RESULT *)>;
    void Fetch(const void *userParam, const WinHttpSession &session, const std::wstring &feedURL, FN_ON_FETCH_COMPLETE onFetchComplete);

    const std::wstring FeedUrl() const { return m_feedUrl; }
    HANDLE GetFetchCompleteEvent() const { return m_fetchCompleteEvent; }

private:
    std::wstring m_feedUrl;
    std::vector<char> m_data;
    FN_ON_FETCH_COMPLETE m_onFetchComplete;
    HANDLE m_fetchCompleteEvent;
};
