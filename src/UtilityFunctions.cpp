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
#include "UtilityFunctions.h"

#include <iostream>
#include <cstring>
#include <memory>
#include <thread>
#include <LogIt.h>
#include <regex>

#ifdef _WIN32
	#include <WS2tcpip.h>
#else // assume Linux
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>
#endif

using std::string;
using std::ostringstream;
using std::endl;

#ifdef _WIN32
bool startWinsock()
{
	// wrap function in critical section - don't want multiple threads starting Winsock.
	static std::mutex g_sStartWinsockMutex;
	std::lock_guard<std::mutex> scopedLock(g_sStartWinsockMutex);

	static bool g_sWinsockStarted = false;
	if (g_sWinsockStarted) return true;

	const WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	const int err = WSAStartup(wVersionRequested, &wsaData);

	g_sWinsockStarted = (err == 0);
	if (!g_sWinsockStarted) 
	{
		LOG(Log::ERR) << __FUNCTION__ << " Failed to start winsock, error code ["<<err<<"]";
	}
	else
	{
		LOG(Log::DBG) << __FUNCTION__ << " Started winsock";
	}

	return g_sWinsockStarted;
}
#endif

bool UtilityFunctions::isDottedQuadIpAddress(const std::string& address)
{
	static const std::regex ipRegex("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");
	return std::regex_match(address, ipRegex);
}

std::string UtilityFunctions::convertAddressToIp(const std::string& address)
{
	if (isDottedQuadIpAddress(address)) return address;

	LOG(Log::INF) << __FUNCTION__ << " converting address string [" << address << "] to IPv4 dotted quad";
	struct addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

#ifdef _WIN32
	if (!startWinsock())
	{
		LOG(Log::ERR) << __FUNCTION__ << " Failed to start winsock to resolve address ["<<address<<"] to IP. Returning IP [ERROR!]";
		return "ERROR!";
	}
#endif

	struct addrinfo *result = 0;
	const int returnCode = getaddrinfo(address.c_str(), NULL, &hints, &result);
	std::shared_ptr<struct addrinfo> resultPtr(result, &freeaddrinfo);
	if (returnCode != 0)
	{
		ostringstream msg;
		msg << "getaddrinfo failed, with return code [" << returnCode << "], error msg: " << gai_strerror(returnCode) << endl;
		LOG(Log::ERR) << msg.str();
		throw std::runtime_error(msg.str());
	}
	struct sockaddr_in* sockAddr = (struct sockaddr_in*)(resultPtr->ai_addr);
	const string ipAddress(inet_ntoa(sockAddr->sin_addr));
	LOG(Log::INF) << __FUNCTION__ << " converted address string [" << address << "] to IPv4 dotted quad [" << ipAddress << "]";
	return ipAddress;
}
