#pragma once
#include "PolyClosed.h"

class TrackRegion : public PolyClosed
{
public:
	bool operator<( const TrackRegion& rhs ) const
	{
		return uid < rhs.uid;
	}
	TrackRegion( std::vector< const Vec2 >&& vList,unsigned int uid )
		:
		PolyClosed( std::move( vList ),PolyClosed::MakeOutwardCoefficient() ),
		uid( uid )
	{}
	unsigned int GetID() const
	{
		return uid;
	}
protected:
	virtual void HandleCollision( CollidableCircle& obj,Vec2 normal ) const override
	{
		obj.Track( uid );
	}
private:
	const unsigned int uid;
};