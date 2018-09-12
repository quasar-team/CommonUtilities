/*
 * StateMachine.cpp
 *
 *  Created on: Oct 1, 2014
 *      Author: bfarnham
 */
#include "StateMachine.h"
#include <LogIt.h>
#include <LogItComponentIds.h>
#include <chrono>
#include "ScopedTrue.h"

using std::this_thread::get_id;
using std::chrono::milliseconds;

StateMachine::StateMachine(StateFactory& stateFactory)
:m_stateFactory(stateFactory), m_stopRunning(false), m_isRunning(false)
{}

StateMachine::~StateMachine()
{}

void StateMachine::run(const size_t& reconnectionPauseMs)
{
	LOG(Log::INF, STATES)<<__FUNCTION__<<" thread id ["<<get_id()<<"], state machine starting, reconnection pause set at ["<<reconnectionPauseMs<<"ms]";
	ScopedTrue scopedTrue(&m_isRunning);

	for(STATE_IDS::STATE_ID currentStateId = STATE_IDS::CONNECT; !m_stopRunning;)
	{
		if(!executeState(currentStateId))
		{
			handleMessyExit(currentStateId);
		}

		const STATE_IDS::STATE_ID nextStateId = getNextStateId(currentStateId);

		if(currentStateId == STATE_IDS::DISCONNECT && nextStateId == STATE_IDS::CONNECT && !m_stopRunning)
		{
			LOG(Log::INF, STATES)<<__FUNCTION__<<" thread id ["<<get_id()<<"], making transition from DISCONNECT -> RECONNECT, i.e. a reconnection. Sleeping thread for ["<<reconnectionPauseMs<<"s] to avoid thrashing";
			std::this_thread::sleep_for(milliseconds(reconnectionPauseMs));
		}

		currentStateId = nextStateId;
	}

	LOG(Log::INF, STATES)<<__FUNCTION__<<" thread id ["<<get_id()<<"], state machine exiting, stopRunning ["<<m_stopRunning<<"]";
}

enum STATE_IDS::STATE_ID StateMachine::getNextStateId(const enum STATE_IDS::STATE_ID& currentStateId) const
{
	if(currentStateId == STATE_IDS::DISCONNECT)
	{
		return STATE_IDS::CONNECT;
	}
	else
	{
		return static_cast<STATE_IDS::STATE_ID>(currentStateId+1);
	}
}

bool StateMachine::executeState(const enum STATE_IDS::STATE_ID& stateId) const
{
	std::shared_ptr<State> state = m_stateFactory.getState(stateId);

	LOG(Log::DBG, STATES)<<__FUNCTION__<<"thread id ["<<get_id()<<"],  starting execution for state id ["<<STATE_IDS::toString(stateId)<<"] object ["<<state->toString()<<"]";

	const bool isStateCompletedOk = state->execute();
	if(!isStateCompletedOk)
	{
		LOG(Log::WRN, STATES)<<__FUNCTION__<<" thread id ["<<get_id()<<"], failure during execution of state ["<<state->toString()<<"]";
	}
	else
	{
		LOG(Log::INF, STATES)<<__FUNCTION__<<" thread id ["<<get_id()<<"], state ["<<state->toString()<<"] completed [OK]";
	}

	return isStateCompletedOk;
}

void StateMachine::stop()
{
	m_stopRunning = true;
}

bool StateMachine::isRunning() const
{
	return m_isRunning;
}

void StateMachine::handleMessyExit(enum STATE_IDS::STATE_ID& currentStateId)
{
	LOG(Log::WRN, STATES) << __FUNCTION__ << " thread id ["<<get_id()<<"], handling a messy exit from failed state ["<<STATE_IDS::toString(currentStateId)<<"]";

	if(currentStateId != STATE_IDS::DISCONNECT)
	{
		LOG(Log::WRN, STATES) << __FUNCTION__ << " thread id ["<<get_id()<<"], running disconnect state, trying to make clean exit";
		currentStateId = STATE_IDS::DISCONNECT;
		const bool disconnected = executeState(currentStateId);
		LOG(Log::WRN, STATES) << __FUNCTION__ << " thread id ["<<get_id()<<"], disconnect state success? ["<<(disconnected?'Y':'N')<<"]";
	}
}
