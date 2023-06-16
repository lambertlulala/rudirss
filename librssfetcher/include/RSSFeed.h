#pragma once

#include "RSSFeedEntry.h"
#include "FeedParser.h"
#include "libmsxml.h"

#include <vector>
#include <string>

class RSSFeed : public RSSFeedEntry, public FeedParser
{
protected:
    using ENTRIES = std::vector<RSSFeedEntry>;
    ENTRIES m_entries;
    std::wstring m_version;

    void Parse(WinMSXML &xml);
    RSSFeedEntry CreateItemElement(WinMSXML::XMLElement& element);

public:
    RSSFeed();
    virtual ~RSSFeed();

    virtual void ParseFromFile(const std::wstring &file);
    virtual void ParseFromString(const std::wstring &xmlString);
    virtual void IterateEntries(FN_ON_ITERATE_ENTRY onIterateEntry);
    virtual void ClearEntries();

    const std::wstring GetVersion() const { return m_version; }
};
