#pragma once

#include "libmsxml.h"

#include <map>
#include <string>
#include <functional>

namespace FeedCommon
{
    enum class FeedSpecification
    {
        RSS,
        Atom,
        None
    };

    using FN_ON_ITERATE_ELEMENT = std::function<bool(const std::wstring_view&, const std::wstring_view&,
        const WinMSXML::XMLElement& element)>;
    void IterateSiblingElements(const WinMSXML::XMLElement& element, FN_ON_ITERATE_ELEMENT onIterateElement);
    FeedSpecification GetFeedSpecification(const WinMSXML &xml);
};
