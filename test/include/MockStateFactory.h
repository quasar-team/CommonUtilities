/*
 * MockStateFactory.h
 *
 *  Created on: Jun 11, 2015
 *      Author: bfarnham
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
