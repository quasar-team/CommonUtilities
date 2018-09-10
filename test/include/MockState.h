/*
 * MockState.h
 *
 *  Created on: Jun 11, 2015
 *      Author: bfarnham
 */

#ifndef MOCKSTATE_H_
#define MOCKSTATE_H_

#include "State.h"

class MockState : public State
{
public:
	MockState(const enum STATE_IDS::STATE_ID stateId, bool* const stopRunningFlag, const bool executeReturnValue = true);
	virtual ~MockState();

	virtual bool execute();
	virtual std::string toString();

	size_t getExecutionCount() const;

	const bool m_executeReturnValue;

private:
	size_t m_executionCounter;
};

#endif /* MOCKSTATE_H_ */
