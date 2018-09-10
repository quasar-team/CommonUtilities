#pragma once

#include <string>

namespace UtilityFunctions
{
	bool isDottedQuadIpAddress(const std::string& address);
	std::string convertAddressToIp(const std::string& address);
}