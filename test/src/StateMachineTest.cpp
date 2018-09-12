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
#include "StateMachineTest.h"
#include <LogIt.h>
#include <LogLevels.h>

using namespace STATE_IDS;

extern Log::LogComponentHandle STATES = Log::INVALID_HANDLE;


StateMachineTests::StateMachineThread::StateMachineThread(StateMachine& stateMachine)
:m_stateMachine(stateMachine), m_status(CONSTRUCTED)
{
	LOG(Log::INF) << __FUNCTION__ << "+";
	LOG(Log::INF) << __FUNCTION__ << "-";
}

StateMachineTests::StateMachineThread::~StateMachineThread()
{
	LOG(Log::INF) << __FUNCTION__ << "+";
	LOG(Log::INF) << __FUNCTION__ << "-";
}

void StateMachineTests::StateMachineThread::operator()()
{
	LOG(Log::INF) << __FUNCTION__ << "+ thread id ["<<std::this_thread::get_id()<<"]";
	LOG(Log::INF) << __FUNCTION__ << " got lock";
	m_status = WAITING_FOR_START;

	try
	{
		LOG(Log::INF) << __FUNCTION__ << " waiting for notify";
		LOG(Log::INF) << __FUNCTION__ << " notified";
		m_status = STARTED;

		m_stateMachine.run(0);
	}
	catch(...)
	{
	    LOG(Log::INF) << __FUNCTION__ << " State machine thread exception (probably interrupt), exiting";
	}

	m_status = COMPLETED;
	LOG(Log::INF) << __FUNCTION__ << "-";
}

StateMachineTests::StateMachineTests()
:m_stopRunningFlag(false), m_mockStateFactory(&m_stopRunningFlag), m_testee(m_mockStateFactory), m_stateMachineThreadObj(m_testee)
{
	LOG(Log::TRC) << __FUNCTION__ << "+";
	LOG(Log::TRC) << __FUNCTION__ << "-";
}

StateMachineTests::~StateMachineTests()
{
	LOG(Log::TRC) << __FUNCTION__ << "+";
	LOG(Log::TRC) << __FUNCTION__ << "-";
}

bool StateMachineTests::startStateMachine()
{
	LOG(Log::INF) << __FUNCTION__ << " starting state machine thread";
	m_stateMachineThread = std::thread(std::ref(m_stateMachineThreadObj));
	LOG(Log::INF) << __FUNCTION__ << " state machine thread started";
	if (waitForStateMachineThreadStatus(STARTED))
	{
		LOG(Log::INF) << __FUNCTION__ << " state machine thread started";
		return true;
	}
	else
	{
		LOG(Log::ERR) << __FUNCTION__ << " state machine thread failed to start";
		return false;
	}
}

bool StateMachineTests::stopStateMachine()
{
	LOG(Log::INF) << __FUNCTION__ << " calling stop";
	m_testee.stop();
	LOG(Log::INF) << __FUNCTION__ << " called stop, waiting for completion";
	if (waitForStateMachineThreadStatus(COMPLETED))
	{
		LOG(Log::INF) << __FUNCTION__ << " state machine thread completed, waiting for state machien thread to join";
		if (m_stateMachineThread.joinable())
		{
			m_stateMachineThread.join();
		}
		LOG(Log::INF) << __FUNCTION__ << " state machine thread joined";
		return true;
	}
	else
	{
		LOG(Log::ERR) << __FUNCTION__ << " state machine thread failed to complete!";
		return false;
	}
}

bool StateMachineTests::waitForStateMachineThreadStatus(const STATE_MACHINE_THREAD_STATUS& targetStatus)
{
	const size_t snoozeMs = 10;
	const size_t maxWait = snoozeMs * 20;
	size_t elapsedWait = 0;

	LOG(Log::INF) << __FUNCTION__ << " waiting for target status [" << targetStatus << "], current status [" << m_stateMachineThreadObj.m_status<< "]";
	while (m_stateMachineThreadObj.m_status != targetStatus && elapsedWait < maxWait)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(snoozeMs));
		LOG(Log::INF) << __FUNCTION__ << " post snooze status [" << m_stateMachineThreadObj.m_status << "] elapsed [" << elapsedWait << "]";
		elapsedWait += snoozeMs;
	}

	LOG(Log::INF) << __FUNCTION__ << " wait complete target status [" << targetStatus << "], current status [" << m_stateMachineThreadObj.m_status << "]";
	return targetStatus == m_stateMachineThreadObj.m_status;
}

void StateMachineTests::TearDown()
{
	LOG(Log::INF) << __FUNCTION__ << "+";
	if(!stopStateMachine())
	{
		LOG(Log::ERR) << __FUNCTION__ << " failed to stop state machine thread";
	}
	LOG(Log::INF) << __FUNCTION__ << "-";
}

bool StateMachineTests::isMockStateExecuted(const STATE_IDS::STATE_ID& stateId)
{
	std::shared_ptr<State> state = m_mockStateFactory.getState(stateId);

	if(!state)
	{
		LOG(Log::ERR) << __FUNCTION__ << ": No state!";
		return false;
	}

	State* rawState = state.get();
	if(!rawState)
	{
		LOG(Log::ERR) << __FUNCTION__ << ": null state";
		return false;
	}

	MockState* mockState = dynamic_cast<MockState*>(rawState);
	if(!mockState)
	{
		LOG(Log::ERR) << __FUNCTION__ << ": invalid state type";
		return false;
	}

	return mockState->getExecutionCount() > 0;
}

TEST_F(StateMachineTests, testStartAndStop)
{
	EXPECT_TRUE(startStateMachine());
	EXPECT_TRUE(stopStateMachine());
}

TEST_F(StateMachineTests, stateMachineRun)
{
	EXPECT_TRUE(startStateMachine());
	EXPECT_TRUE(m_testee.isRunning()) << "state machine should now be running";
}

TEST_F(StateMachineTests, stateMachineStop)
{
	EXPECT_TRUE(startStateMachine());
	m_testee.stop();

	EXPECT_TRUE(waitForStateMachineThreadStatus(COMPLETED));
	EXPECT_FALSE(m_testee.isRunning()) << "state machine should have stopped";
}

TEST_F(StateMachineTests, targetStatesExecuted)
{
	EXPECT_TRUE(startStateMachine());
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_TRUE(stopStateMachine());

	EXPECT_TRUE(isMockStateExecuted(CONNECT)) << "state should be executed more than once";
	EXPECT_TRUE(isMockStateExecuted(SYSQUERY)) << "state should be executed more than once";
	EXPECT_TRUE(isMockStateExecuted(COMMANDS)) << "state should be executed more than once";
	EXPECT_TRUE(isMockStateExecuted(DISCONNECT)) << "state should be executed more than once";
}

TEST_F(StateMachineTests, disconnectExecutedAfterFailedConnectState)
{
	State* failedConnectState = new MockState(STATE_IDS::CONNECT, &m_stopRunningFlag, false);
	StateSharedPtr failedConnectStatePtr(failedConnectState);
	m_mockStateFactory.setState(STATE_IDS::CONNECT, failedConnectStatePtr);

	EXPECT_TRUE(startStateMachine());
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_TRUE(stopStateMachine());

	EXPECT_TRUE(isMockStateExecuted(CONNECT)) << "state should be executed more than once";
	EXPECT_FALSE(isMockStateExecuted(SYSQUERY)) << "state should be omitted";
	EXPECT_FALSE(isMockStateExecuted(COMMANDS)) << "state should be omitted";
	EXPECT_TRUE(isMockStateExecuted(DISCONNECT)) << "state should be executed more than once";
}

TEST_F(StateMachineTests, disconnectExecutedAfterFailedSysQueryState)
{
	State* failedSysQueryState = new MockState(STATE_IDS::SYSQUERY, &m_stopRunningFlag, false);
	StateSharedPtr failedSysQueryStatePtr(failedSysQueryState);
	m_mockStateFactory.setState(STATE_IDS::SYSQUERY, failedSysQueryStatePtr);

	EXPECT_TRUE(startStateMachine());
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_TRUE(stopStateMachine());

	EXPECT_TRUE(isMockStateExecuted(CONNECT)) << "state should be executed more than once";
	EXPECT_TRUE(isMockStateExecuted(SYSQUERY)) << "state should be executed more than once";
	EXPECT_FALSE(isMockStateExecuted(COMMANDS)) << "state should be omitted";
	EXPECT_TRUE(isMockStateExecuted(DISCONNECT)) << "state should be executed more than once";
}

TEST_F(StateMachineTests, stopCommandsState)
{
	static bool stateWasExecutedAtLeastOnce = false;
	static bool stateWasStopped = false;

	class StoppableCommandState : public State
	{
	public:
		StoppableCommandState(bool* const stopRunningFlag):State(STATE_IDS::COMMANDS, stopRunningFlag){};
		virtual ~StoppableCommandState(){};

		virtual bool execute()
		{
			while(shouldContinueRunning())
			{
				stateWasExecutedAtLeastOnce = true;
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			stateWasStopped = !shouldContinueRunning();

			return true;
		};
		virtual std::string toString(){return "StoppableCommandState";};
	};

	State* stoppableCommandState = new StoppableCommandState(&m_stopRunningFlag);
	StateSharedPtr state(stoppableCommandState);
	m_mockStateFactory.setState(STATE_IDS::COMMANDS, state);


	// start (and wait for 100ms to get into stoppable commands state)
	EXPECT_TRUE(startStateMachine());
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	m_stopRunningFlag = true;

	// wait a bit to let the stop flag take effect
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_TRUE(stateWasExecutedAtLeastOnce) << "state should be executed more than once";
	EXPECT_TRUE(stateWasStopped) << "state should have been stopped by the stop flag";
}