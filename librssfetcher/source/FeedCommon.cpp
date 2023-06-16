#include "FeedCommon.h"

#include <stdexcept>

using namespace FeedCommon;

void FeedCommon::IterateSiblingElements(const WinMSXML::XMLElement& element, FN_ON_ITERATE_ELEMENT onIterateElement)
{
    if (!element)
        throw std::invalid_argument("Error: empty element.");

    if (!onIterateElement)
        throw std::invalid_argument("Error: invalid FN_ON_ITERATE_ELEMENT callback function.");

    HRESULT result = S_OK;
    WinMSXML::XMLElement elementIter = element;
    do
    {
        CComBSTR name;
        result = elementIter->get_baseName(&name);
        if (S_OK != result)
            break;

        CComBSTR text;
        result = elementIter->get_text(&text);
        if (S_OK != result)
            break;

        if (!onIterateElement(std::wstring_view(name), std::wstring_view(text), elementIter))
            break;

        WinMSXML::XMLElement tmp;
        result = elementIter->get_nextSibling(&tmp);
        if (S_OK != result)
            break;
        elementIter = tmp;
    } while (S_OK == result);
}

FeedSpecification FeedCommon::GetFeedSpecification(const WinMSXML &xml)
{
    auto xmlDoc = xml.GetXMLDocument();
    WinMSXML::XMLElement firstChild;
    if (FAILED(xmlDoc->get_firstChild(&firstChild)) || !firstChild)
        throw std::runtime_error("Error: unable to get first child of root.");

    WinMSXML::XMLElement feed;
    FeedSpecification spec = FeedSpecification::None;
    IterateSiblingElements(firstChild, [&](const std::wstring_view& name, const std::wstring_view& value,
        const WinMSXML::XMLElement& element) -> bool {
            if (L"rss" == name)
            {
                spec = FeedSpecification::RSS;
                return false;
            }
            else if (L"feed" == name)
            {
                spec = FeedSpecification::Atom;
                return false;
            }

            return true;
        });

    return spec;
}
