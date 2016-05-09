#pragma once
#include "Screen.h"
#include "Camera.h"
#include "Viewport.h"
#include "Ship.h"
#include "Map.h"
#include "ShieldMeter.h"
#include "Observer.h"
#include "BlackHole.h"
#include "LapDisplay.h"
#include "Timer.h"
#include "CountScreen.h"
#include "ShipControllerKeyboard.h"
#include "ShipControllerGamepad.h"
#include "InputSystem.h"
#include "Starfield.h"

class GameScreen : public Screen
{
private:
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
	GameScreen( D3DGraphics& gfx,InputSystem& input,ScreenContainer* ctr )
		:
		Screen( ctr ),
		map( "tracktest.dxf" ),
		ship( L"spaceship.png",map.GetTrackRegionManager(),map.GetStartPosition() ),
		port( gfx,{ 0.0f,float( gfx.GetHeight() - 1u ),0.0f,float( gfx.GetWidth() - 1u ) },
			gfx.GetViewRegion() ),
		cam( port,port.GetWidth(),port.GetHeight() ),
		starscape( cam,port ),
		meter( { 20,45,20,int( port.GetWidth() / 4.0f ) },ship ),
		timesFont( L"Times New Roman",60 ),
		arialFont( L"Arial",20 ),
		lapDisplay( ship,{ 860.0f,15.0f } ),
		input( input ),
		gfx( gfx )
	{
		controllers.push_back( std::make_unique<ShipControllerKeyboard>( ship,input.kbd ) );
		if( input.di.PadExists() )
		{
			controllers.push_back( std::make_unique<ShipControllerGamepad>( ship,input.di.GetPad() ) );
		}
		ship.AddObserver( deathListener );
		ship.FocusOn( cam );
		starscape.LockToCam();
	}
	virtual void Update( float dt ) override
	{
		if( !deathListener.IsDead() )
		{
			ship.Update( dt );
			map.TestCollision( ship );
		}
		map.Update( dt );
		starscape.Update();
	}
	virtual void DrawPreBloom( D3DGraphics& gfx ) override
	{
		port.Draw( starscape.GetDrawable() );

		if( !deathListener.IsDead() )
		{
			ship.FocusOn( cam );
			cam.Draw( ship.GetDrawable() );
		}

		cam.Draw( map.GetDrawable() );
	}
	virtual void DrawPostBloom( D3DGraphics& gfx ) override
	{
		port.Draw( meter.GetDrawable() );
		port.Draw( lapDisplay.GetDrawable() );

		if( deathListener.IsDead() )
		{
			gfx.DrawString( L"GAME\nOVER",{ 400.0f,300.0f },timesFont,GRAY );
			gfx.DrawString( L"Press ENTER to RESTART",{ 375.0f,550.0f },arialFont,RED );
		}
	}
	virtual void HandleInput() override
	{
		if( deathListener.IsDead() )
		{
			while( !input.kbd.KeyEmpty() )
			{
				const KeyEvent key = input.kbd.ReadKey();
				if( key.GetCode() == VK_RETURN && key.IsPress() )
				{
					ChangeScreen( std::make_unique< CountScreen >(
						std::make_unique< GameScreen >( gfx,input,nullptr ),parent ) );
				}
			}
		}
		else
		{
			for( const auto& c : controllers )
			{
				c->Process();
			}
		}
	}
private:
	D3DGraphics& gfx;
	InputSystem& input;
	Viewport port;
	Camera cam;
	Map map;
	Ship ship;
	ShieldMeter meter;
	Starscape starscape;
	Font timesFont;
	Font arialFont;
	LapDisplay lapDisplay;
	std::vector<std::unique_ptr<ShipController>> controllers;
};