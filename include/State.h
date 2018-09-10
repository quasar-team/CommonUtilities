/*
 * State.h
 *
 *  Created on: Oct 1, 2014
 *      Author: bfarnham
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
