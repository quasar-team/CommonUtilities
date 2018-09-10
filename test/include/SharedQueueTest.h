/*
 * SharedQueueTest.h
 *
 *  Created on: Jun 16, 2015
 *      Author: bfarnham
 */

#ifndef SHAREDQUEUETEST_H_
#define SHAREDQUEUETEST_H_

#include "gtest/gtest.h"
#include <boost/shared_ptr.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <list>

#include "SharedQueue.h"

class SharedQueueTest;

class SharedQueueTest  : public ::testing::Test
{
public:

	class QueueItem
	{
	public:
		QueueItem(const size_t& id);
		const size_t m_id;
	};
	typedef boost::shared_ptr<QueueItem> QueueItemPtr;

	struct WorkerThreadStartPredicate
	{
		WorkerThreadStartPredicate(bool& externalStartFlag):m_externalStartFlag(externalStartFlag){};
		bool& m_externalStartFlag;
		bool operator()();
	};

	class PutThreadObj
	{
	public:
		PutThreadObj(SharedQueue<QueueItemPtr>& sharedQueue, const size_t& threadId, const size_t& numberOfPuts, boost::shared_mutex& sharedMutex, boost::condition& wait, WorkerThreadStartPredicate& startPredicate, const size_t& postStartPause=0);

		void waitForSignal();
		void operator()();

		SharedQueue<QueueItemPtr>& m_sharedQueue;
		const size_t m_threadId;
		const size_t m_numberOfPuts;

		boost::shared_mutex& m_sharedMutex;
		boost::condition& m_wait;
		WorkerThreadStartPredicate& m_startPredicate;
		const size_t m_postStartPause;
	};

    class GetThreadObj
    {
    public:
        GetThreadObj(SharedQueue<QueueItemPtr>& sharedQueue, const size_t& threadId, boost::shared_mutex& sharedMutex, boost::condition& wait, WorkerThreadStartPredicate& startPredicate);

        void waitForSignal();
        void operator()();

        SharedQueue<QueueItemPtr>& m_sharedQueue;
        const size_t m_threadId;
        std::list<QueueItemPtr> m_gotItems;

        boost::shared_mutex& m_sharedMutex;
        boost::condition& m_wait;
        WorkerThreadStartPredicate& m_startPredicate;
    };

	SharedQueueTest();
	virtual ~SharedQueueTest();

	static size_t calculateThreadId(const size_t& threadNumber);
	static size_t calculateQueueItemNumber(const size_t& threadNumber, const size_t& itemNumber);
	bool testExpectedQueueItemsPresent(const size_t& threadCount, const size_t& itemCount);

	SharedQueue<QueueItemPtr> m_testee;

	boost::shared_mutex m_workerThreadMutex;
	boost::condition m_workerThreadWait;
};

#endif /* SHAREDQUEUETEST_H_ */
