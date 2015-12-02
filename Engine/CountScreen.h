#pragma once
#include "Screen.h"
#include "Keyboard.h"
#include "GameScreen.h"
#include "ChiliMath.h"
#include <random>
#include <vector>
#include <memory>

class CountScreen : public Screen, public ScreenContainer
{
public:
	CountScreen( std::unique_ptr< Screen > child,ScreenContainer* ctr )
		:
		Screen( ctr ),
		timesFont( L"Times New Roman",150 ),
		child( std::move( child ) )
	{
		SetOtherParent( *this->child,this );
	}
	virtual void Update( float dt )
	{
		time = max( time - dt,0.0f );
		if( time <  1.0f )
		{
			SetOtherParent( *child,parent );
			ChangeScreen( std::move( child ) );
		}
	}
	virtual void DrawPreBloom( D3DGraphics& gfx ) override
	{
		child->DrawPreBloom( gfx );
	}
	virtual void DrawPostBloom( D3DGraphics& gfx ) override
	{
		child->DrawPostBloom( gfx );
		gfx.DrawString( std::to_wstring( int( time ) ),{ 450.0f,350.0f },timesFont,RED );
	}
	virtual void HandleInput() override
	{
	}
private:
	float time = 3.999f;
	Font timesFont;
	std::unique_ptr< Screen > child;
};