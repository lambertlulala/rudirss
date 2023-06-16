#include "RSSFeedEntry.h"

RSSFeedEntry::RSSFeedEntry()
{

}

RSSFeedEntry::RSSFeedEntry(const RSSFeedEntry& rhs) : m_childElements{ rhs.m_childElements }
{

}

RSSFeedEntry::RSSFeedEntry(RSSFeedEntry&& rhs) noexcept: m_childElements(std::move(rhs.m_childElements))
{

}

RSSFeedEntry& RSSFeedEntry::operator=(const RSSFeedEntry& rhs)
{
    if (this == &rhs)
        return *this;

    m_childElements = rhs.m_childElements;
    return *this;
}

RSSFeedEntry::~RSSFeedEntry()
{

}

std::wstring RSSFeedEntry::GetTitle() const
{
    return GetValue(L"title");
}

std::wstring RSSFeedEntry::GetLink() const
{
    return GetValue(L"link");
}

std::wstring RSSFeedEntry::GetDescription() const
{
    return GetValue(L"description");
}

std::wstring RSSFeedEntry::GetAuthor() const
{
    return GetValue(L"author");
}

std::wstring RSSFeedEntry::GetID() const
{
    return GetValue(L"id");
}

std::wstring RSSFeedEntry::GetUpdated() const
{
    return GetValue(L"updated");
}

void RSSFeedEntry::SetValue(const std::wstring& name, const std::wstring& value)
{
    m_childElements[name] = value;
}

std::wstring RSSFeedEntry::GetValue(const std::wstring& name) const 
{
    auto it = m_childElements.find(name);
    if (m_childElements.end() != it)
        return it->second;

    return {};

}
