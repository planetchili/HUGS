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
#include "Keyboard.h"
#include "CountScreen.h"
#include "ShipControllerKeyboard.h"

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
	GameScreen( D3DGraphics& gfx,KeyboardClient& kbd,ScreenContainer* ctr )
		:
		Screen( ctr ),
		map( "tracktest.dxf" ),
		ship( L"USS Turgidity.png",map.GetTrackRegionManager(),map.GetStartPosition() ),
		port( gfx,{ 0,D3DGraphics::SCREENHEIGHT - 1,0,D3DGraphics::SCREENWIDTH - 1 } ),
		cam( port,port.GetWidth(),port.GetHeight() ),
		meter( { 20,45,20,D3DGraphics::SCREENWIDTH / 4 },ship ),
		timesFont( L"Times New Roman",60 ),
		arialFont( L"Arial",20 ),
		lapDisplay( ship,{ 860.0f,15.0f } ),
		kbd( kbd ),
		gfx( gfx ),
		kbdCtrl( ship,kbd )
	{
		ship.AddObserver( deathListener );
	}
	virtual void Update( float dt ) override
	{
		if( !deathListener.IsDead() )
		{
			ship.Update( dt );
			map.TestCollision( ship );
		}
		map.Update( dt );
	}
	virtual void Draw( D3DGraphics& gfx ) override
	{
		if( !deathListener.IsDead() )
		{
			ship.FocusOn( cam );
			cam.Draw( ship.GetDrawable() );
		}

		cam.Draw( map.GetDrawable() );
		port.Draw( meter.GetDrawable() );
		port.Draw( lapDisplay.GetDrawable() );

		if( deathListener.IsDead() )
		{
			gfx.DrawString( L"GAME\nOVER",{ 400.0f,300.0f },timesFont,GRAY );
			gfx.DrawString( L"Press ENTER to RESTART",{375.0f,550.0f },arialFont,RED );
		}
	}
	virtual void HandleInput() override
	{
		if( deathListener.IsDead() )
		{
			while( !kbd.KeyEmpty() )
			{
				const KeyEvent key = kbd.ReadKey();
				if( key.GetCode() == VK_RETURN && key.IsPress() )
				{
					ChangeScreen( std::make_unique< CountScreen >(
						std::make_unique< GameScreen >( gfx,kbd,nullptr ),parent ) );
				}
			}
		}
		kbdCtrl.Process();
	}
private:
	D3DGraphics& gfx;
	KeyboardClient& kbd;
	Viewport port;
	Camera cam;
	Map map;
	Ship ship;
	ShieldMeter meter;
	Font timesFont;
	Font arialFont;
	LapDisplay lapDisplay;
	ShipControllerKeyboard kbdCtrl;
};