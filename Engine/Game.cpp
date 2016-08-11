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
#include "TitleScreen.h"
#include "IntroScreen.h"

Game::Game( HWND hWnd,KeyboardServer& kServer,MouseServer& mServer )
	:
	gfx( hWnd ),
	input( hWnd,mServer,kServer )
{
	screenContainer.pScreen = std::make_unique<IntroScreen>( gfx,input,&screenContainer );
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
	input.di.UpdatePads();

	try
	{
		screenContainer.pScreen->HandleInput();
		screenContainer.pScreen->Update( dt );
	}
	catch( Screen::Change )
	{
		UpdateModel( dt );
	}
}

void Game::ComposeFrame()
{
	screenContainer.pScreen->DrawPreBloom( gfx );
	gfx.ProcessBloom();
	screenContainer.pScreen->DrawPostBloom( gfx );
}