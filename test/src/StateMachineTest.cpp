/*
 * StateMachineTest.cpp
 *
 *  Created on: Jun 11, 2015
 *      Author: bfarnham
 */
#include "StateMachineTest.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <LogIt.h>
#include <LogLevels.h>

using namespace STATE_IDS;

extern Log::LogComponentHandle STATES = Log::INVALID_HANDLE;

StateMachineTests::StateMachineThread::StateMachineThread(StateMachine& stateMachine)
:m_stateMachine(stateMachine), m_state(CONSTRUCTED)
{}

void StateMachineTests::StateMachineThread::operator()()
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_state = WAITING_FOR_START;

	try
	{
		m_wait.wait(lock);
		m_state = STARTED;

		m_stateMachine.run(0);
	}
	catch(...)
	{
	    LOG(Log::INF) << "State machine thread exception (probably interrupt), exiting";
	}

	m_state = COMPLETED;
}

bool StateMachineTests::StateMachineThread::start(const size_t& postStartPauseMs)
{
	if(waitForThreadState(StateMachineThread::WAITING_FOR_START))
	{
		m_wait.notify_all();
		if(waitForThreadState(StateMachineThread::STARTED))
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(postStartPauseMs));
			return true;
		}
	}

	return false;
}

bool StateMachineTests::StateMachineThread::waitForThreadState(const THREAD_STATE& targetState)
{
	const size_t snoozeMs = 10;
	const size_t maxWait = snoozeMs * 20;
	size_t elapsedWait = 0;
	
	//EDIT BY DAMIAN: I removed __func__  (everywhere in this file) as this functionality doesn't work on visual studio compiler. This information is already included by logit anyway.
	//LOG(Log::DBG) << __func__ << " waiting for target state ["<<targetState<<"], current state ["<< m_state <<"]";
	LOG(Log::DBG) << " waiting for target state ["<<targetState<<"], current state ["<< m_state <<"]";
	while(m_state != targetState && elapsedWait < maxWait)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(snoozeMs));
		//LOG(Log::DBG) << __func__ << " post snooze state ["<< m_state <<"] elapsed ["<<elapsedWait<<"]";
		LOG(Log::DBG) << " post snooze state ["<< m_state <<"] elapsed ["<<elapsedWait<<"]";
		elapsedWait += snoozeMs;
	}

	//LOG(Log::DBG) << __func__ << " wait complete target state ["<<targetState<<"], current state ["<< m_state <<"]";
	LOG(Log::DBG) << " wait complete target state ["<<targetState<<"], current state ["<< m_state <<"]";
	return targetState == m_state;
}

StateMachineTests::StateMachineTests()
:m_stopRunningFlag(false), m_mockStateFactory(&m_stopRunningFlag), m_testee(m_mockStateFactory), m_stateMachineThreadObj(m_testee), m_stateMachineThread(boost::ref(m_stateMachineThreadObj))
{
	Log::initializeLogging(Log::INF);
}

bool StateMachineTests::interruptStateMachineThreadAndWaitForExit()
{
    if(m_stateMachineThreadObj.m_state != StateMachineThread::COMPLETED)
    {
        m_stateMachineThread.interrupt();
        waitForStateMachineThreadToExit();
    }

    return m_stateMachineThreadObj.m_state == StateMachineThread::COMPLETED;
}

void StateMachineTests::TearDown()
{
    interruptStateMachineThreadAndWaitForExit();
}

bool StateMachineTests::isMockStateExecuted(const STATE_IDS::STATE_ID& stateId)
{
	std::shared_ptr<State> state = m_mockStateFactory.getState(stateId);

	if(!state)
	{
		LOG(Log::ERR) << "No state!";
		return false;
	}

	State* rawState = state.get();
	if(!rawState)
	{
		LOG(Log::ERR) << "null state";
		return false;
	}

	MockState* mockState = dynamic_cast<MockState*>(rawState);
	if(!mockState)
	{
		LOG(Log::ERR) << "invalid state type";
		return false;
	}

	return mockState->getExecutionCount() > 0;
}

void StateMachineTests::waitForStateMachineThreadToExit()
{
	try
	{
		m_stateMachineThread.timed_join(boost::posix_time::milliseconds(1000));
	}
	catch(boost::thread_interrupted)
	{
		//LOG(Log::ERR) << __func__ << " Worker thread failed to exit within ["<<1000<<"ms]";
		LOG(Log::ERR) << " Worker thread failed to exit within ["<<1000<<"ms]";
	}
}


TEST_F(StateMachineTests, stateMachineRun)
{
	m_stateMachineThreadObj.start(100);

	EXPECT_TRUE(m_testee.isRunning()) << "state machine should now be running";
}

TEST_F(StateMachineTests, stateMachineStop)
{
	m_stateMachineThreadObj.start(100);

	m_testee.stop();
	waitForStateMachineThreadToExit();

	EXPECT_FALSE(m_testee.isRunning()) << "state machine should have stopped";
}

TEST_F(StateMachineTests, targetStatesExecuted)
{
	m_stateMachineThreadObj.start(100);
	EXPECT_TRUE(interruptStateMachineThreadAndWaitForExit());

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

	m_stateMachineThreadObj.start(100);
	EXPECT_TRUE(interruptStateMachineThreadAndWaitForExit());

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

	m_stateMachineThreadObj.start(100);
	EXPECT_TRUE(interruptStateMachineThreadAndWaitForExit());

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
				boost::this_thread::sleep(boost::posix_time::milliseconds(10));
			}
			stateWasStopped = !shouldContinueRunning();

			return true;
		};
		virtual std::string toString(){return "StoppableCommandState";};
	};

	State* stoppableCommandState = new StoppableCommandState(&m_stopRunningFlag);
	StateSharedPtr state(stoppableCommandState);
	m_mockStateFactory.setState(STATE_IDS::COMMANDS, state);


	// start (and wait for 100ms to get started)
	m_stopRunningFlag = false;
	m_stateMachineThreadObj.start(100);
	m_stopRunningFlag = true;

	// wait a bit to let the stop flag take effect
	boost::this_thread::sleep(boost::posix_time::milliseconds(100));

	EXPECT_TRUE(stateWasExecutedAtLeastOnce) << "state should be executed more than once";
	EXPECT_TRUE(stateWasStopped) << "state should have been stopped by the stop flag";
}
