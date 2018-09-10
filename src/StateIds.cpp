#include "StateIds.h"

const std::string STATE_ID_NAMES[] = {"CONNECT", "SYSQUERY", "COMMANDS", "DISCONNECT", "END"};

std::string STATE_IDS::toString(const STATE_IDS::STATE_ID& id)
{
	if(id >= CONNECT && id <=END) return STATE_ID_NAMES[id];

	return "ERROR - UNKNOWN STATE";
}
