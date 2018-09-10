/*
 * MockStateFactory.cpp
 *
 *  Created on: Jun 11, 2015
 *      Author: bfarnham
 */
#include "MockStateFactory.h"
#include <boost/assign/list_of.hpp>

#include "MockState.h"

using boost::shared_ptr;
using boost::assign::map_list_of;
using boost::assign::list_of;

MockStateFactory::MockStateFactory(bool* const stopRunningFlag)
{
	m_states[STATE_IDS::CONNECT] = StateSharedPtr(new MockState(STATE_IDS::CONNECT, stopRunningFlag));
	m_states[STATE_IDS::SYSQUERY] = StateSharedPtr(new MockState(STATE_IDS::SYSQUERY, stopRunningFlag));
	m_states[STATE_IDS::COMMANDS] = StateSharedPtr(new MockState(STATE_IDS::COMMANDS, stopRunningFlag));
	m_states[STATE_IDS::DISCONNECT] = StateSharedPtr(new MockState(STATE_IDS::DISCONNECT, stopRunningFlag));
}

MockStateFactory::~MockStateFactory()
{}

StateSharedPtr MockStateFactory::getState(const STATE_IDS::STATE_ID& stateId)
{
	return m_states[stateId];
}

void MockStateFactory::setState(const STATE_IDS::STATE_ID& stateId, StateSharedPtr state)
{
	m_states[stateId] = state;
}
