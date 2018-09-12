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
#ifndef MOCKSTATEFACTORY_H_
#define MOCKSTATEFACTORY_H_

#include <map>
#include "TypeDefinitions.h"
#include "StateFactory.h"

class MockStateFactory : public StateFactory
{
public:
	MockStateFactory(bool* const stopRunningFlag);
	virtual ~MockStateFactory();

	void setState(const STATE_IDS::STATE_ID& stateId, StateSharedPtr state);
	virtual StateSharedPtr getState(const STATE_IDS::STATE_ID& stateId);

private:
	std::map<STATE_IDS::STATE_ID, StateSharedPtr > m_states;
};

#endif /* MOCKSTATEFACTORY_H_ */
