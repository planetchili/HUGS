#pragma once
#include "Ship.h"

class ShipController
{
public:
	ShipController( Ship& ship )
		:
		ship( ship )
	{}
	virtual void Process() = 0;
protected:
	Ship& ship;
};