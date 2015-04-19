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
			const Color meterColor = 
				{ unsigned char(255 * (1.0f - parent.ship.GetShieldPercent())),0,0 };
		}
	private:
		const ShieldMeter& parent;
	};
public:
private:
	const int borderWidth = 5;
	const int spaceWidth = 3;
	const Color borderColor = GRAY;
	const RectI region;
	const Ship& ship;
};