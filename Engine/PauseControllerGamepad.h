#pragma once
#include "PauseController.h"
#include "DInput.h"

class PauseControllerGamepad : public PauseController
{
public:
	PauseControllerGamepad( GameScreen& screen,Gamepad& pad )
		:
		PauseController( screen ),
		pad( pad )
	{
		filter.AddCondition( Gamepad::Event( pauseIndex,false ) );
	}
	virtual void Process() override
	{
		pad.ExtractEvents( filter );
		while( !filter.Empty() )
		{
			const auto e = filter.GetEvent();
			if( e.GetType() == Gamepad::Event::Button && e.GetIndex() == pauseIndex
				&& e.IsPressed() )
			{
				screen.Pause();
			}
		}
	}
private:
	const int pauseIndex = 7;
	Gamepad& pad;
	Gamepad::Filter filter;
};