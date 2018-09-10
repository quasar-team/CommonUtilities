/*
 * MockState.cpp
 *
 *  Created on: Jun 11, 2015
 *      Author: bfarnham
 */
#include "MockState.h"
#include <boost/thread.hpp>

const std::string g_sMockStateInfoString("MockState");

MockState::MockState(const enum STATE_IDS::STATE_ID stateId, bool* const stopRunningFlag, const bool executeReturnValue /* true*/)
:State(stateId, stopRunningFlag), m_executeReturnValue(executeReturnValue), m_executionCounter(0)
{}

MockState::~MockState()
{}

bool MockState::execute()
{
	++m_executionCounter;
	return m_executeReturnValue;
}

std::string MockState::toString()
{
	return g_sMockStateInfoString;
}

size_t MockState::getExecutionCount() const
{
	return m_executionCounter;
}
