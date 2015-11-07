#pragma once
#include "ShipController.h"
#include "DInput.h"

class ShipControllerGamepad : public ShipController
{
public:
	ShipControllerGamepad( Ship& ship,Gamepad& pad )
		:
		ShipController( ship ),
		pad( pad )	
	{
		filter.AddCondition( Gamepad::Event( Gamepad::Analog1,{ 0.0f,0.0f } ) );
		filter.AddCondition( Gamepad::Event( thrustIndex,false ) );
	}
	virtual void Process() override
	{
		pad.ExtractEvents( filter );
		while( !filter.Empty() )
		{
			const auto e = filter.GetEvent();
			if( e.GetType() == e.Stick && e.GetIndex() == Gamepad::Analog1 )
			{
				Vec2 dir = e.GetStickPos();
				if( dir != Vec2 { 0.0f,0.0f } )
				{
					ship.Spin( dir.Normalize() );
				}
			}
			else if( e.GetType() == e.Button && e.GetIndex() == thrustIndex )
			{
				if( e.IsPressed() )
				{
					ship.Thrust();
				}
				else
				{
					ship.StopThrusting();
				}
			}
		}
	}
private:
	const int thrustIndex = 1;
	Gamepad& pad;
	Gamepad::Filter filter;
};