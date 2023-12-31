#include "RudiRSSMainWindow.h"
#include "Resource.h"
#include "FeedBase.h"
#include "ListView.h"

#include <commdlg.h>
#include <CommCtrl.h>
#include <format>

RudiRSSMainWindow::RudiRSSMainWindow() : m_initViewer{ FALSE }, m_font{ nullptr }, m_boldFont{ nullptr },
m_feedListView{ this }, m_feedItemListView{ this }
{
}

RudiRSSMainWindow::~RudiRSSMainWindow()
{
}

void RudiRSSMainWindow::OnRegister(WNDCLASSEXW& wcex)
{
    WCHAR title[MAX_PATH]{};
    WCHAR windowClass[MAX_PATH]{};
    LoadStringW(m_hInstance, IDS_APP_TITLE, title, _countof(title));
    LoadStringW(m_hInstance, IDC_RUDIRSS, windowClass, _countof(windowClass));
    m_title = title;
    m_className = windowClass;

    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainWindow::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_hInstance;
    wcex.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_RUDIRSS));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_RUDIRSS);
    wcex.lpszClassName = m_className.c_str();
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_RUDIRSS));
}

HWND RudiRSSMainWindow::Create()
{
    HWND hWnd = CreateWindowW(m_className.c_str(), m_title.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, m_hInstance, nullptr);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    return hWnd;
}

bool RudiRSSMainWindow::Initialize(HINSTANCE hInstance)
{
    bool result = false;
    if (MainWindow::Initialize(hInstance))
    {
        InittializeControl();

        result = m_rudiRSSClient.Initialize();

        if (result)
        {
            m_feedListView.UpdateFeedListFromDatabase();

            m_rudiRSSClient.StartRefreshFeedTimer([&](const FeedDatabase::FeedConsumptionUnit& consumptionUnit) {
                if (FeedDatabase::FeedConsumptionUnit::OperationType::NOTIFY_INSERTION_COMPLETE == consumptionUnit.opType)
                {
                    ::RedrawWindow(m_feedListView.m_hWnd, nullptr, nullptr, RDW_INVALIDATE);

                    long long lastSelectedFeedId = m_feedListView.GetLastSelectedFeedId();
                    if (FeedListView::ALL_FEEDS_LIST_INDEX != m_feedListView.GetLastSelectedFeedIndex()
                        && lastSelectedFeedId == consumptionUnit.feed.feedid)
                    {
                        m_feedItemListView.UpdateSelectedFeed(lastSelectedFeedId);
                    }
                }
                });
        }
    }

    return result;
}

LRESULT RudiRSSMainWindow::OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        return OnCommand(hWnd, message, wParam, lParam);
    }
    break;

    case WM_NOTIFY:
    {
        return OnProcessListViewCommand(hWnd, message, wParam, lParam);
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_SIZE:
        UpdateControl();
        break;

    case WM_DESTROY:
        OnDestroy();
        PostQuitMessage(0);
        break;

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT RudiRSSMainWindow::OnCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto id = LOWORD(wParam);
    switch (id)
    {
    case ID_FEED_ITEM_MENU_MARK_AS_READ:
    case ID_FEED_ITEM_MENU_MARK_AS_UNREAD:
    {
        bool result = false;
        auto it = m_feedItemListView.GetRightClickedFeedDataIteratorFromCache(result);
        if (result)
        {
            long long read = static_cast<long long>(ID_FEED_ITEM_MENU_MARK_AS_READ == id);
            m_rudiRSSClient.UpdateFeedDataReadColumn(it->second.feeddataid, read);
            it->second.read = read;
            ListView_Update(m_feedItemListView.m_hWnd, it->first);
        }
    }
    break;

    case ID_FEED_ITEM_MENU_COPY_TITLE:
    {
        FeedDatabase::FeedData feedData;
        if (m_feedItemListView.GetRightClickedFeedDataFromCache(feedData))
        {
            std::wstring title;
            FeedCommon::ConvertStringToWideString(feedData.title, title);
            FeedCommon::CopyToClipboard(title);
        }
    }
    break;

    case ID_FEED_ITEM_MENU_COPY_LINK:
    {
        FeedDatabase::FeedData feedData;
        if (m_feedItemListView.GetRightClickedFeedDataFromCache(feedData))
        {
            std::wstring link;
            FeedCommon::ConvertStringToWideString(feedData.link, link);
            FeedCommon::CopyToClipboard(link);
        }
    }
    break;

    case ID_FEED_ITEM_MENU_OPEN_WITH_DEFAULT_BROWSER:
    {
        FeedDatabase::FeedData feedData;
        if (m_feedItemListView.GetRightClickedFeedDataFromCache(feedData))
        {
            std::wstring link;
            FeedCommon::ConvertStringToWideString(feedData.link, link);
            ShellExecute(nullptr, L"Open", link.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        }
    }
    break;

    case ID_FILE_IMPORT_FROM_OPML:
    {
        OpenImportOPMLDialog();
    }
    break;

    case ID_FILE_IMPORT_FROM_LIST_FILE:
    {
        OpenImportListFileDialog();
    }
    break;

    case ID_FEED_MENU_REFRESH:
    {
        int lastRightClickedItem = m_feedListView.GetLastRightClickedItem();
        if (FeedListView::ALL_FEEDS_LIST_INDEX != lastRightClickedItem)
        {
            m_rudiRSSClient.RefreshFeedByOffset(lastRightClickedItem - 1);
        }
    }
    break;

    case ID_FEED_MENU_REFRESH_ALL:
    {
        m_rudiRSSClient.RefreshAllFeeds();
    }
    break;

    case ID_FEED_MENU_DELETE_THIS_FEED:
    {
        bool result = false;
        auto it = m_feedListView.GetRightClickedFeedIteratorFromCache(result);
        if (result)
        {
            std::wstring title;
            FeedCommon::ConvertStringToWideString(it->second.title, title);

            if (IDYES == MessageBox(hWnd, std::format(L"Delete the feed '{}'?", title).c_str(), L"Delete the feed", MB_ICONWARNING | MB_YESNO))
            {
                std::wstring guid;
                FeedCommon::ConvertStringToWideString(it->second.guid, guid);
                m_rudiRSSClient.DeleteFeed(it->second.feedid, guid);
                m_feedListView.UpdateFeedListFromDatabase();
                long long cnt = 0;
                m_rudiRSSClient.QueryFeedTableCount(cnt);
                if (0 == cnt)
                {
                    m_feedListView.ResetLastSelectedFeedIndex();
                    m_feedItemListView.DeleteAllItems();
                }
            }
        }
    }
    break;

    case ID_FEED_MENU_DELETE_ALL:
    {
        if (IDYES == MessageBox(hWnd, L"Delete all feeds?", L"Delete all feeds", MB_ICONWARNING | MB_YESNO))
        {
            m_rudiRSSClient.DeleteAllFeedsAndAllFeedData();
            m_feedListView.DeleteAllItems();
            m_feedListView.ResetLastSelectedFeedId();
            m_feedListView.ResetLastSelectedFeedIndex();
            m_feedItemListView.DeleteAllItems();
        }
    }
    break;

    case IDC_SEARCH_EDIT_CONTROL:
    {
        switch (HIWORD(wParam))
        {
        case EN_CHANGE:
        {
            OnEditSearchBox();
        }
        break;

        default:
            break;
        }
    }
    break;

    case IDC_SEARCH_TYPE_COMBOBOX:
    {
        switch (HIWORD(wParam))
        {
        case CBN_SELCHANGE:
        {
            OnEditSearchBox();
        }
        break;

        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void RudiRSSMainWindow::OnDestroy()
{
    DeleteObject(m_font);
    DeleteObject(m_boldFont);
}

void RudiRSSMainWindow::InittializeControl()
{
    InitFont();

    RECT rc{};
    GetClientRect(m_hWnd, &rc);
    LONG height = rc.bottom - rc.top;

    m_searchBox.Initialize(rc.right - SearchBox::DEFAULT_WIDTH, 0, SearchBox::DEFAULT_WIDTH, SearchBox::DEFAULT_HEIGHT, 
        m_hWnd, m_hInstance, (HMENU)IDC_SEARCH_EDIT_CONTROL, L"Search");
    m_searchBox.SetFont(m_font);

    m_comboSearchType.Attach(CreateWindow(WC_COMBOBOX, nullptr, CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE,
        rc.right - SearchBox::DEFAULT_WIDTH - DEFAULT_SEARCH_TPYE_COMBOBOX_WIDTH,
        0, DEFAULT_SEARCH_TPYE_COMBOBOX_WIDTH, DEFAULT_SEARCH_TPYE_COMBOBOX_HEIGHT, m_hWnd, (HMENU)IDC_SEARCH_TYPE_COMBOBOX, m_hInstance, nullptr));
    SendMessage(m_comboSearchType.m_hWnd, CB_INSERTSTRING, static_cast<int>(SearchBox::SearchType::SOURCE_FEEDS), (LPARAM)L"Source feeds");
    SendMessage(m_comboSearchType.m_hWnd, CB_INSERTSTRING, static_cast<int>(SearchBox::SearchType::FEED_ITEMS), (LPARAM)L"Feed items");
    SendMessage(m_comboSearchType.m_hWnd, CB_SETCURSEL, static_cast<int>(SearchBox::SearchType::SOURCE_FEEDS), 0);
    SendMessage(m_comboSearchType.m_hWnd, WM_SETFONT, (WPARAM)m_font, TRUE);

    DisplayConfiguration displayConfig;
    m_rudiRSSClient.LoadDisplayConfiguration(displayConfig);

    int y = SearchBox::DEFAULT_HEIGHT + 1;
    m_feedListView.Initialize(m_hWnd, m_hInstance, (HMENU)IDC_FEED_LIST_VIEW, 0, y, displayConfig.feedWidth, height);
    m_feedListView.SetFeedSortMethod(displayConfig.feedSortMethod);
    SendMessage(m_feedListView.m_hWnd, WM_SETFONT, (WPARAM)m_font, TRUE);
    GetClientRect(m_feedListView.m_hWnd, &rc);
    LONG viewerX = m_feedListView.GetWidth();

    const int titleColWidth = displayConfig.feedItemTitleColumnWidth;
    const int updatedColWidth = displayConfig.feedItemUpdatedColumnWidth;
    m_feedItemListView.Initialize(m_hWnd, m_hInstance, (HMENU)IDC_FEED_ITEM_LIST_VIEW, rc.right + 1, y,
        titleColWidth + updatedColWidth, height, titleColWidth, updatedColWidth);
    SendMessage(m_feedItemListView.m_hWnd, WM_SETFONT, (WPARAM)m_font, TRUE);
    GetClientRect(m_feedItemListView.m_hWnd, &rc);
    viewerX += m_feedItemListView.GetWidth();

    RECT viewerRect{};
    GetClientRect(m_hWnd, &viewerRect);
    viewerRect.left = viewerX + 1;
    viewerRect.top = y;
    m_viewer.Initialize(m_hWnd, viewerRect, m_rudiRSSClient.GetRudiRSSFolder(), [&]() {
        InterlockedExchange(reinterpret_cast<LONG*>(&m_initViewer), TRUE);
        });
}

void RudiRSSMainWindow::InitFont()
{
    LOGFONT font{};
    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(font), &font, 0);
    m_font = CreateFont(font.lfHeight, font.lfWidth, font.lfEscapement, font.lfOrientation, font.lfWeight, font.lfItalic, font.lfUnderline, font.lfStrikeOut,
        font.lfCharSet, font.lfOutPrecision, font.lfClipPrecision, font.lfQuality, font.lfPitchAndFamily, font.lfFaceName);

    font.lfWeight = FW_BOLD;
    m_boldFont = CreateFont(font.lfHeight, font.lfWidth, font.lfEscapement, font.lfOrientation, font.lfWeight, font.lfItalic, font.lfUnderline, font.lfStrikeOut,
        font.lfCharSet, font.lfOutPrecision, font.lfClipPrecision, font.lfQuality, font.lfPitchAndFamily, font.lfFaceName);
}

void RudiRSSMainWindow::UpdateControl()
{
    if (!m_hWnd)
        return;

    RECT rc{};
    GetClientRect(m_hWnd, &rc);
    LONG height = rc.bottom - rc.top - SearchBox::DEFAULT_HEIGHT;

    MoveWindow(m_searchBox.m_hWnd, rc.right - SearchBox::DEFAULT_WIDTH, 0, m_searchBox.GetWidth(), SearchBox::DEFAULT_HEIGHT, TRUE);
    MoveWindow(m_comboSearchType.m_hWnd, rc.right - SearchBox::DEFAULT_WIDTH - DEFAULT_SEARCH_TPYE_COMBOBOX_WIDTH,
        0, DEFAULT_SEARCH_TPYE_COMBOBOX_WIDTH, DEFAULT_SEARCH_TPYE_COMBOBOX_HEIGHT, TRUE);

    int y = SearchBox::DEFAULT_HEIGHT + 1;
    MoveWindow(m_feedListView.m_hWnd, 0, y, m_feedListView.GetWidth(), height, TRUE);
    GetClientRect(m_feedListView.m_hWnd, &rc);
    LONG viewerX = m_feedListView.GetWidth();

    MoveWindow(m_feedItemListView.m_hWnd, rc.right + 1, y, m_feedItemListView.GetWidth(), height, TRUE);
    GetClientRect(m_feedItemListView.m_hWnd, &rc);
    viewerX += m_feedItemListView.GetWidth();

    if (InterlockedOr(reinterpret_cast<LONG*>(&m_initViewer), 0))
    {
        RECT viewerRect{};
        GetClientRect(m_hWnd, &viewerRect);
        viewerRect.left = viewerX + 1;
        viewerRect.top = y;
        m_viewer.MoveWindow(viewerRect);
    }
}

LRESULT RudiRSSMainWindow::OnProcessListViewCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case IDC_FEED_LIST_VIEW:
    {
        return m_feedListView.OnProcessMessage(hWnd, message, wParam, lParam);
    }
    break;

    case IDC_FEED_ITEM_LIST_VIEW:
    {
        return m_feedItemListView.OnProcessMessage(hWnd, message, wParam, lParam);
    }

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void RudiRSSMainWindow::OpenImportOPMLDialog()
{
    WCHAR path[MAX_PATH] = {};
    OPENFILENAME ofn = { sizeof(ofn) };
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = L"OPML file\0*.opml\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = ARRAYSIZE(path);
    if (GetOpenFileName(&ofn))
    {
        m_rudiRSSClient.ImportFromOPML(ofn.lpstrFile, [&](const std::vector<std::wstring>& feedUrls) {
            m_feedListView.UpdateFeedList(feedUrls);
            });
    }
}

void RudiRSSMainWindow::OpenImportListFileDialog()
{
    WCHAR path[MAX_PATH] = {};
    OPENFILENAME ofn = { sizeof(ofn) };
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = L"All files\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = ARRAYSIZE(path);
    if (GetOpenFileName(&ofn))
    {
        m_rudiRSSClient.ImportFromListFile(ofn.lpstrFile, [&](const std::vector<std::wstring>& feedUrls) {
            m_feedListView.UpdateFeedList(feedUrls);
            });
    }
}

BOOL RudiRSSMainWindow::IsViewerInitialized()
{
    return InterlockedOr(reinterpret_cast<LONG*>(&m_initViewer), 0);
}

void RudiRSSMainWindow::ClearLastSearchResult()
{
    m_lastSearchText.clear();
}

void RudiRSSMainWindow::ClearSearchBox()
{
    SetWindowText(m_searchBox.m_hWnd, L"");
}

SearchBox::SearchType RudiRSSMainWindow::GetSearchType()
{
    return static_cast<SearchBox::SearchType>(SendMessage(m_comboSearchType.m_hWnd, CB_GETCURSEL, 0, 0));
}

void RudiRSSMainWindow::OnEditSearchBox()
{
    m_lastSearchText = std::move(m_searchBox.GetSearchText());
    if (!m_lastSearchText.empty())
    {
        std::string query = std::format("%{}%", m_lastSearchText);
        long long cnt = 0;
        if (SearchBox::SearchType::FEED_ITEMS == GetSearchType())
        {
            if (FeedListView::ALL_FEEDS_LIST_INDEX == m_feedListView.GetLastSelectedFeedIndex())
                m_rudiRSSClient.QueryFeedDataCountByTitle(query, cnt);
            else
                m_rudiRSSClient.QueryFeedDataCountByFeedIdByTitle(m_feedListView.GetLastSelectedFeedId(), query, cnt);

            m_feedItemListView.DeleteAllItems();
            if (cnt > 0)
            {
                ListView_SetItemCount(m_feedItemListView.m_hWnd, cnt);
            }
        }
        else
        {
            m_rudiRSSClient.QueryFeedCountByTitle(query, cnt);
            m_feedListView.DeleteAllItems();
            if (cnt > 0)
            {
                ListView_SetItemCount(m_feedListView.m_hWnd, cnt + 1);
            }
        }
    }
    // Recover normal results
    else
    {
        if (SearchBox::SearchType::FEED_ITEMS == GetSearchType())
        {
            if (FeedListView::ALL_FEEDS_LIST_INDEX == m_feedListView.GetLastSelectedFeedIndex())
            {
                m_feedItemListView.UpdateAllFeeds();
            }
            else
            {
                m_feedItemListView.UpdateSelectedFeed(m_feedListView.GetLastSelectedFeedId());
            }
        }
        else
        {
            m_feedListView.UpdateFeedListFromDatabase();
        }
    }
}
