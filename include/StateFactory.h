/*
 * StateFactory.h
 *
 *  Created on: Jun 10, 2015
 *      Author: bfarnham
 */

#ifndef STATEFACTORY_H_
#define STATEFACTORY_H_

#include <memory>

#include "StateIds.h"
#include "State.h"

class StateFactory
{
public:
	StateFactory();
	virtual ~StateFactory();

	virtual std::shared_ptr<State> getState(const STATE_IDS::STATE_ID& stateId) = 0;
};



#endif /* STATEFACTORY_H_ */
