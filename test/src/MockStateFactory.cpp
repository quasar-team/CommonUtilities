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
#include "MockStateFactory.h"
#include <boost/assign/list_of.hpp>

#include "MockState.h"

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
