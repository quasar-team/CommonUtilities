/*
 * SharedQueueTest.cpp
 *
 *  Created on: Jun 16, 2015
 *      Author: bfarnham
 */

#include "SharedQueueTest.h"
#include <list>
#include <LogIt.h>
#include <LogLevels.h>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/ratio/ratio.hpp>

typedef boost::chrono::duration<size_t, boost::milli> DurationMs;

#define EXPECT_SIZE_EQ(expected, actual) \
	EXPECT_EQ(size_t(expected), actual)

SharedQueueTest::QueueItem::QueueItem(const size_t& id)
:m_id(id)
{}


SharedQueueTest::PutThreadObj::PutThreadObj(SharedQueue<QueueItemPtr>& sharedQueue,  const size_t& threadId, const size_t& numberOfPuts, boost::shared_mutex& sharedMutex, boost::condition& wait, WorkerThreadStartPredicate& startPredicate, const size_t& postStartPause/*=0*/)
:m_sharedQueue(sharedQueue), m_threadId(threadId), m_numberOfPuts(numberOfPuts), m_sharedMutex(sharedMutex), m_wait(wait), m_startPredicate(startPredicate), m_postStartPause(postStartPause)
{}

void SharedQueueTest::PutThreadObj::waitForSignal()
{
    boost::shared_lock<boost::shared_mutex> sharedLock(m_sharedMutex);
    m_wait.wait(sharedLock, m_startPredicate);
}

void SharedQueueTest::PutThreadObj::operator()()
{
	waitForSignal();
	LOG(Log::TRC) << "putter thread platform id ["<<boost::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"] running, will insert ["<<m_numberOfPuts<<"] items after a post start pause of ["<<m_postStartPause<<"]";

	if(m_postStartPause > 0)
	{
	    LOG(Log::TRC) << "Putter thread started, but executing post start pause of ["<<m_postStartPause<<"]";
	    boost::this_thread::sleep(boost::posix_time::milliseconds(m_postStartPause));
	    LOG(Log::TRC) << "Putter thread, post start pause of ["<<m_postStartPause<<"] complete";
	}

	for(size_t i=0; i<m_numberOfPuts; ++i)
	{
		const size_t itemNumber = SharedQueueTest::calculateQueueItemNumber(m_threadId, i);
		SharedQueueTest::QueueItem* item = new SharedQueueTest::QueueItem(itemNumber);
		QueueItemPtr itemToPut(item);
		m_sharedQueue.put(itemToPut);
	}

	LOG(Log::TRC) << "putter thread platform id ["<<boost::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"] complete, inserted ["<<m_numberOfPuts<<"] items";
}


SharedQueueTest::GetThreadObj::GetThreadObj(SharedQueue<QueueItemPtr>& sharedQueue,  const size_t& threadId, boost::shared_mutex& sharedMutex, boost::condition& wait, WorkerThreadStartPredicate& startPredicate)
:m_sharedQueue(sharedQueue), m_threadId(threadId), m_sharedMutex(sharedMutex), m_wait(wait), m_startPredicate(startPredicate)
{}

void SharedQueueTest::GetThreadObj::waitForSignal()
{
    boost::shared_lock<boost::shared_mutex> sharedLock(m_sharedMutex);
    m_wait.wait(sharedLock, m_startPredicate);
}

void SharedQueueTest::GetThreadObj::operator()()
{
    waitForSignal();
    LOG(Log::TRC) << "getter thread platform id ["<<boost::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"] running, getting items...";

    const size_t maxTimeoutMs = 1000;
    while(true)
    {
        size_t postPopQueueSz;
        bool isItemValid = false;
        SharedQueueTest::QueueItemPtr item = m_sharedQueue.maxWaitTake(isItemValid, postPopQueueSz, maxTimeoutMs);

        if(isItemValid)
        {
            m_gotItems.push_back(item);
        }
        else
        {
            LOG(Log::TRC) << "getter thread platform id ["<<boost::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"] no items received within ["<<maxTimeoutMs<<"ms], assume producers completed, exiting.";
            break;
        }
    }

    LOG(Log::TRC) << "getter thread platform id ["<<boost::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"] exiting, got ["<<m_gotItems.size()<<"] items";
}


bool SharedQueueTest::WorkerThreadStartPredicate::operator()()
{
	return m_externalStartFlag;
};

SharedQueueTest::SharedQueueTest()
{
	Log::initializeLogging(Log::INF);
}

SharedQueueTest::~SharedQueueTest()
{}

size_t SharedQueueTest::calculateThreadId(const size_t& threadNumber)
{
	return threadNumber * 1000000;
}

size_t SharedQueueTest::calculateQueueItemNumber(const size_t& threadNumber, const size_t& itemNumber)
{
	return calculateThreadId(threadNumber) + itemNumber;
}

bool SharedQueueTest::testExpectedQueueItemsPresent(const size_t& threadCount, const size_t& itemCount)
{
	std::set<size_t> expectedItemIds;
	for(size_t threadCounter=0; threadCounter < threadCount; ++threadCounter)
	{
		for(size_t itemCounter=0; itemCounter < itemCount; ++itemCounter)
		{
			const size_t expectedItemId = calculateQueueItemNumber(threadCounter, itemCounter);
			LOG(Log::TRC) << "expecting item ["<<expectedItemId<<"] in queue";
			expectedItemIds.insert(expectedItemId);
		}
	}
	const size_t expectedItemCount = expectedItemIds.size();
	LOG(Log::INF) << "expecting ["<<expectedItemCount<<"] items in queue";

	size_t actualItemCount = 0;
	size_t sizeAfterTake = 0;
	do
	{
		QueueItemPtr queueItem = m_testee.blockingTake(sizeAfterTake);
		++actualItemCount;

		std::set<size_t>::iterator pos = expectedItemIds.find(queueItem->m_id);

		if(pos == expectedItemIds.end())
		{
			LOG(Log::ERR) << "queue did not contain expected item ["<<queueItem->m_id<<"]";
			return false;
		}
		else
		{
			LOG(Log::TRC) << "queue contained expected item ["<<queueItem->m_id<<"]";
			expectedItemIds.erase(pos);
		}
	}
	while(sizeAfterTake > 0);

	if(actualItemCount != expectedItemCount)
	{
		LOG(Log::ERR) << "queue did not contain expected number of items, expected ["<<expectedItemCount<<"] actual ["<<actualItemCount<<"]";
		return false;
	}

	return true;
}

TEST_F(SharedQueueTest, singleThreadPutAndTake)
{
	SharedQueueTest::QueueItem* item = new SharedQueueTest::QueueItem(1);
	QueueItemPtr itemToPut(item);
	EXPECT_SIZE_EQ(1, m_testee.put(itemToPut)) << "should be a single item in the queue";

	size_t postTakeSize = 999;
	QueueItemPtr itemTaken = m_testee.blockingTake(postTakeSize);

	EXPECT_SIZE_EQ(0, postTakeSize) << "should be no items in the queue";
	EXPECT_SIZE_EQ(1, itemTaken->m_id) << "should be the same item as previously inserted";
}

TEST_F(SharedQueueTest, multiThreadPuts)
{
	bool workerThreadStartCondition = false;
	WorkerThreadStartPredicate startPredicate(workerThreadStartCondition);

	const size_t putterThreadCount = 20;
	const size_t putterThreadPutCount = 50000;

	boost::thread_group putterThreadGroup;
	std::list< boost::shared_ptr<PutThreadObj> > threadObjs;
	for(size_t i=0; i<putterThreadCount; ++i)
	{
		boost::shared_ptr<PutThreadObj> threadObj(new PutThreadObj(m_testee, i, putterThreadPutCount, m_workerThreadMutex, m_workerThreadWait, startPredicate));
		threadObjs.push_back(threadObj);

		boost::thread* putterThread = new boost::thread(boost::ref(*threadObj));
		putterThreadGroup.add_thread(putterThread);
	}

	// sleep to let all putter threads get ready for notify_all.
	boost::this_thread::sleep(boost::posix_time::milliseconds(200));

	// start all the workers
	workerThreadStartCondition = true;
	m_workerThreadWait.notify_all();

	putterThreadGroup.join_all();

	EXPECT_TRUE(testExpectedQueueItemsPresent(putterThreadCount, putterThreadPutCount)) << "threads should have put all expected items into the queue";
}

TEST_F(SharedQueueTest, multiThreadPutsAndMultiThreadTakes)
{
    bool workerThreadStartCondition = false;
    WorkerThreadStartPredicate startPredicate(workerThreadStartCondition);

    const size_t putterThreadCount = 10;
    const size_t putterThreadPutCount = 100000;

    boost::thread_group putterThreadGroup;
    std::list< boost::shared_ptr<PutThreadObj> > putterThreadObjs;
    for(size_t i=0; i<putterThreadCount; ++i)
    {
        boost::shared_ptr<PutThreadObj> threadObj(new PutThreadObj(m_testee, i, putterThreadPutCount, m_workerThreadMutex, m_workerThreadWait, startPredicate));
        putterThreadObjs.push_back(threadObj);

        boost::thread* putterThread = new boost::thread(boost::ref(*threadObj));
        putterThreadGroup.add_thread(putterThread);
    }

    const size_t getterThreadCount = 10;

    boost::thread_group getterThreadGroup;
    std::list< boost::shared_ptr<GetThreadObj> > getterThreadObjs;
    for(size_t i=0; i<getterThreadCount; ++i)
    {
        boost::shared_ptr<GetThreadObj> threadObj(new GetThreadObj(m_testee, i, m_workerThreadMutex, m_workerThreadWait, startPredicate));
        getterThreadObjs.push_back(threadObj);

        boost::thread* getterThread = new boost::thread(boost::ref(*threadObj));
        getterThreadGroup.add_thread(getterThread);
    }


    // sleep to let all putter threads get ready for notify_all.
    LOG(Log::INF) << "starting all threads simultaneously in 200ms...";
    boost::this_thread::sleep(boost::posix_time::milliseconds(200));

    // start all the workers
    workerThreadStartCondition = true;
    m_workerThreadWait.notify_all();

    putterThreadGroup.join_all();
    getterThreadGroup.join_all();

    const size_t totalItemsPut = putterThreadCount * putterThreadPutCount;
    LOG(Log::INF) << "all threads completed, check that ["<<totalItemsPut<<"] items were retrieved by the getter threads";

    size_t totalItemsGot = 0;
    BOOST_FOREACH(boost::shared_ptr<GetThreadObj> getThreadObj, getterThreadObjs)
    {
        totalItemsGot += getThreadObj->m_gotItems.size();
    }

    EXPECT_EQ(totalItemsPut, totalItemsGot) << "all ["<<totalItemsPut<<"] put items should have been retrieved from the queue";
}

TEST_F(SharedQueueTest, maxWaitTestSynchronous)
{
    bool isItemValid = true;
    size_t sizeAfterTake=999;

    // no items in queue
    QueueItemPtr item = m_testee.maxWaitTake(isItemValid, sizeAfterTake);
    EXPECT_FALSE(isItemValid) << "timeout should have expired";
    EXPECT_EQ((size_t)0, sizeAfterTake) << "should be no items in the queue";
    EXPECT_TRUE(item == 0) << "returned item should be empty";

    // put then take item synchronously in queue.
    m_testee.put( QueueItemPtr(new SharedQueueTest::QueueItem(999)));
    item = m_testee.maxWaitTake(isItemValid, sizeAfterTake);
    EXPECT_TRUE(isItemValid) << "should have received item";
    EXPECT_EQ((size_t)0, sizeAfterTake) << "should be no items left in the queue";
    EXPECT_EQ((size_t)999, item->m_id) << "returned item should be 999";

    // block main thread: waiting for take (max 100ms)...
    item = m_testee.maxWaitTake(isItemValid, sizeAfterTake, 100);
    EXPECT_FALSE(isItemValid) << "timeout should have expired";
}

TEST_F(SharedQueueTest, maxWaitTestAsychronous)
{
    bool workerThreadStartCondition = true;
    WorkerThreadStartPredicate startPredicate(workerThreadStartCondition);

    // start the putter thread (will wait for 100ms before putting)
    SharedQueueTest::PutThreadObj putterThreadObjectj(m_testee, 1, 1, m_workerThreadMutex, m_workerThreadWait, startPredicate, 100);
    boost::thread putterThread(putterThreadObjectj);

    bool isItemValid = true;
    size_t sizeAfterTake=999;
    QueueItemPtr item;

    // try and take within 1ms - should fail
    item = m_testee.maxWaitTake(isItemValid, sizeAfterTake, 1);
    EXPECT_FALSE(isItemValid) << "timeout should have expired, putter thread should not have put yet";
    EXPECT_FALSE(item != 0);

    // try and take, wait for 200ms, should
    item = m_testee.maxWaitTake(isItemValid, sizeAfterTake, 200);
    EXPECT_TRUE(isItemValid) << "timeout should have expired, putter thread should not have put yet";
    EXPECT_TRUE(item != 0);

    putterThread.join();
}

TEST_F(SharedQueueTest, maxWaitForNonEmptySynchronous)
{
    EXPECT_FALSE(m_testee.maxWaitForNonEmpty(0)) << "initial condition, queue empty";

    SharedQueueTest::QueueItem* item = new SharedQueueTest::QueueItem(0);
    QueueItemPtr itemToPut(item);

    m_testee.put(itemToPut);

    EXPECT_FALSE(m_testee.isEmpty());
    EXPECT_TRUE(m_testee.maxWaitForNonEmpty(0)) << "should not have to wait, immediate return as queue not empty";
}

TEST_F(SharedQueueTest, maxWaitForNonEmptyAsynchronous)
{
    EXPECT_FALSE(m_testee.maxWaitForNonEmpty(0)) << "initial condition, queue empty";

    bool workerThreadStartCondition = true;
    WorkerThreadStartPredicate startPredicate(workerThreadStartCondition);

    // start putter thread (will wait for 100ms before putting)
    SharedQueueTest::PutThreadObj putterThreadObjectj(m_testee, 1, 1, m_workerThreadMutex, m_workerThreadWait, startPredicate, 100);
    boost::thread putterThread(putterThreadObjectj);

    EXPECT_FALSE(m_testee.maxWaitForNonEmpty(1)) << "queue should be empty after 1ms, putter thread not executed yet";

    boost::chrono::system_clock::time_point timeAtStartWait = boost::chrono::system_clock::now();
    EXPECT_TRUE(m_testee.maxWaitForNonEmpty(500)) << "queue should be non empty once putter thread executes after initial 100ms delay";
    const DurationMs durationMs = boost::chrono::duration_cast<DurationMs>(boost::chrono::system_clock::now() - timeAtStartWait);

    EXPECT_TRUE(durationMs.count() < 250) << "wait for maxWaitForNonEmpty to return ["<<durationMs.count()<<"] should not wait for even half of the 500ms max time";

    putterThread.join();
}
