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
