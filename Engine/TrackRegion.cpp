#include "TrackRegion.h"
#include "Ship.h"
#include <exception>
#include <assert.h>

void TrackRegion::HandleCollision( PhysicalCircle& obj,Vec2 normal ) const
{
	try
	{
		dynamic_cast<Ship&>( obj ).Track( uid );
	}
	catch( std::bad_cast )
	{
		assert( "Non-ship collided with track region" && false );
	}
}