#pragma once
#include "PolyClosedPhysical.h"

class TrackRegion : public PolyClosedPhysical
{
public:
	bool operator<( const TrackRegion& rhs ) const
	{
		return uid < rhs.uid;
	}
	TrackRegion( std::vector< Vec2 >&& vList,unsigned int uid )
		:
		PolyClosedPhysical( std::move( vList ),PolyClosed::MakeOutwardCoefficient() ),
		uid( uid )
	{}
	unsigned int GetID() const
	{
		return uid;
	}
protected:
	virtual void HandleCollision( PhysicalCircle& obj,Vec2 normal ) const override;
private:
	const unsigned int uid;
};