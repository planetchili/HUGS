#pragma once
#include "D3DGraphics.h"
#include "TriangleStrip.h"
#include <array>

class TexturedQuad
{
public:
	class Drawable : public ::Drawable
	{
	public:
		Drawable( const TexturedQuad& parent )
			:
			parent( parent )
		{}
		virtual void Rasterize( D3DGraphics& gfx ) const override
		{
			std::array<Vertex,4 > quadTrans;
			for( int i = 0; i < 4; i++ )
			{
				quadTrans[i].t = parent.quad[i].t;
				quadTrans[i].v = trans * parent.quad[i].v;
			}

			gfx.DrawTriangleTex( quadTrans[0],quadTrans[1],quadTrans[3],clip,
				parent.texture );
			gfx.DrawTriangleTex( quadTrans[1],quadTrans[2],quadTrans[3],clip,
				parent.texture );
		}
	private:
		const TexturedQuad& parent;
	};
public:
	TexturedQuad( const std::wstring& filename,float scale = 1.0f,Vec2 center = { 0.0f,0.0f } )
		:
		texture( Surface::FromFile( filename ) )
	{
		const float xOff = float( texture.GetWidth() ) / 2.0f;
		const float yOff = float( texture.GetHeight() ) / 2.0f;
		quad[0].v = { -xOff,-yOff };
		quad[0].t = { 0.0f,0.0f };
		quad[1].v = { xOff,-yOff };
		quad[1].t = { float( texture.GetWidth() - 1 ),0.0f };
		quad[2].v = { xOff,yOff };
		quad[2].t = { float( texture.GetWidth() - 1 ),float( texture.GetHeight() - 1 ) };
		quad[3].v = { -xOff,yOff };
		quad[3].t = { 0.0f,float( texture.GetHeight() - 1 ) };
		for( Vertex& v : quad )
		{
			v.v = ( v.v - center ) * scale;
		}
	}
	TexturedQuad( TexturedQuad&& donor )
		:
		texture( std::move( donor.texture ) ),
		quad( std::move( donor.quad ) )
	{}
	Drawable GetDrawable() const
	{
		return Drawable( *this );
	}
private:
	Surface texture;
	std::array<Vertex,4> quad;
};