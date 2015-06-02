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
	: gfx( hWnd ),
	audio( hWnd ),
	kbd( kServer ),
	mouse( mServer ),
	map( "tracktest.dxf" ),
	ship( L"USS Turgidity.png",map.GetTrackRegionManager(),map.GetStartPosition() ),
	port( gfx,{ 0,D3DGraphics::SCREENHEIGHT - 1,0,D3DGraphics::SCREENWIDTH - 1 } ),
	cam( port,port.GetWidth(),port.GetHeight() ),
	meter( { 20,45,20,D3DGraphics::SCREENWIDTH / 4 },ship ),
	timesFont( L"Times New Roman",60 )
{
	ship.AddObserver( deathListener );
	ship.RegisterLapObserver( lapListener );
}

Game::~Game()
{
}

void Game::Go()
{
	HandleInput();
	UpdateModel();

	gfx.BeginFrame();
	ComposeFrame();
	gfx.EndFrame();
}

void Game::HandleInput( )
{
	const KeyEvent key = kbd.ReadKey( );
	switch( key.GetCode( ) )
	{
	case VK_LEFT:
		if( key.IsPress( ) )
		{
			ship.Spin( -1.0f );
		}
		else
		{
			ship.StopSpinning( -1.0f );
		}
		break;
	case VK_RIGHT:
		if( key.IsPress( ) )
		{
			ship.Spin( 1.0f );
		}
		else
		{
			ship.StopSpinning( 1.0f );
		}
		break;
	case VK_SPACE:
		if( key.IsPress( ) )
		{
			ship.Thrust( );
		}
		else
		{
			ship.StopThrusting( );
		}
		break;
	}
}

void Game::UpdateModel( )
{
#if NDEBUG
	const float dt = timer.GetTimeSec();
	timer.StartWatch();
#else
	const float dt = 1.0f / 60.0f;
#endif

	if( !deathListener.IsDead() )
	{
		ship.Update( dt );
		map.TestCollision( ship );
	}
	map.Update( dt );
}

void Game::ComposeFrame()
{
	if( !deathListener.IsDead() )
	{
		ship.FocusOn( cam );
		cam.Draw( ship.GetDrawable() );
	}

	cam.Draw( map.GetDrawable() );
	port.Draw( meter.GetDrawable() );

	if( deathListener.IsDead() )
	{
		gfx.DrawString( L"GAME\nOVER",{ 400.0f,300.0f },timesFont,GRAY );
	}

	gfx.DrawString( std::to_wstring( lapListener.GetLapCount() ),{ 920.0f,0.0f },timesFont,GRAY );
}
