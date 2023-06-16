#pragma once

#include "FetchUnit.h"
#include "WinHttpSession.h"

#include <deque>
#include <memory>
#include <atlcore.h>
#include <functional>
#include <vector>

class FeedFetcher
{
protected:
    static const size_t DEFAULT_MAX_UNIT_COUNT = 8;

    size_t m_maxUnitCount;
    WinHttpSession m_session;
    HANDLE m_fetchThread;
    HANDLE m_stopEvent;
    HANDLE m_fetchSemaphore;

    using PTR_FETCH_UNIT = std::unique_ptr<FetchUnit>;
    std::deque<PTR_FETCH_UNIT> m_idleUnits;
    std::deque<std::wstring> m_feedUrls;
    ATL::CComCriticalSection m_feedLock;
    ATL::CComCriticalSection m_idleLock;
    std::vector<HANDLE> m_unitCompleteEvents;

    static unsigned __stdcall ThreadFetch(void* param);
    void Fetch();

    void ReleaseSemaphore();

    void PushUnit(PTR_FETCH_UNIT &unit, std::deque<PTR_FETCH_UNIT> &unitQueue, ATL::CComCriticalSection &unitLock);
    PTR_FETCH_UNIT PopUnit(std::deque<PTR_FETCH_UNIT> &unitQueue, ATL::CComCriticalSection &unitLock);
    size_t GetQueueSize(std::deque<PTR_FETCH_UNIT> &unitQueue, ATL::CComCriticalSection &unitLock);

    using FN_ON_FETCH_UNIT_COMPLETE = std::function<void(const FetchUnit*, bool, DWORD, const char*, size_t, const WINHTTP_ASYNC_RESULT*)>;
    FN_ON_FETCH_UNIT_COMPLETE m_onFetchUnitComplete;

public:
    FeedFetcher();
    explicit FeedFetcher(size_t maxUnitCount);
    FeedFetcher &operator=(const FeedFetcher& rhs) = delete;
    virtual ~FeedFetcher();

    void Initialize(FN_ON_FETCH_UNIT_COMPLETE onFetchUnitComplete);
    void Deinitialize();
    void StartFetchRoutine();
    void StopFetchRoutine();

    size_t GetFeedUrlSize();
    void PushFeedUrl(const std::wstring &feedUrl);
    bool PopFeedUrl(std::wstring &feedUrl);
};
