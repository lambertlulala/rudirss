#include "FeedFetcher.h"

#include <process.h>
#include <atlbase.h>

#include <stdexcept>
#include <iterator>
#include <algorithm>

FeedFetcher::FeedFetcher() : FeedFetcher(DEFAULT_MAX_UNIT_COUNT)
{
}

FeedFetcher::FeedFetcher(size_t maxUnitCount) : m_maxUnitCount{ maxUnitCount }, 
m_fetchThread{ nullptr }, m_stopEvent{ nullptr }
{
    m_stopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    m_fetchSemaphore = CreateSemaphore(nullptr, 0, m_maxUnitCount, nullptr);
    m_feedLock.Init();
    m_idleLock.Init();
}

FeedFetcher::~FeedFetcher()
{
    StopFetchRoutine();
    CloseHandle(m_stopEvent);
    CloseHandle(m_fetchSemaphore);
}

void FeedFetcher::Initialize(FN_ON_FETCH_UNIT_COMPLETE onFetchUnitComplete)
{
    if (!SUCCEEDED(m_session.Initialize(L"RSS feed fetcher")))
        throw std::runtime_error("Error: initialize HTTP session.");

    if (0 == m_maxUnitCount)
        throw std::invalid_argument("Error: max unit count is 0.");

    m_onFetchUnitComplete = onFetchUnitComplete;
    std::generate_n(std::back_inserter(m_idleUnits), m_maxUnitCount, []() { return std::make_unique<FetchUnit>(); });
    for (const auto& unit : m_idleUnits)
    {
        m_unitCompleteEvents.push_back(unit->GetFetchCompleteEvent());
    }
}
void FeedFetcher::Deinitialize()
{
    m_onFetchUnitComplete = nullptr;
    m_session.Close();
    m_idleUnits.clear();
    m_unitCompleteEvents.clear();
}

void FeedFetcher::StartFetchRoutine()
{
    StopFetchRoutine();

    ResetEvent(m_stopEvent);
    if (!m_fetchThread)
    {
        m_fetchThread = (HANDLE)_beginthreadex(nullptr, 0, ThreadFetch, this, 0, nullptr);
    }
}

void FeedFetcher::StopFetchRoutine()
{
    if (m_fetchThread)
    {
        SetEvent(m_stopEvent);
        ReleaseSemaphore();
        WaitForSingleObject(m_fetchThread, INFINITE);
        CloseHandle(m_fetchThread);
        m_fetchThread = nullptr;

        for (const auto& handle : m_unitCompleteEvents)
        {
            WaitForSingleObject(handle, INFINITE);
        }
    }
}

unsigned __stdcall FeedFetcher::ThreadFetch(void* param)
{
    auto pThis = reinterpret_cast<FeedFetcher*>(param);
    pThis->Fetch();
    return 0;
}

void FeedFetcher::Fetch()
{
    while (WAIT_OBJECT_0 != WaitForSingleObject(m_stopEvent, 0))
    {
        WaitForSingleObject(m_fetchSemaphore, INFINITE);
        if (WAIT_OBJECT_0 == WaitForSingleObject(m_stopEvent, 0))
            break;

        auto idleUnit = PopUnit(m_idleUnits, m_idleLock);
        if (!idleUnit)
            continue;

        std::wstring feedUrl;
        if (!PopFeedUrl(feedUrl))
        {
            PushUnit(idleUnit, m_idleUnits, m_idleLock);
            continue;
        }

        try
        {
            auto unit = idleUnit.release();
            unit->Fetch(unit, m_session, feedUrl,
                [&](const void* userParam, bool result, DWORD statusCode, const char* data, size_t size, const WINHTTP_ASYNC_RESULT* asyncResult) {
                    std::unique_ptr<FetchUnit> ptrUnit(reinterpret_cast<FetchUnit*>(const_cast<void*>(userParam)));
                    if (m_onFetchUnitComplete)
                        m_onFetchUnitComplete(ptrUnit.get(), result, statusCode, data, size, asyncResult);
                    SetEvent(ptrUnit->GetFetchCompleteEvent());
                    PushUnit(ptrUnit, m_idleUnits, m_idleLock);
                    ReleaseSemaphore();
                });
        }
        catch (const std::exception& e)
        {
        }
    }
}

void FeedFetcher::ReleaseSemaphore()
{
    ::ReleaseSemaphore(m_fetchSemaphore, 1, nullptr);
}

size_t FeedFetcher::GetFeedUrlSize()
{
    ATL::CComCritSecLock lock(m_feedLock);
    return m_feedUrls.size();
}

void FeedFetcher::PushFeedUrl(const std::wstring& feedUrl)
{
    m_feedLock.Lock();
    m_feedUrls.push_back(feedUrl);
    m_feedLock.Unlock();
    ReleaseSemaphore();
}

bool FeedFetcher::PopFeedUrl(std::wstring& feedUrl)
{
    ATL::CComCritSecLock lock(m_feedLock);
    if (m_feedUrls.empty())
        return false;

    feedUrl = m_feedUrls.front();
    m_feedUrls.pop_front();

    return true;
}

void FeedFetcher::PushUnit(PTR_FETCH_UNIT &unit, std::deque<PTR_FETCH_UNIT> &unitQueue, ATL::CComCriticalSection &unitLock)
{
    ATL::CComCritSecLock lock(unitLock);
    unitQueue.push_back(std::move(unit));
}

FeedFetcher::PTR_FETCH_UNIT FeedFetcher::PopUnit(std::deque<PTR_FETCH_UNIT> &unitQueue, ATL::CComCriticalSection &unitLock)
{
    PTR_FETCH_UNIT unit;
    ATL::CComCritSecLock lock(unitLock);
    if (unitQueue.size() > 0)
    {
        unit = std::move(unitQueue.front());
        unitQueue.pop_front();
    }
    return unit;
}

size_t FeedFetcher::GetQueueSize(std::deque<PTR_FETCH_UNIT> &unitQueue, ATL::CComCriticalSection &unitLock)
{
    ATL::CComCritSecLock lock(unitLock);
    return unitQueue.size();
}
