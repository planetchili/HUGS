#pragma once
#include <vector>
#include "TrackRegion.h"

class TrackRegionManager
{
public:
	void AddRegion( std::vector< const Vec2 >&& vList,unsigned int uid )
	{
		regions.emplace_back( std::move( vList ),uid );
		std::sort( regions.begin(),regions.end() );
	}
	unsigned int GetRegionCount() const
	{
		return regions.size();
	}
	bool IDsAreContiguous() const
	{
		for( unsigned int i = 0; i < regions.size(); i++ )
		{
			if( regions[i].GetID() != i )
			{
				return false;
			}
		}
		return true;
	}
	void TestCollision( CollidableCircle& obj ) const
	{
		for( const TrackRegion& r : regions )
		{
			r.TestCollision( obj );
		}
	}
private:
	std::vector< TrackRegion > regions;
};