#pragma once

#include <string>

class Feed
{
public:
    virtual std::wstring GetTitle() const = 0;
    virtual std::wstring GetLink() const = 0;
    virtual std::wstring GetDescription() const = 0;
    virtual std::wstring GetAuthor() const = 0;
    virtual std::wstring GetID() const = 0;
    virtual std::wstring GetUpdated() const = 0;
    virtual void SetValue(const std::wstring& name, const std::wstring& value) = 0;
    virtual std::wstring GetValue(const std::wstring& name) const = 0;
};
