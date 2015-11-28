/****************************************************************************************** 
 *	Chili DirectX Framework Version 14.03.22											  *	
 *	Game.cpp																			  *
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
#include "Game.h"
#include "Mat3.h"
#include "TriangleStrip.h"
#include <array>

Game::Game( HWND hWnd,KeyboardServer& kServer,MouseServer& mServer )
	:
	gfx( hWnd ),
	input( hWnd,mServer,kServer )
{
}

Game::~Game()
{
}

void Game::Go()
{
#if NDEBUG
	const float dt = timer.GetTimeSec();
	timer.StartWatch();
#else
	const float dt = 1.0f / 60.0f;
#endif

	UpdateModel( dt );
	gfx.BeginFrame();
	ComposeFrame();
	gfx.EndFrame();
}

void Game::UpdateModel( float dt )
{
}

void Game::ComposeFrame()
{
	gfx.DrawRectangle( RectI { 200,215,200,215 },WHITE );
}

//todo:
//fix width/height in d3dgraphics and dependencies
//fix drawrect inclusive/exclusive bullshit
//implement fringe system
//convert all graphics to use viewport
//reduce bloom processor to work only on relevent regions (vertical besides the upscale)
//fix/improve bloom processor upscale code to work edges and look pretty
