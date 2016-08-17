#pragma once
#include "Drawable.h"
#include "Ship.h"

class ShieldMeter
{
private:
	class Drawable : public ::Drawable
	{
	public:
		Drawable( const ShieldMeter& parent )
			:
			parent( parent )
		{}
		virtual void Rasterize( D3DGraphics& gfx ) const override
		{
			const Vei2 offset = trans.ExtractTranslation();
			const Color meterColor = 
				{ 0u,255u,unsigned char( 255 * parent.ship.GetShieldPercent() ),
				unsigned char( 255 * parent.ship.GetShieldPercent() ) };
			const int meterWidth = int( ( ( parent.region.right - parent.region.left ) -
				( 2 * parent.borderWidth + 2 * parent.spaceWidth ) ) * parent.ship.GetShieldPercent() );
			gfx.DrawRectangle<SolidRasterizer>( parent.region.left + offset.x,parent.region.right + offset.x,
				parent.region.top + offset.y,parent.region.top + parent.borderWidth + offset.y,
				parent.borderColor );
			gfx.DrawRectangle<SolidRasterizer>( parent.region.left + offset.x,parent.region.left + parent.borderWidth + offset.x,
				parent.region.top + parent.borderWidth + offset.y,
				parent.region.bottom - parent.borderWidth + offset.y,parent.borderColor );
			gfx.DrawRectangle<SolidRasterizer>( parent.region.right - parent.borderWidth + offset.x,parent.region.right + offset.x,
				parent.region.top + parent.borderWidth + offset.y,parent.region.bottom - parent.borderWidth + offset.y,
				parent.borderColor );
			gfx.DrawRectangle<SolidRasterizer>( parent.region.left + offset.x,parent.region.right + offset.x,
				parent.region.bottom - parent.borderWidth + offset.y,parent.region.bottom + offset.y,parent.borderColor );
			gfx.DrawRectangle<SolidRasterizer>( parent.region.left + parent.borderWidth + parent.spaceWidth + offset.x,
				parent.region.left + parent.borderWidth + parent.spaceWidth + meterWidth + offset.x,
				parent.region.top + parent.borderWidth + parent.spaceWidth + offset.y,
				parent.region.bottom - ( parent.borderWidth + parent.spaceWidth ) + offset.y,meterColor );
		}
	private:
		const ShieldMeter& parent;
	};
public:
	ShieldMeter( const RectI region,const Ship& ship )
		:
		region( region ),
		ship( ship )
	{}
	Drawable GetDrawable() const
	{
		return Drawable( *this );
	}
private:
	const int borderWidth = 2;
	const int spaceWidth = 5;
	const Color borderColor = Color { WHITE,0u };
	const RectI region;
	const Ship& ship;
};