#pragma once
#include "PolyClosed.h"
#include "TriangleStrip.h"

class Map
{
public:
	Map( std::string filename )
		:
		boundaries( filename ),
		model(boundaries.ExtractStripVertices( wallWidth ))
	{}
	TriangleStrip::Drawable GetDrawable( ) const
	{
		return model.GetDrawable();
	}
	void HandleCollision( CollidableCircle& obj )
	{
		boundaries.HandleCollision( obj );
	}

private:
	const float wallWidth = 40.0f;
	PolyClosed boundaries;
	TriangleStrip model;
};