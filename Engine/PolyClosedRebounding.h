#pragma once
#include "PolyClosed.h"

class PolyClosedRebounding : public PolyClosed
{
public:
	PolyClosedRebounding( std::vector< Vec2 >&& vList,float facingCoefficient )
		:
		PolyClosed( std::move( vList ),facingCoefficient )
	{}
protected:
	virtual void HandleCollision( PhysicalCircle& obj,Vec2 normal ) const override
	{
		obj.Rebound( normal );
	}
};