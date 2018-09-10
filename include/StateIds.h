/*
 * StateIds.h
 *
 *  Created on: Oct 1, 2014
 *      Author: bfarnham
 */

#ifndef SRC_STATEIDS_H_
#define SRC_STATEIDS_H_

#include <string>

namespace STATE_IDS
{
	enum STATE_ID {CONNECT, SYSQUERY, COMMANDS, DISCONNECT, END};

	std::string toString(const STATE_ID& id);
}



#endif /* SRC_STATEIDS_H_ */
