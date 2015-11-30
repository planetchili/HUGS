#pragma once
#include "D3DGraphics.h"
#include "Ship.h"
#include <sstream>
#include <iomanip>

class LapDisplay
{
public:
	class Drawable : public ::Drawable
	{
	public:
		Drawable( const LapDisplay& parent )
			:
			parent( parent )
		{}
		virtual void Rasterize( D3DGraphics& gfx ) const override
		{
			std::wstringstream readout;
			readout.setf( std::ios::fixed );
			unsigned int lapNumber = 1;
			for( float time : parent.ship.Timer() )
			{
				readout << L"Lap " << lapNumber << L": "
					<< std::setprecision( 2 ) << time << std::endl;
				lapNumber++;
			}
			readout << L"Lap " << lapNumber << L" (in progress): "
				<< std::setprecision( 2 ) << parent.ship.Timer().GetCurrentLapTime();

			const Vec2 offset = trans.ExtractTranslation();
			gfx.DrawString( readout.str(),parent.pos + offset,parent.font,parent.color );
		}
	private:
		const LapDisplay& parent;
	};
public:
	LapDisplay( const Ship& ship,Vec2 pos )
		:
		ship( ship ),
		pos( pos ),
		font( L"Arial",10.0f,false )
	{}
	Drawable GetDrawable() const
	{
		return Drawable( *this );
	}
private:
	const Color color = BLUE;
	const Font font;
	const Vec2 pos;
	const Ship& ship;
};