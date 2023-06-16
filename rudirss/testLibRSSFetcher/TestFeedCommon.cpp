#include "pch.h"
#include "FeedCommon.h"

#include <format>

using namespace FeedCommon;

TEST(TestFeedCommon, TestIterateSiblingElements)
{
    ASSERT_TRUE(SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)));

    {
        try
        {
            WCHAR dir[256]{};
            GetCurrentDirectory(_countof(dir), dir);

            WinMSXML xml;
            ASSERT_TRUE(xml.Init());
            xml.Load(std::format(L"{}\\..\\..\\testdata\\sample-rss.xml", dir));
            ASSERT_TRUE(true);

            auto elements = xml.GetElementsByTagName(L"channel");
            ASSERT_TRUE(elements);

            long length = 0;
            elements->get_length(&length);
            elements->reset();

            WinMSXML::XMLElement channel;
            ASSERT_TRUE(SUCCEEDED(elements->get_item(0, &channel)));

            WinMSXML::XMLElement childElement;
            ASSERT_TRUE(SUCCEEDED(channel->get_firstChild(&childElement)));

            IterateSiblingElements(childElement, [](const std::wstring_view& name, const std::wstring_view& value,
                const WinMSXML::XMLElement& element) -> bool {
                    std::wcout << std::format(L"name: {}, value: {}\n", name, value);
                    return true;
                });
            ASSERT_TRUE(true);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
            ASSERT_FALSE(true);
        }
    }

    CoUninitialize();
}

TEST(TestFeedCommon, TestGetFeedSpecification)
{
    ASSERT_TRUE(SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)));

    {
        try
        {
            WCHAR dir[256]{};
            GetCurrentDirectory(_countof(dir), dir);

            WinMSXML xml;
            ASSERT_TRUE(xml.Init());
            xml.Load(std::format(L"{}\\..\\..\\testdata\\sample-rss.xml", dir));
            auto spec = GetFeedSpecification(xml);
            ASSERT_TRUE(FeedSpecification::RSS == spec);
            xml.Load(std::format(L"{}\\..\\..\\testdata\\hacker_news.xml", dir));
            spec = GetFeedSpecification(xml);
            ASSERT_TRUE(FeedSpecification::RSS == spec);
            xml.Load(std::format(L"{}\\..\\..\\testdata\\planet_emacs.xml", dir));
            spec = GetFeedSpecification(xml);
            ASSERT_TRUE(FeedSpecification::Atom == spec);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
            ASSERT_FALSE(true);
        }
    }

    CoUninitialize();
}
