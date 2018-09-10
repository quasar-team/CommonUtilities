#include "UtilityFunctions.h"

#include <iostream>
#include<thread>
#include <LogIt.h>
#include <boost/regex.hpp>

#ifdef _WIN32
	#include <WS2tcpip.h>
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
	static const boost::regex ipRegex("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");
	return boost::regex_match(address, ipRegex);
}

std::string UtilityFunctions::convertAddressToIp(const std::string& address)
{
	if (isDottedQuadIpAddress(address)) return address;

	LOG(Log::INF) << __FUNCTION__ << " converting address string [" << address << "] to IPv4 dotted quad";
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
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
	boost::shared_ptr<struct addrinfo> resultPtr(result, &freeaddrinfo);
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