/****************************************************************************************** 
 *	Chili DirectX Framework Version 14.03.22											  *	
 *	Game.h																				  *
 *	Copyright 2014 PlanetChili.net <http://www.planetchili.net>							  *
 *																						  *
 *	This file is part of The Chili DirectX Framework.									  *
 *																						  *
 *	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
 *	it under the terms of the GNU General Public License as published by				  *
 *	the Free Software Foundation, either version 3 of the License, or					  *
 *	(at your option) any later version.													  *
 *																						  *
 *	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
 *	GNU General Public License for more details.										  *
 *																						  *
 *	You should have received a copy of the GNU General Public License					  *
 *	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
 ******************************************************************************************/
#pragma once

#include "D3DGraphics.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Sound.h"
#include "Timer.h"
#include "FrameTimer.h"
#include "PolyClosed.h"
#include "Camera.h"
#include "Viewport.h"
#include "Ship.h"
#include "Map.h"
#include "ShieldMeter.h"
#include "Observer.h"
#include "BlackHole.h"

class Game
{
private:
	class LapListener : public Observer
	{
	public:
		virtual void OnNotify() override
		{
			count++;
		}
		int GetLapCount() const
		{
			return count;
		}
	private:
		int count = 0;
	} lapListener;
	class DeathListener : public Observer
	{
	public:
		virtual void OnNotify() override
		{
			isDead = true;
		}
		bool IsDead() const
		{
			return isDead;
		}
	private:
		bool isDead = false;
	} deathListener;
public:
	Game( HWND hWnd,KeyboardServer& kServer,MouseServer& mServer );
	~Game();
	void Go();
private:
	void ComposeFrame();
	/********************************/
	/*  User Functions              */

	void UpdateModel( );
	void HandleInput();

	/********************************/
private:
	D3DGraphics gfx;
	KeyboardClient kbd;
	MouseClient mouse;
	DSound audio;
	/********************************/
	/*  User Variables              */

	Timer timer;
	Viewport port;
	Camera cam;
	Map map;
	Ship ship;
	ShieldMeter meter;
	Font timesFont;
	/********************************/
};