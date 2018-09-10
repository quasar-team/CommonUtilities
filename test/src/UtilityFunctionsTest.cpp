/*
* ScopedTrueTest.cpp
*
*  Created on: Nov 27, 2017
*      Author: bfarnham
*/

#include "gtest/gtest.h"
#include "UtilityFunctions.h"

TEST(UtilityFunctionsTest, testIsDottedQuadIpAddress)
{
	EXPECT_TRUE(UtilityFunctions::isDottedQuadIpAddress("127.0.0.1"));
	EXPECT_FALSE(UtilityFunctions::isDottedQuadIpAddress("not.an.ip.address"));
}

TEST(UtilityFunctionsTest, testConvertAddressToIpCanConvertLocalhost)
{
	for (size_t i = 0; i < 10; ++i)
	{
		EXPECT_EQ("127.0.0.1", UtilityFunctions::convertAddressToIp("localhost"));
	}
}