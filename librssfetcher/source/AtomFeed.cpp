#include "AtomFeed.h"
#include "libmsxml.h"
#include "FeedCommon.h"

#include <stdexcept>

using namespace FeedCommon;

AtomFeed::AtomFeed()
{

}

AtomFeed::~AtomFeed()
{

}

void AtomFeed::Parse(WinMSXML& xml)
{
    auto xmlDoc = xml.GetXMLDocument();
    WinMSXML::XMLElement firstChild;
    if (FAILED(xmlDoc->get_firstChild(&firstChild)) || !firstChild)
        throw std::runtime_error("Error: unable to get first child of root.");

    WinMSXML::XMLElement feed;
    IterateSiblingElements(firstChild, [&](const std::wstring_view& name, const std::wstring_view& value,
        const WinMSXML::XMLElement& element) -> bool {
            if (L"feed" == name)
            {
                feed = element;
                return false;
            }

            return true;
        });

    if (!feed)
        throw std::runtime_error("Error: unable to get feed element.");

    WinMSXML::XMLElement feedFirstChild;
    if (FAILED(feed->get_firstChild(&feedFirstChild)) || !feedFirstChild)
        throw std::runtime_error("Error: unable to get first child of rss.");

    IterateSiblingElements(feedFirstChild, [&](const std::wstring_view& name, const std::wstring_view& value,
        const WinMSXML::XMLElement& element) -> bool {
            if (L"entry" != name)
            {
                SetValue(std::wstring(name), std::wstring(value));
            }
            else
            {
                WinMSXML::XMLElement entryFirstChild;
                if (SUCCEEDED(element->get_firstChild(&entryFirstChild)))
                {
                    AtomFeedEntry feedEntry = CreateItemElement(entryFirstChild);
                    m_entries.push_back(std::move(feedEntry));
                }
            }

            return true;
        });
}

AtomFeedEntry AtomFeed::CreateItemElement(WinMSXML::XMLElement& element)
{
    AtomFeedEntry feedEntry;
    IterateSiblingElements(element, [&](const std::wstring_view& name, const std::wstring_view& value,
        const WinMSXML::XMLElement& element) -> bool {
            feedEntry.SetValue(std::wstring(name), std::wstring(value));
            return true;
        });

    return feedEntry;
}

void AtomFeed::ParseFromFile(const std::wstring& file)
{
    WinMSXML xml;
    xml.Init();
    xml.Load(file);
    Parse(xml);
}

void AtomFeed::ParseFromString(const std::wstring& xmlString)
{
    if (xmlString.empty())
        throw std::runtime_error("Error: XML string is empty.");

    WinMSXML xml;
    xml.Init();
    xml.LoadFromString(xmlString);
    Parse(xml);
}

void AtomFeed::IterateEntries(FN_ON_ITERATE_ENTRY onIterateEntry)
{
    if (!onIterateEntry)
        throw std::runtime_error("Error: invalid FN_ON_ITERATE_ENTRY callback function.");

    for (const auto& entry : m_entries)
    {
        onIterateEntry(entry);
    }
}

void AtomFeed::ClearEntries()
{
    m_entries.clear();
}
