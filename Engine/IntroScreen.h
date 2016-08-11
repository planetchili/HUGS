#pragma once
#include "Screen.h"
#include "InputSystem.h"
#include "ChiliMath.h"
#include "TitleScreen.h"
#include "Midi.h"

class IntroScreen : public Screen
{
public:
	IntroScreen( D3DGraphics& gfx,InputSystem& input,ScreenContainer* ctr )
		:
		Screen( ctr ),
		input( input ),
		gfx( gfx ),
		uranusPic( Surface::FromFile( L"uranus.png" ) ),
		introMore( L"blt.mid",0.0f,30.0f )
	{
	}
	virtual void Update( float dt ) override
	{
		time += dt;
		switch( stage )
		{
		case Stage::Wait:
			if( time > waitTime )
			{
				stage = Stage::FadeIn;
				time = 0.0f;
				introMore.Play();
			}
			break;
		case Stage::FadeIn:
			if( time > fadeInTime )
			{
				stage = Stage::Hold;
				time = 0.0f;
				alpha = 1.0f;
			}
			else
			{
				alpha = time / fadeInTime;
			}
			break;
		case Stage::Hold:
			if( time > holdTime )
			{
				stage = Stage::FadeOut;
				time = 0.0f;
			}
			break;
		case Stage::FadeOut:
			if( time > fadeOutTime )
			{
				ChangeScreen( std::make_unique<TitleScreen>( gfx,input,parent ) );
			}
			else
			{
				alpha = 1.0f - time / fadeOutTime;
			}
			break;
		}
	}
	virtual void DrawPreBloom( D3DGraphics& gfx ) override
	{
	}
	virtual void DrawPostBloom( D3DGraphics& gfx ) override
	{
		gfx.CopySurfaceViewRegion( uranusPic );
		gfx.Fade( unsigned char( 31.9f * alpha ) * 8u );
	}
	virtual void HandleInput() override
	{
		while( !input.kbd.KeyEmpty() )
		{
			const KeyEvent key = input.kbd.ReadKey();
		}
	}
private:
	enum class Stage
	{
		Wait,
		FadeIn,
		Hold,
		FadeOut
	};
private:
	D3DGraphics& gfx;
	InputSystem& input;
	Surface uranusPic;
	float time = 0.0f;
	Stage stage = Stage::Wait;
	MidiSong introMore;
	float alpha;
	static constexpr float waitTime = 3.0f;
	static constexpr float fadeInTime = 4.0f;
	static constexpr float holdTime = 0.8f;
	static constexpr float fadeOutTime = 1.2f;
};