#pragma once

#include <string>
#include <functional>

class Feed;

class FeedParser
{
public:
    virtual void ParseFromFile(const std::wstring &file) = 0;
    virtual void ParseFromString(const std::wstring &xmlString) = 0;

    using FN_ON_ITERATE_ENTRY = std::function<void(const Feed& feed)>;
    virtual void IterateEntries(FN_ON_ITERATE_ENTRY onIterateEntry) = 0;
    virtual void ClearEntries() = 0;
};
