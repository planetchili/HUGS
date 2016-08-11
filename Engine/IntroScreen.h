#pragma once
#include "Screen.h"
#include "InputSystem.h"
#include "TitleScreen.h"
#include "TexturedQuad.h"
#include "Viewport.h"
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
		introMore( L"blt.mid",0.0f,30.0f ),
		hitler( L"hitler_blk.png" ),
		awesome( L"hitawe.png" ),
		port( gfx,{ 0.0f,float( gfx.GetHeight() - 1u ),0.0f,float( gfx.GetWidth() - 1u ) },
			gfx.GetViewRegion() )
	{
	}
	virtual void Update( float dt ) override
	{
		time += dt;
		switch( ss )
		{
		case SubStage::Wait:
			if( time > waitTime )
			{
				ss = SubStage::FadeIn;
				time = 0.0f;
				introMore.Play();
			}
			break;
		case SubStage::FadeIn:
			if( time > fadeInTime )
			{
				ss = SubStage::Hold;
				time = 0.0f;
				alpha = 1.0f;
			}
			else
			{
				alpha = time / fadeInTime;
			}
			break;
		case SubStage::Hold:
			if( time > holdTime )
			{
				ss = SubStage::FadeOut;
				time = 0.0f;
			}
			break;
		case SubStage::FadeOut:
			if( time > fadeOutTime )
			{
				if( ms == MasterStage::Hitler )
				{
					ChangeScreen( std::make_unique<TitleScreen>( gfx,input,parent ) );
				}
				else
				{
					ms = MasterStage::Hitler;
					ss = SubStage::Wait;
					time = 0.0f;
					alpha = 0.0f;
				}
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
		switch( ms )
		{
		case MasterStage::Uranus:
			gfx.CopySurfaceViewRegion( uranusPic );
			break;
		case MasterStage::Hitler:
		{
			auto hd = hitler.GetDrawable();
			hd.Transform( Mat3::Translation( { 900.0f,600.0f } ) );
			port.Draw( hd );
		}
			break;
		}
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
	enum class SubStage
	{
		Wait,
		FadeIn,
		Hold,
		FadeOut
	};
	enum class MasterStage
	{
		Uranus,
		Hitler
	};
private:
	D3DGraphics& gfx;
	InputSystem& input;
	Viewport port;
	Surface uranusPic;
	TexturedQuad hitler;
	TexturedQuad awesome;
	float time = 0.0f;
	SubStage ss = SubStage::Wait;
	MasterStage ms = MasterStage::Uranus;
	MidiSong introMore;
	float alpha;
	static constexpr float waitTime = 1.0f;
	static constexpr float fadeInTime = 3.8f;
	static constexpr float holdTime = 1.5f;
	static constexpr float fadeOutTime = 1.0f;
};