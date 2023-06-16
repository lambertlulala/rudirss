#pragma once

#include "Feed.h"
#include <map>

class RSSFeedEntry : public Feed
{
protected:
    std::map<std::wstring, std::wstring> m_childElements;

public:
    RSSFeedEntry();
    RSSFeedEntry(const RSSFeedEntry &rhs);
    RSSFeedEntry(RSSFeedEntry &&rhs) noexcept;
    RSSFeedEntry &operator=(const RSSFeedEntry &rhs);
    virtual ~RSSFeedEntry();

    virtual std::wstring GetTitle() const;
    virtual std::wstring GetLink() const;
    virtual std::wstring GetDescription() const;
    virtual std::wstring GetAuthor() const;
    virtual std::wstring GetID() const;
    virtual std::wstring GetUpdated() const;
    virtual void SetValue(const std::wstring& name, const std::wstring& value);
    virtual std::wstring GetValue(const std::wstring& name) const ;
};

