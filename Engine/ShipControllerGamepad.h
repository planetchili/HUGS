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
		filter.AddCondition( Gamepad::Event( Gamepad::Dpad,{ 0.0f,0.0f } ) );
		filter.AddCondition( Gamepad::Event( thrustIndex,false ) );
	}
	virtual void Process() override
	{
		pad.ExtractEvents( filter );
		while( !filter.Empty() )
		{
			const auto e = filter.GetEvent();
			if( e.GetType() == e.Stick && e.GetIndex() == Gamepad::Dpad )
			{
				if( e.GetStickPos().x < 0.0f )
				{
					ship.Spin( -1.0f );
				}
				else if( e.GetStickPos().x > 0.0f )
				{
					ship.Spin( 1.0f );
				}
				else
				{
					ship.StopSpinning( 1.0f );
					ship.StopSpinning( -1.0f );
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
	const int thrustIndex = 0;
	Gamepad& pad;
	Gamepad::Filter filter;
};