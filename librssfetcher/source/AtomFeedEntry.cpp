#include "AtomFeedEntry.h"

AtomFeedEntry::AtomFeedEntry()
{

}

AtomFeedEntry::AtomFeedEntry(const AtomFeedEntry& rhs) : m_childElements{ rhs.m_childElements }
{

}

AtomFeedEntry::AtomFeedEntry(AtomFeedEntry&& rhs) noexcept: m_childElements(std::move(rhs.m_childElements))
{

}

AtomFeedEntry& AtomFeedEntry::operator=(const AtomFeedEntry& rhs)
{
    if (this == &rhs)
        return *this;

    m_childElements = rhs.m_childElements;
    return *this;
}

AtomFeedEntry::~AtomFeedEntry()
{

}

std::wstring AtomFeedEntry::GetTitle() const
{
    return GetValue(L"title");
}

std::wstring AtomFeedEntry::GetLink() const
{
    return GetValue(L"link");
}

std::wstring AtomFeedEntry::GetDescription() const
{
    return GetValue(L"description");
}

std::wstring AtomFeedEntry::GetAuthor() const
{
    return GetValue(L"author");
}

std::wstring AtomFeedEntry::GetID() const
{
    return GetValue(L"id");
}

std::wstring AtomFeedEntry::GetUpdated() const
{
    return GetValue(L"updated");
}

void AtomFeedEntry::SetValue(const std::wstring& name, const std::wstring& value)
{
    m_childElements[name] = value;
}

std::wstring AtomFeedEntry::GetValue(const std::wstring& name) const
{
    auto it = m_childElements.find(name);
    if (m_childElements.end() != it)
        return it->second;

    return {};
}
