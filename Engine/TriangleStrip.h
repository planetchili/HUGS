#pragma once
#include "Vec2.h"
#include "Colors.h"
#include "D3DGraphics.h"
#include "Mat3.h"
#include "Drawable.h"
#include <vector>
#include <memory>
#include <algorithm>


class TriangleStrip
{
public:
	class Drawable : public ::Drawable
	{
	public:
		Drawable( const TriangleStrip& parent,Color c )
			:
			parent( parent ),
			c( c )
		{}
		virtual void Rasterize( D3DGraphics& gfx ) const override
		{
			RectI clipI( clip );
			for( auto i = parent.vertices.begin(),end = parent.vertices.end() - 2;
				i != end; i++ )
			{
				gfx.DrawTriangle( trans * *i,trans * *( i + 1 ),trans * *(i + 2),clipI,c );
			}
		}
	private:
		const Color c;
		const TriangleStrip& parent;
	};
public:
	TriangleStrip( std::initializer_list< Vec2 > vList )
		:
		vertices( vList )
	{}
	TriangleStrip( std::vector< Vec2 >&& movable )
		:
		vertices( movable )
	{}
	Drawable GetDrawable( Color c = WHITE ) const
	{
		return Drawable( *this,c );
	}
private:
	std::vector< Vec2 > vertices;
};