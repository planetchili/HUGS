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
		filter( { VK_LEFT,VK_RIGHT,VK_SPACE,VK_UP,VK_DOWN } )
	{}
	virtual void Process() override
	{
		kbd.ExtractEvents( filter );
		while( !filter.Empty() )
		{
			const auto e = filter.GetEvent();
			switch( e.GetCode() )
			{
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
			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
				{
					Vec2 dir = { 0.0f,0.0f };
					if( kbd.KeyIsPressed( VK_UP ) )
					{
						dir += { 0.0f,-1.0f };
					}
					if( kbd.KeyIsPressed( VK_DOWN ) )
					{
						dir += { 0.0f,1.0f };
					}
					if( kbd.KeyIsPressed( VK_LEFT ) )
					{
						dir += { -1.0f,0.0f };
					}
					if( kbd.KeyIsPressed( VK_RIGHT ) )
					{
						dir += { 1.0f,0.0f };
					}
					if( dir != Vec2 { 0.0f,0.0f } )
					{
						ship.Spin( dir.Normalize() );
					}
				}
			}
		}
	}
private:
	KeyboardFilter filter;
	KeyboardClient& kbd;
};