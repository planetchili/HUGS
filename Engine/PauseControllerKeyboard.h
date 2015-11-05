#pragma once
#include "PauseController.h"
#include "Keyboard.h"

class PauseControllerKeyboard : public PauseController
{
public:
	PauseControllerKeyboard( GameScreen& screen,KeyboardClient& kbd )
		:
		PauseController( screen ),
		kbd( kbd ),
		filter( { VK_ESCAPE } )
	{}
	virtual void Process() override
	{
		kbd.ExtractEvents( filter );
		while( !filter.Empty() )
		{
			const auto e = filter.GetEvent();
			if( e.GetCode() == VK_ESCAPE && e.IsPress() )
			{
				screen.Pause();
			}
		}
	}
private:
	KeyboardFilter filter;
	KeyboardClient& kbd;
};