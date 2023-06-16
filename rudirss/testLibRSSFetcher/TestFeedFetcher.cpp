#include "pch.h"

#include "FeedFetcher.h"

#include <iostream>
#include <format>

TEST(TestFetchFeed, TestFetchFeeds)
{
    try
    {
        FeedFetcher feedFetcher(8);
        feedFetcher.Initialize([&](const FetchUnit* fetchUnit, bool result, DWORD statusCode, const char* data, size_t size,
            const WINHTTP_ASYNC_RESULT* asyncResult) {
                std::wcout << std::format(L"URL: {}, statusCode: {}\n", fetchUnit->FeedUrl(), statusCode);
            });
        feedFetcher.StartFetchRoutine();

        std::vector<std::wstring> feeds = { L"https://www.majorgeeks.com/files/rss", L"https://www.geeksforgeeks.org//feed",
            L"https://news.ycombinator.com/rss", L"https://planet.emacslife.com/atom.xml", L"https://totalcmd.net/rss.xml",
            L"https://www.ptt.cc/atom/Tech_Job.xml", L"https://www.ptt.cc/atom/Soft_Job.xml", L"https://www.ptt.cc/atom/careerplan.xml",
            L"https://www.ptt.cc/atom/salary.xml", L"https://www.ptt.cc/atom/Job.xml" };
        for (const auto& feed : feeds)
        {
            feedFetcher.PushFeedUrl(feed);
        }

        Sleep(1000);
        feedFetcher.StopFetchRoutine();
        EXPECT_TRUE(0 == feedFetcher.GetFeedUrlSize());
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        ASSERT_FALSE(true);
    }
}