#pragma once
#include "Dinput.h"
#include "Keyboard.h"
#include "Mouse.h"

class InputSystem
{
public:
	InputSystem( HWND hWnd,MouseServer& mouseServer,KeyboardServer& kbdServer )
		:
		di( hWnd ),
		mouse( mouseServer ),
		kbd( kbdServer )
	{}
public:
	DirectInput di;
	MouseClient mouse;
	KeyboardClient kbd;
};