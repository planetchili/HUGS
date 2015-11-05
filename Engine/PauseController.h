#pragma once
#include "GameScreen.h"

class PauseController
{
public:
	PauseController( GameScreen& screen )
		:
		screen( screen )
	{}
	virtual void Process() = 0;
protected:
	GameScreen& screen;
};