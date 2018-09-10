/*
 * State.cpp
 *
 *  Created on: Jun 11, 2015
 *      Author: bfarnham
 */
#include "State.h"

State::State(const enum STATE_IDS::STATE_ID stateId, bool* const stopRunningFlag)
:m_stateId(stateId), m_stopRunningFlag(stopRunningFlag)
{}

State::~State()
{}

bool State::shouldContinueRunning() const
{
	if(m_stopRunningFlag != 0)
	{
		return !(*m_stopRunningFlag);
	}

	return false;
}
