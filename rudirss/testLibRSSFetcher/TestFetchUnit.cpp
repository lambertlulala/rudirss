#include "pch.h"

#include "FetchUnit.h"

#include <Windows.h>
#include <vector>

class TestFakeFetchUnit : public FetchUnit
{
public:
    HANDLE m_waitEvent;

    TestFakeFetchUnit() : m_waitEvent{ nullptr } {}
    virtual ~TestFakeFetchUnit()
    {
        if (m_waitEvent)
            CloseHandle(m_waitEvent);
    }
};

TEST(TestFetchUnit, TestFetchFeed)
{
    WinHttpSession session;
    ASSERT_TRUE(SUCCEEDED(session.Initialize(L"Testing Agent")));

    TestFakeFetchUnit testFetchUnit;
    testFetchUnit.m_waitEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    ASSERT_TRUE(testFetchUnit.m_waitEvent);
    DWORD responseStatusCode = 0;
    DWORD error = 0;
    try
    {
        testFetchUnit.Fetch(nullptr, session, L"https://www.makeuseof.com/feed",
            [&](const void *userParam, bool result, DWORD statusCode, const char* data, size_t size, const WINHTTP_ASYNC_RESULT* asyncResult) {
                responseStatusCode = statusCode;
                if (asyncResult)
                    error = asyncResult->dwError;
                SetEvent(testFetchUnit.m_waitEvent);
            });
    }
    catch (const std::exception& e)
    {
        ASSERT_FALSE(true);
    }

    WaitForSingleObject(testFetchUnit.m_waitEvent, INFINITE);
    EXPECT_TRUE(HTTP_STATUS_OK == responseStatusCode);
}

TEST(TestFetchUnit, TestFetchFeeds)
{
    WinHttpSession session;
    ASSERT_TRUE(SUCCEEDED(session.Initialize(L"Testing Agent")));

    TestFakeFetchUnit testFetchUnit;
    testFetchUnit.m_waitEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    ASSERT_TRUE(testFetchUnit.m_waitEvent);

    std::vector<std::wstring> feeds = { L"https://www.majorgeeks.com/files/rss", L"https://www.geeksforgeeks.org//feed",
        L"https://news.ycombinator.com/rss", L"https://planet.emacslife.com/atom.xml", L"https://totalcmd.net/rss.xml"};
    DWORD responseStatusCode = 0;
    DWORD error = 0;
    try
    {
        for (const auto &feed: feeds)
        {
            testFetchUnit.Fetch(nullptr, session, feed,
                [&](const void *userParam, bool result, DWORD statusCode, const char* data, size_t size, const WINHTTP_ASYNC_RESULT* asyncResult) {
                    responseStatusCode = statusCode;
                    if (asyncResult)
                        error = asyncResult->dwError;
                    SetEvent(testFetchUnit.m_waitEvent);
                });

            WaitForSingleObject(testFetchUnit.m_waitEvent, INFINITE);
            ResetEvent(testFetchUnit.m_waitEvent);
            EXPECT_TRUE(HTTP_STATUS_OK == responseStatusCode);
        }
    }
    catch (const std::exception& e)
    {
        ASSERT_FALSE(true);
    }
}

TEST(TestFetchUnit, TestFetchWithoutCallback)
{
    WinHttpSession session;
    ASSERT_TRUE(SUCCEEDED(session.Initialize(L"Testing Agent")));

    TestFakeFetchUnit testFetchUnit;
    try
    {
        testFetchUnit.Fetch(nullptr, session, L"https://www.makeuseof.com/feed", nullptr); 
        ASSERT_FALSE(true);
    }
    catch (const std::exception& e)
    {
        ASSERT_TRUE(true);
    }
}

TEST(TestFetchUnit, TestGetPageWithoutSecurity)
{
    WinHttpSession session;
    ASSERT_TRUE(SUCCEEDED(session.Initialize(L"Testing Agent")));

    TestFakeFetchUnit testFetchUnit;
    testFetchUnit.m_waitEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    ASSERT_TRUE(testFetchUnit.m_waitEvent);
    DWORD responseStatusCode = 0;
    DWORD error = 0;
    try
    {
        testFetchUnit.Fetch(nullptr, session, L"http://www.bing.com",
            [&](const void *userParam, bool result, DWORD statusCode, const char* data, size_t size, const WINHTTP_ASYNC_RESULT* asyncResult) {
                responseStatusCode = statusCode;
                if (asyncResult)
                    error = asyncResult->dwError;
                SetEvent(testFetchUnit.m_waitEvent);
            });
    }
    catch (const std::exception& e)
    {
        ASSERT_FALSE(true);
    }

    WaitForSingleObject(testFetchUnit.m_waitEvent, INFINITE);
    EXPECT_TRUE(HTTP_STATUS_OK == responseStatusCode);
}
