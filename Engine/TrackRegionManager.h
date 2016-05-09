#pragma once
#include <set>
#include "TrackRegion.h"

class TrackRegionManager
{
public:
	void AddRegion( std::vector< Vec2 >&& vList,unsigned int uid )
	{
		regions.emplace( std::move( vList ),uid );
	}
	std::set< TrackRegion >::const_iterator GetStart() const
	{
		return regions.cbegin();
	}
	std::set< TrackRegion >::const_iterator GetEnd() const
	{
		return regions.cend();
	}
	void TestCollision( PhysicalCircle& obj ) const
	{
		for( const TrackRegion& r : regions )
		{
			r.TestCollision( obj );
		}
	}
private:
	std::set< TrackRegion > regions;
};