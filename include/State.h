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
#ifndef SRC_STATE_H_
#define SRC_STATE_H_

#include "StateIds.h"

class State
{
public:
	State(const enum STATE_IDS::STATE_ID stateId, bool* const stopRunningFlag);
	virtual ~State();

	virtual bool execute() = 0;
	virtual std::string toString() = 0;

	bool shouldContinueRunning() const;

	const enum STATE_IDS::STATE_ID m_stateId;

protected:
	bool* const m_stopRunningFlag;
};



#endif /* SRC_STATE_H_ */
