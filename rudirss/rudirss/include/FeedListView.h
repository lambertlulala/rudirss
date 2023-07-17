#pragma once

#include "ListView.h"
#include "FeedDatabase.h"

#include <map>

class RudiRSSMainWindow;

class FeedListView : public ListView
{
public:
    FeedListView();
    explicit FeedListView(RudiRSSMainWindow *mainWindow);
    virtual ~FeedListView();

    void Initialize(HWND hWnd, HINSTANCE hInstance, HMENU windowId, int x, int y, int width, int height);

    virtual LRESULT OnProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    static const int ALL_FEEDS_LIST_INDEX = 0;

    long long GetLastSelectedFeedId();
    int GetLastSelectedFeedIndex();

    void UpdateFeedListFromDatabase();

    virtual void ClearCache();

protected:
    long long m_lastSelectedFeedId;
    int m_lastSelectedFeedIndex;
    RudiRSSMainWindow* m_mainWindow;
    std::map<long long, FeedDatabase::Feed> m_cache;
};
