#include "CollidableCircle.h"
#include "PhysicalCircle.h"

void CollidableCircle::TestCollision( PhysicalCircle& other )
{
	if( ( pos - other.pos ).LenSq() < sq( radius + other.radius ) )
	{
		HandleCollision( other );
	}
}