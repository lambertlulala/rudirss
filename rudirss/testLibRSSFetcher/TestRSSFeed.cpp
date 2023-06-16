#include "pch.h"

#include "FeedCommon.h"
#include "RSSFeed.h"

#include <Windows.h>
#include <fstream>
#include <format>

using namespace FeedCommon;

TEST(TestRSSFeed, TestLoadSampleFeed)
{
    ASSERT_TRUE(SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)));

    {
        try
        {
            WCHAR dir[256]{};
            GetCurrentDirectory(_countof(dir), dir);

            RSSFeed rssFeed;
            rssFeed.ParseFromFile(std::format(L"{}\\..\\..\\testdata\\sample-rss.xml", dir));
            ASSERT_TRUE(true);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
            ASSERT_FALSE(true);
        }

        try
        {
            RSSFeed rssFeed;
            rssFeed.ParseFromFile(L"wrong_feed.xml");
            ASSERT_FALSE(true);
        }
        catch (const std::exception& e)
        {
            ASSERT_TRUE(true);
        }
    }

    CoUninitialize();
}

TEST(TestRSSFeed, TestLoadSampleFeedFromString)
{
    ASSERT_TRUE(SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)));

    {
        try
        {
            WCHAR dir[256]{};
            GetCurrentDirectory(_countof(dir), dir);
            std::wfstream f(std::format(L"{}\\..\\..\\testdata\\sample-rss.xml", dir), std::iostream::in);
            ASSERT_TRUE(f);

            f.seekg(f.beg, f.end);
            size_t size = f.tellg();
            f.seekg(0);
            std::wstring xmlString(size, 0);
            f.read(const_cast<wchar_t*>(xmlString.data()), size);

            RSSFeed rssFeed;
            rssFeed.ParseFromString(xmlString);
            auto version = rssFeed.GetVersion();
            ASSERT_TRUE(L"2.0" == version);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
            ASSERT_FALSE(false);
        }
    }

    CoUninitialize();
}

TEST(TestRSSFeed, TestParseFeeds)
{
    ASSERT_TRUE(SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)));

    {
        try
        {

            WCHAR dir[256]{};
            GetCurrentDirectory(_countof(dir), dir);
            std::wstring baseDir = std::format(L"{}\\..\\..\\testdata\\", dir);
            std::vector<std::wstring> feedFiles = { baseDir + L"major_geeks.xml", baseDir + L"sample-rss.xml",  baseDir + L"hacker_news.xml" };

            for (const auto& feedFile : feedFiles)
            {
                RSSFeed rssFeed;
                rssFeed.ParseFromFile(feedFile);

                std::wcout << std::format(L"Feed info -> title: {}, link: {}, description: {}\n\n",
                    rssFeed.GetTitle(), rssFeed.GetLink(), rssFeed.GetDescription());
                rssFeed.IterateEntries([](const Feed& feed) {
                    wprintf(L"title: %s\nid: %s\ndescription: %s\n\n",
                    feed.GetTitle().c_str(), feed.GetLink().c_str(), feed.GetDescription().c_str());
                    });
                ASSERT_TRUE(true);
            }
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
            ASSERT_FALSE(false);
        }
    }

    CoUninitialize();

}
