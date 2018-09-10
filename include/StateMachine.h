/*
 * StateMachine.h
 *
 *  Created on: Oct 1, 2014
 *      Author: bfarnham
 */

#ifndef SRC_STATEMACHINE_H_
#define SRC_STATEMACHINE_H_

#include "StateIds.h"
#include "State.h"
#include "StateFactory.h"

class StateMachine
{
public:
	StateMachine(StateFactory& stateFactory);
	virtual ~StateMachine();

	void run(const size_t& reconnectionPauseMs);
	void stop();
	bool isRunning() const;

private:
	bool executeState(const enum STATE_IDS::STATE_ID& stateId) const;
	void handleMessyExit(enum STATE_IDS::STATE_ID& currentStateId);

private:
	enum STATE_IDS::STATE_ID getNextStateId(const enum STATE_IDS::STATE_ID& currentStateId) const;

	StateFactory& m_stateFactory;

	bool m_stopRunning;
	bool m_isRunning;
};

#endif /* SRC_STATEMACHINE_H_ */
