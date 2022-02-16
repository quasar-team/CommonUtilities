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
#pragma once
#include <queue>
#include <chrono>
#include <mutex>
#include <condition_variable>

/**
 * Recommend that the queue template instantiation uses
 * std::shared_ptr<your_class>, to avoid having to
 * clean up allocated memory after queued items are
 * processed
 *
 * example instance declaration
 * SharedQueue< std::shared_ptr<MyClass> > m_sharedQueue;
 */
template<typename TQueueItem>
class SharedQueue
{
public:
	SharedQueue(){};
	virtual ~SharedQueue(){};

	const size_t put(TQueueItem queueItem)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_queue.push(queueItem);

		const size_t result =  m_queue.size();
		m_wait.notify_one();

		return result;
	};

	TQueueItem blockingTake(size_t& sizeAfterTake)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		while(m_queue.empty())
		{
			m_wait.wait(lock);
		}

		TQueueItem queueItem = m_queue.front();
		m_queue.pop();

		sizeAfterTake = m_queue.size();
		return queueItem;
	};

	TQueueItem maxWaitTake(bool& isItemValid, size_t& sizeAfterTake, const size_t& maxWaitMs = 1)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

	    if(!m_queue.empty())
	    {
	        TQueueItem queueItem = m_queue.front();
	        m_queue.pop();

	        sizeAfterTake = m_queue.size();
	        isItemValid = true;
	        return queueItem;
	    }
	    else
	    {
			const bool waitResult = m_wait.wait_for(lock, std::chrono::milliseconds(maxWaitMs)) != std::cv_status::timeout;
            if(waitResult && !m_queue.empty())
            {
                TQueueItem queueItem = m_queue.front();
                m_queue.pop();

                sizeAfterTake = m_queue.size();
                isItemValid = true;
                return queueItem;
            }
	    }

	    sizeAfterTake = 0;
        isItemValid = false;
        return TQueueItem();
	}

	bool maxWaitForNonEmpty(const size_t& maxWaitMs)
	{
	    if(!m_queue.empty())
	    {
	        return true;
	    }
	    else
	    {
			std::unique_lock<std::mutex> lock(m_mutex);
	        return m_wait.wait_for(lock, std::chrono::milliseconds(maxWaitMs)) != std::cv_status::timeout;
	    }
	}

	bool isEmpty() const
	{
	    return m_queue.empty();
	}

	size_t discardContents()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		const size_t numItemsDiscarded = getSize();
		std::swap(m_queue, std::queue<TQueueItem>());
		return numItemsDiscarded;
	}

	const size_t getSize()
	{
		return m_queue.size();
	}

private:
	std::queue<TQueueItem> m_queue;

	std::mutex m_mutex;
	std::condition_variable m_wait;
};
