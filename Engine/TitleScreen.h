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
#include "GameScreen.h"
#include "ChiliMath.h"
#include <random>
#include <vector>

class TitleScreen : public Screen
{
private:
	class Brush
	{
	public:
		Brush( TitleScreen& parent )
			:
			parent( parent ),
			gen( parent.rd() ),
			dis6( 0,5 ),
			disRot( 0.0f,stdRotation )
		{}
		void Step()
		{
			MutateColor();

			vel = vel.Rotation( disRot( gen ) );
			pos += vel;
			pos.x = wrapzero( pos.x,float( parent.funk.GetWidth() ) );
			pos.y = wrapzero( pos.y,float( parent.funk.GetHeight() ) );

			const int yStart = int( pos.y - brushRadius );
			const int yEnd = int( pos.y + brushRadius );
			const int xStart = int( pos.x - brushRadius );
			const int xEnd = int( pos.x + brushRadius );
			
			for( int y = yStart; y < yEnd; y++ )
			{
				for( int x = xStart; x < xEnd; x++ )
				{
					parent.funk.PutPixelAlpha( 
						wrapzero( x,int( parent.funk.GetWidth() ) ),
						wrapzero( y,int( parent.funk.GetHeight() ) ),
						c );
				}
			}
		}
	private:
		void MutateColor()
		{
			switch( dis6( gen ) )
			{
			case 0:
				c.r += 1;
				break;
			case 1:
				c.r -= 1;
				break;
			case 2:
				c.g += 1;
				break;
			case 3:
				c.g -= 1;
				break;
			case 4:
				c.b += 1;
				break;
			case 5:
				c.b -= 1;
				break;
			}
		}
	private:
		TitleScreen& parent;
		const float stdRotation = PI / 10.0f;
		const float brushRadius = 8.0f;
		Color c = { 48,0,0,0 };
		Vec2 vel = { 5.0f,0.0f };
		Vec2 pos = { 0.0f,350.0f };
		std::mt19937 gen;
		std::uniform_int_distribution<unsigned int> dis6;
		std::normal_distribution<float> disRot;
	};
public:
	TitleScreen( D3DGraphics& gfx,KeyboardClient& kbd,ScreenContainer& ctr )
		:
		Screen( ctr ),
		port( gfx,{ 0,D3DGraphics::SCREENHEIGHT - 1,0,D3DGraphics::SCREENWIDTH - 1 } ),
		cam( port,port.GetWidth(),port.GetHeight() ),
		timesFont( L"Times New Roman",100 ),
		arialFont( L"Arial",30 ),
		kbd( kbd ),
		gfx( gfx ),
		funk( D3DGraphics::SCREENWIDTH,D3DGraphics::SCREENHEIGHT )
	{
		brushes.reserve( nBrushes );
		for( int i = 0; i < nBrushes; i++ )
		{
			brushes.emplace_back( *this );
		}
	}
	virtual void Update( float dt ) override
	{
		HandleInput();
		for( Brush& b : brushes )
		{
			b.Step();
		}
	}
	virtual void Draw( D3DGraphics& gfx ) override
	{
		gfx.CopySurface( funk );
		gfx.DrawString( L"H.U.G.S.",{ 250.0f,300.0f },timesFont,BLACK );
		gfx.DrawString( L"PRESS ENTER",{ 400.0f,500.0f },arialFont,BLACK );
	}
private:
	void HandleInput()
	{
		const KeyEvent key = kbd.ReadKey();

		if( key.GetCode() == VK_RETURN && key.IsPress() )
		{
			ChangeScreen( std::make_unique< GameScreen >( gfx,kbd,parent ) );
		}
	}
private:
	D3DGraphics& gfx;
	KeyboardClient& kbd;
	Viewport port;
	Camera cam;
	Font timesFont;
	Font arialFont;
	Surface funk;
	std::random_device rd;
	std::vector< Brush > brushes;
	const int nBrushes = 50;
};