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
