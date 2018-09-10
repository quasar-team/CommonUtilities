/*
 * ScopedTrue.h
 *
 *  Created on: Nov 27, 2017
 *      Author: bfarnham
 */

#pragma once

class ScopedTrue
{
public:
	ScopedTrue(bool* booleanValue);
	virtual ~ScopedTrue();
private:
	bool* m_booleanValue;
};
