#pragma once

#include "Feed.h"
#include <map>

class AtomFeedEntry : public Feed
{
protected:
    std::map<std::wstring, std::wstring> m_childElements;

public:
    AtomFeedEntry();
    AtomFeedEntry(const AtomFeedEntry &rhs);
    AtomFeedEntry(AtomFeedEntry &&rhs) noexcept;
    AtomFeedEntry&operator=(const AtomFeedEntry &rhs);
    virtual ~AtomFeedEntry();

    virtual std::wstring GetTitle() const;
    virtual std::wstring GetLink() const;
    virtual std::wstring GetDescription() const;
    virtual std::wstring GetAuthor() const;
    virtual std::wstring GetID() const;
    virtual std::wstring GetUpdated() const;
    virtual void SetValue(const std::wstring& name, const std::wstring& value);
    virtual std::wstring GetValue(const std::wstring& name) const;
};


