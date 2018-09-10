/*
 * ScopedTrue.cpp
 *
 *  Created on: Nov 27, 2017
 *      Author: bfarnham
 */

#include "ScopedTrue.h"

ScopedTrue::ScopedTrue(bool* booleanValue)
:m_booleanValue(booleanValue)
{
	*m_booleanValue = true;
}

ScopedTrue::~ScopedTrue()
{
	*m_booleanValue = false;
}

