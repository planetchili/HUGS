#pragma once
#include "ShipController.h"
#include "Keyboard.h"

class ShipControllerKeyboard : public ShipController
{
public:
	ShipControllerKeyboard( Ship& ship,KeyboardClient& kbd )
		:
		ShipController( ship ),
		kbd( kbd ),
		filter( { VK_LEFT,VK_RIGHT,VK_SPACE } )
	{}
	virtual void Process() override
	{
		kbd.ExtractEvents( filter );
		while( !filter.Empty() )
		{
			const auto e = filter.GetEvent();
			switch( e.GetCode() )
			{
			case VK_LEFT:
				if( e.IsPress() )
				{
					ship.Spin( -1.0f );
				}
				else
				{
					ship.StopSpinning( -1.0f );
				}
				break;
			case VK_RIGHT:
				if( e.IsPress() )
				{
					ship.Spin( 1.0f );
				}
				else
				{
					ship.StopSpinning( 1.0f );
				}
				break;
			case VK_SPACE:
				if( e.IsPress() )
				{
					ship.Thrust();
				}
				else
				{
					ship.StopThrusting();
				}
				break;
			}
		}
	}
private:
	KeyboardFilter filter;
	KeyboardClient& kbd;
};