/* © Copyright CERN, 2018. All rights not expressly granted are reserved. */
/*
 *
 *  Created on: Sep 12, 2018
 *      Author: Ben Farnham <ben.farnham@cern.ch>
 *
 *  This file is part of Quasar.
 *
 *  Quasar is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public Licence as published by
 *  the Free Software Foundation, either version 3 of the Licence.
 *
 *  Quasar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public Licence for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Quasar.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "SharedQueueTest.h"
#include <list>
#include <thread>
#include <chrono>
#include <algorithm>
#include <LogIt.h>
#include <LogLevels.h>

#define EXPECT_SIZE_EQ(expected, actual) \
	EXPECT_EQ(size_t(expected), actual)

SharedQueueTest::QueueItem::QueueItem(const size_t& id)
:m_id(id)
{}


SharedQueueTest::PutThreadObj::PutThreadObj(SharedQueue<QueueItemPtr>& sharedQueue,  const size_t& threadId, const size_t& numberOfPuts, std::mutex& sharedMutex, std::condition_variable& wait, WorkerThreadStartPredicate& startPredicate, const size_t& postStartPause/*=0*/)
:m_sharedQueue(sharedQueue), m_threadId(threadId), m_numberOfPuts(numberOfPuts), m_sharedMutex(sharedMutex), m_wait(wait), m_startPredicate(startPredicate), m_postStartPause(postStartPause)
{}

void SharedQueueTest::PutThreadObj::waitForSignal()
{
    LOG(Log::INF) << __FUNCTION__ << "+ putter thread id ["<<std::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"]";
    std::unique_lock<std::mutex> sharedLock(m_sharedMutex);
    m_wait.wait(sharedLock, m_startPredicate);
    LOG(Log::INF) << __FUNCTION__ << "- putter thread id ["<<std::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"]";
}

void SharedQueueTest::PutThreadObj::execute()
{
	LOG(Log::INF) << __FUNCTION__ << "+ putter thread id ["<<std::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"] running, will insert ["<<m_numberOfPuts<<"], post start pause ["<<m_postStartPause<<"]";
	waitForSignal();

	if(m_postStartPause > 0)
	{
	    LOG(Log::TRC) << "Putter thread started, but executing post start pause of ["<<m_postStartPause<<"]";
	    std::this_thread::sleep_for(std::chrono::milliseconds(m_postStartPause));
	    LOG(Log::TRC) << "Putter thread, post start pause of ["<<m_postStartPause<<"] complete";
	}

	for(size_t i=0; i<m_numberOfPuts; ++i)
	{
		const size_t itemNumber = SharedQueueTest::calculateQueueItemNumber(m_threadId, i);
		SharedQueueTest::QueueItem* item = new SharedQueueTest::QueueItem(itemNumber);
		QueueItemPtr itemToPut(item);
		m_sharedQueue.put(itemToPut);
	}

	LOG(Log::INF) << __FUNCTION__ << "- putter thread id ["<<std::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"] complete, inserted ["<<m_numberOfPuts<<"] items";
}


SharedQueueTest::GetThreadObj::GetThreadObj(SharedQueue<QueueItemPtr>& sharedQueue,  const size_t& threadId, std::mutex& sharedMutex, std::condition_variable& wait, WorkerThreadStartPredicate& startPredicate)
:m_sharedQueue(sharedQueue), m_threadId(threadId), m_sharedMutex(sharedMutex), m_wait(wait), m_startPredicate(startPredicate)
{
    LOG(Log::INF) << __FUNCTION__ << "+ getter id ["<<std::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"]";
}

SharedQueueTest::GetThreadObj::~GetThreadObj()
{
    LOG(Log::INF) << __FUNCTION__ << "+ getter id ["<<std::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"], count ["<<m_gotItems.size()<<"]";
}

void SharedQueueTest::GetThreadObj::waitForSignal()
{
    LOG(Log::INF) << __FUNCTION__ << "+ getter id ["<<std::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"]";
    std::unique_lock<std::mutex> sharedLock(m_sharedMutex);
    m_wait.wait(sharedLock, m_startPredicate);
    LOG(Log::INF) << __FUNCTION__ << "- getter id ["<<std::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"]";
}

void SharedQueueTest::GetThreadObj::execute()
{
    LOG(Log::INF) << __FUNCTION__ << "+ getter thread id ["<<std::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"] running, getting items...";
    waitForSignal();

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
            LOG(Log::WRN) << __FUNCTION__ << " getter thread id ["<<std::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"] no items received within ["<<maxTimeoutMs<<"ms], assume producers completed, exiting.";
            break;
        }
    }

    LOG(Log::INF) << __FUNCTION__ << "- getter thread id ["<<std::this_thread::get_id()<<"] this ["<<this<<"] id ["<<m_threadId<<"] exiting, got ["<<m_gotItems.size()<<"] items";
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

	std::list<std::thread> putterThreadGroup;
	std::list<std::shared_ptr<PutThreadObj>> threadObjs;
	for(size_t i=0; i<putterThreadCount; ++i)
	{
        threadObjs.emplace_back(std::make_shared<PutThreadObj>(m_testee, i, putterThreadPutCount, m_workerThreadMutex, m_workerThreadWait, startPredicate));
        putterThreadGroup.emplace_back(std::thread(&SharedQueueTest::PutThreadObj::execute, threadObjs.back()));
	}

	// sleep to let all putter threads get ready for notify_all.
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// start all the workers
	workerThreadStartCondition = true;
	m_workerThreadWait.notify_all();

    std::for_each(putterThreadGroup.begin(), putterThreadGroup.end(), [](std::thread& threadObj){ threadObj.join(); });

	EXPECT_TRUE(testExpectedQueueItemsPresent(putterThreadCount, putterThreadPutCount)) << "threads should have put all expected items into the queue";
}

TEST_F(SharedQueueTest, multiThreadPutsAndMultiThreadTakes)
{
    bool workerThreadStartCondition = false;
    WorkerThreadStartPredicate startPredicate(workerThreadStartCondition);

    const size_t putterThreadCount = 10;
    const size_t putterThreadPutCount = 1000000;

    std::list<std::thread> putterThreadGroup;
    std::list< std::shared_ptr<PutThreadObj>> putterThreadObjs;
    for(size_t i=0; i<putterThreadCount; ++i)
    {
        putterThreadObjs.emplace_back(std::make_shared<PutThreadObj>(m_testee, i, putterThreadPutCount, m_workerThreadMutex, m_workerThreadWait, startPredicate));
        putterThreadGroup.emplace_back(&SharedQueueTest::PutThreadObj::execute, putterThreadObjs.back());
    }

    const size_t getterThreadCount = 1;

    std::list<std::thread> getterThreadGroup;
    std::list< std::shared_ptr<GetThreadObj> > getterThreadObjs;
    for(size_t i=0; i<getterThreadCount; ++i)
    {
        getterThreadObjs.emplace_back(std::make_shared<GetThreadObj>(m_testee, i, m_workerThreadMutex, m_workerThreadWait, startPredicate));
        getterThreadGroup.emplace_back(&SharedQueueTest::GetThreadObj::execute, getterThreadObjs.back());
    }

    // sleep to let all putter threads get ready for notify_all.
    LOG(Log::INF) << "starting all threads simultaneously in 100ms...";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // start all the workers
    workerThreadStartCondition = true;
    m_workerThreadWait.notify_all();

    std::for_each(putterThreadGroup.begin(), putterThreadGroup.end(), [](std::thread& threadObj){ threadObj.join(); });
    std::for_each(getterThreadGroup.begin(), getterThreadGroup.end(), [](std::thread& threadObj){ threadObj.join(); });

    const size_t totalItemsPut = putterThreadCount * putterThreadPutCount;
    LOG(Log::INF) << "all threads completed, check that ["<<totalItemsPut<<"] items were retrieved by the getter threads";

    size_t totalItemsGot = 0;
    for(const auto& getThreadObj : getterThreadObjs)
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

    // start the putter thread (will wait for 1000ms before putting)
    SharedQueueTest::PutThreadObj putterThreadObjectj(m_testee, 1, 1, m_workerThreadMutex, m_workerThreadWait, startPredicate, 1000);
    std::thread putterThread(&SharedQueueTest::PutThreadObj::execute, &putterThreadObjectj);

    bool isItemValid = true;
    size_t sizeAfterTake=999;
    QueueItemPtr item;

    // try and take within 1ms - should fail
    item = m_testee.maxWaitTake(isItemValid, sizeAfterTake, 1);
    EXPECT_FALSE(isItemValid) << "timeout should have expired, putter thread should not have put yet";
    EXPECT_FALSE(item != 0);

    // try and take, wait for 2000ms, should get something
    item = m_testee.maxWaitTake(isItemValid, sizeAfterTake, 2000);
    EXPECT_TRUE(isItemValid) << "timeout should not expire, putter thread have put by now";
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
    PutThreadObj putterThreadObjectj(m_testee, 1, 1, m_workerThreadMutex, m_workerThreadWait, startPredicate, 100);
    std::thread putterThread(&SharedQueueTest::PutThreadObj::execute, &putterThreadObjectj);

    EXPECT_FALSE(m_testee.maxWaitForNonEmpty(1)) << "queue should be empty after 1ms, putter thread not executed yet";

    const auto timeAtStartWait = std::chrono::high_resolution_clock::now();
    EXPECT_TRUE(m_testee.maxWaitForNonEmpty(500)) << "queue should be non empty once putter thread executes after initial 100ms delay";
    const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timeAtStartWait);

    EXPECT_TRUE(durationMs.count() < 250) << "wait for maxWaitForNonEmpty to return ["<<durationMs.count()<<"] should not wait for even half of the 500ms max time";

    putterThread.join();
}

TEST_F(SharedQueueTest, testDiscardContents)
{
    EXPECT_TRUE(m_testee.isEmpty()) << "initial condition, queue empty";
    m_testee.put(QueueItemPtr(new SharedQueueTest::QueueItem(0)));
    m_testee.put(QueueItemPtr(new SharedQueueTest::QueueItem(1)));
    m_testee.put(QueueItemPtr(new SharedQueueTest::QueueItem(2)));
    EXPECT_FALSE(m_testee.isEmpty()) << "queue expected not empty";
    std::vector<QueueItemPtr> discardedContents; // empty
    auto result = m_testee.discardContents(discardedContents);
    EXPECT_EQ(3, result) << "returned discarded item count";
    EXPECT_EQ(3, discardedContents.size());
    EXPECT_TRUE(m_testee.isEmpty()) << "final condition, queue empty";
}
