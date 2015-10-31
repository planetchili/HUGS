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
	{}
	virtual void Process() override
	{
		while( !pad.IsEmpty() )
		{
			const auto e = pad.ReadEvent();
			if( e.GetType() == e.Stick && e.GetIndex() == 0 )
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
			else if( e.GetType() == e.Button && e.GetIndex() == 1 )
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
	Gamepad& pad;
};