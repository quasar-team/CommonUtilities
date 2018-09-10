/*
 * ScopedTrueTest.cpp
 *
 *  Created on: Nov 27, 2017
 *      Author: bfarnham
 */

#include "gtest/gtest.h"
#include "ScopedTrue.h"

TEST(ScopedTrueTest, testSetsValueAsScopeChanges)
{
	bool testee = false;

	{
		EXPECT_FALSE(testee) << "expect initial value to be false";
		ScopedTrue localScope(&testee);
		EXPECT_TRUE(testee) << "expect scoped true to set value to true";
	}
	EXPECT_FALSE(testee) << "expect scoped true to set value to true";

	try
	{
		ScopedTrue tryCatchScope(&testee);
		EXPECT_TRUE(testee) << "expect scoped true before exception";
		throw(std::runtime_error("bof! something bad happened"));
		FAIL() << "should have thrown exception";
	}
	catch(const std::runtime_error& e) {} // don't actually care
	EXPECT_FALSE(testee);
}
