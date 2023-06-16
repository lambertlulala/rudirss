#include "RSSFeed.h"
#include "libmsxml.h"
#include "FeedCommon.h"

#include <stdexcept>

using namespace FeedCommon;

RSSFeed::RSSFeed()
{

}

RSSFeed::~RSSFeed()
{

}

void RSSFeed::Parse(WinMSXML& xml)
{
    auto xmlDoc = xml.GetXMLDocument();
    WinMSXML::XMLElement firstChild;
    if (FAILED(xmlDoc->get_firstChild(&firstChild)) || !firstChild)
        throw std::runtime_error("Error: unable to get first child of root.");

    WinMSXML::XMLElement rss;
    IterateSiblingElements(firstChild, [&](const std::wstring_view& name, const std::wstring_view& value,
        const WinMSXML::XMLElement& element) -> bool {
            if (L"rss" == name)
            {
                rss = element;
                return false;
            }

            return true;
        });

    if (!rss)
        throw std::runtime_error("Error: unable to get rss element.");
    m_version = xml.GetAttributeValue(rss, L"version");

    WinMSXML::XMLElement rssFirstChild;
    if (FAILED(rss->get_firstChild(&rssFirstChild)) || !rssFirstChild)
        throw std::runtime_error("Error: unable to get first child of rss.");

    WinMSXML::XMLElement channel;
    IterateSiblingElements(rssFirstChild, [&](const std::wstring_view& name, const std::wstring_view& value,
        const WinMSXML::XMLElement& element) -> bool {
            if (L"channel" == name)
            {
                channel = element;
                return false;
            }

            return true;
        });
    if (!channel)
        throw std::runtime_error("Error: unable to get channel element.");

    WinMSXML::XMLElement channelFirstChild;
    if (FAILED(channel->get_firstChild(&channelFirstChild)) || !channelFirstChild)
        throw std::runtime_error("Error: unable to get first child of channel.");

    IterateSiblingElements(channelFirstChild, [&](const std::wstring_view& name, const std::wstring_view& value,
        const WinMSXML::XMLElement& element) -> bool {
            if (L"item" != name)
            {
                SetValue(std::wstring(name), std::wstring(value));
            }
            else
            {
                WinMSXML::XMLElement firstChild;
                if (SUCCEEDED(element->get_firstChild(&firstChild)))
                {
                    RSSFeedEntry feedEntry = CreateItemElement(firstChild);
                    m_entries.push_back(std::move(feedEntry));
                }
            }

            return true;
        });
}

RSSFeedEntry RSSFeed::CreateItemElement(WinMSXML::XMLElement& element)
{
    RSSFeedEntry feedEntry;
    IterateSiblingElements(element, [&](const std::wstring_view& name, const std::wstring_view& value,
        const WinMSXML::XMLElement& element) -> bool {
            feedEntry.SetValue(std::wstring(name), std::wstring(value));
            return true;
        });

    return feedEntry;
}

void RSSFeed::ParseFromFile(const std::wstring& file)
{
    WinMSXML xml;
    xml.Init();
    xml.Load(file);
    Parse(xml);
}

void RSSFeed::ParseFromString(const std::wstring& xmlString)
{
    if (xmlString.empty())
        throw std::runtime_error("Error: XML string is empty.");

    WinMSXML xml;
    xml.Init();
    xml.LoadFromString(xmlString);
    Parse(xml);
}

void RSSFeed::IterateEntries(FN_ON_ITERATE_ENTRY onIterateEntry)
{
    if (!onIterateEntry)
        throw std::runtime_error("Error: invalid FN_ON_ITERATE_ENTRY callback function.");

    for (const auto& entry : m_entries)
    {
        onIterateEntry(entry);
    }
}

void RSSFeed::ClearEntries()
{
    m_version.clear();
    m_entries.clear();
}
