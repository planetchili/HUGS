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
		Drawable( const TriangleStrip& parent )
			:
			parent( parent )
		{}
		virtual void Rasterize( D3DGraphics& gfx ) const override
		{
			RectI clipI( clip );
			for( auto i = parent.vertices.begin(),end = parent.vertices.end() - 2;
				i != end; i++ )
			{
				gfx.DrawTriangle( trans * *i,trans * *( i + 1 ),trans * *(i + 2),clipI,parent.color );
			}
		}
	private:
		const TriangleStrip& parent;
	};
public:
	TriangleStrip( std::initializer_list< Vec2 > vList,Color color = WHITE )
		:
		vertices( vList ),
		color( color )
	{}
	TriangleStrip( std::vector< const Vec2 >&& movable,Color color = WHITE )
		:
		vertices( movable ),
		color( color )
	{}
	Drawable GetDrawable() const
	{
		return Drawable( *this );
	}
private:
	Color color;
	std::vector< const Vec2 > vertices;
};