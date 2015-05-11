#pragma once
#include "PolyClosed.h"

class PolyClosedRebounding : public PolyClosed
{
public:
	PolyClosedRebounding( std::vector< const Vec2 >&& vList,float facingCoefficient )
		:
		PolyClosed( std::move( vList ),facingCoefficient )
	{}
protected:
	virtual void HandleCollision( CollidableCircle& obj,Vec2 normal ) override
	{
		obj.Rebound( normal );
	}
};