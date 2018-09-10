/*
 * MockState.h
 *
 *  Created on: Jun 11, 2015
 *      Author: bfarnham
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
