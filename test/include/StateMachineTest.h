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
#ifndef STATE_MACHINE_TEST_H_
#define STATE_MACHINE_TEST_H_

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include "gtest/gtest.h"
#include "StateMachine.h"
#include "StateFactory.h"
#include "MockStateFactory.h"
#include "MockState.h"

class StateMachineTests : public ::testing::Test
{
public:

	class StateMachineThread
	{
	public:
		enum THREAD_STATE {CONSTRUCTED, WAITING_FOR_START, STARTED, COMPLETED};

		StateMachineThread(StateMachine& stateMachine);

		void operator()();
		bool start(const size_t& postStartPauseMs);
		bool waitForThreadState(const THREAD_STATE& targetState);

		StateMachine& m_stateMachine;

		boost::mutex m_mutex;
		boost::condition m_wait;

		THREAD_STATE m_state;
	};

	StateMachineTests();

	virtual void TearDown();

	bool isMockStateExecuted(const STATE_IDS::STATE_ID& stateId);
	void waitForStateMachineThreadToExit();
	bool interruptStateMachineThreadAndWaitForExit();

	bool m_stopRunningFlag;
	MockStateFactory m_mockStateFactory;
	StateMachine m_testee;
	StateMachineThread m_stateMachineThreadObj;
	boost::thread m_stateMachineThread;
};

#endif /* STATE_MACHINE_TEST_H_ */
