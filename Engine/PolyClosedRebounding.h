#pragma once
#include "PolyClosedPhysical.h"

class PolyClosedRebounding : public PolyClosedPhysical
{
public:
	PolyClosedRebounding( std::vector< Vec2 >&& vList,float facingCoefficient )
		:
		PolyClosedPhysical( std::move( vList ),facingCoefficient )
	{}
protected:
	virtual void HandleCollision( PhysicalCircle& obj,Vec2 normal ) const override
	{
		obj.Rebound( normal );
	}
};