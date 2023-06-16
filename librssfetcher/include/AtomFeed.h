#pragma once

#include "AtomFeedEntry.h"
#include "FeedParser.h"
#include "libmsxml.h"

#include <vector>
#include <string>

class AtomFeed : public AtomFeedEntry, public FeedParser
{
protected:
    using ENTRIES = std::vector<AtomFeedEntry>;
    ENTRIES m_entries;

    void Parse(WinMSXML& xml);
    AtomFeedEntry CreateItemElement(WinMSXML::XMLElement& element);

public:
    AtomFeed();
    virtual ~AtomFeed();

    virtual void ParseFromFile(const std::wstring& file);
    virtual void ParseFromString(const std::wstring& xmlString);
    virtual void IterateEntries(FN_ON_ITERATE_ENTRY onIterateEntry);
    virtual void ClearEntries();
};
