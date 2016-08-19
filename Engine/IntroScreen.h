#pragma once
#include "Screen.h"
#include "InputSystem.h"
#include "TitleScreen2.h"
#include "TexturedQuad.h"
#include "Viewport.h"
#include "Midi.h"
#include "MidiJukebox.h"
#include <queue>
#include <memory>


class IntroScreen : public Screen
{
private:
	class Phase
	{
	public:
		Phase( IntroScreen& parent )
			:
			parent( parent )
		{}
		// return -1.0 if not finished, else return unconsumed dt
		virtual float Update( float dt ) = 0;
		virtual void Draw() const = 0;
	protected:
		IntroScreen& parent;
	};
	class PlayIntro : public Phase
	{
		using Phase::Phase;
	public:
		virtual float Update( float dt ) override
		{
			parent.jukebox.GetSong( L"blt.mid" ).Play();
			return dt;
		}
		virtual void Draw() const override
		{}
	};
	class TimedPhase : public Phase
	{
	public:
		TimedPhase( float endTime,IntroScreen& parent )
			:
			Phase( parent ),
			endTime( endTime )
		{}
		virtual float Update( float dt ) override
		{
			time += dt;
			if( time >= endTime )
			{
				return time - endTime;
			}
			return -1.0f;
		}
		float GetAlpha() const
		{
			return time / endTime;
		}
		float GetAlphaComp() const
		{
			return 1.0f - GetAlpha();
		}
	protected:
		float time = 0.0f;
		const float endTime;
	};
	class BlackWait : public TimedPhase
	{
	public:
		BlackWait( float waitTime,IntroScreen& parent )
			:
			TimedPhase( waitTime,parent )
		{}
		virtual void Draw() const override
		{}
	};
	class ImageFadeIn : public TimedPhase
	{
	public:
		ImageFadeIn( const Surface& image,float endTime,IntroScreen& parent )
			:
			TimedPhase( endTime,parent ),
			image( image )
		{}
		virtual void Draw() const override
		{
			parent.gfx.CopySurfaceViewRegion( image );
			parent.gfx.Fade( unsigned char( 31.9f * GetAlpha() ) * 8u );
		}
	protected:
		const Surface& image;
	};
	class ImageHold : public TimedPhase
	{
	public:
		ImageHold( const Surface& image,float endTime,IntroScreen& parent )
			:
			TimedPhase( endTime,parent ),
			image( image )
		{}
		virtual void Draw() const override
		{
			parent.gfx.CopySurfaceViewRegion( image );
		}
	protected:
		const Surface& image;
	};
	class ImageFadeOut : public TimedPhase
	{
	public:
		ImageFadeOut( const Surface& image,float endTime,IntroScreen& parent )
			:
			TimedPhase( endTime,parent ),
			image( image )
		{}
		virtual void Draw() const override
		{
			parent.gfx.CopySurfaceViewRegion( image );
			parent.gfx.Fade( unsigned char( 31.9f * GetAlphaComp() ) * 8u );
		}
	protected:
		const Surface& image;
	};
	class HitlerFadeIn : public TimedPhase
	{
	public:
		HitlerFadeIn( float endTime,IntroScreen& parent )
			:
			TimedPhase( endTime,parent )
		{}
		virtual void Draw() const override
		{
			parent.gfx.CopySurfaceViewRegion( parent.moon );
			auto ud = parent.uranus2.GetDrawable();
			ud.Transform( Mat3::Translation( { 150.0f,150.0f } ) );
			parent.port.Draw( ud );
			auto hd = parent.hitler.GetDrawable();
			hd.Transform( Mat3::Translation( parent.hitlerBasePos ) );
			parent.port.Draw( hd );
			parent.gfx.Fade( unsigned char( 31.9f * GetAlpha() ) * 8u );
		}
	};
	class HitlerAwesome : public TimedPhase
	{
	public:
		HitlerAwesome( float endTime,IntroScreen& parent )
			:
			TimedPhase( endTime,parent )
		{}
		virtual void Draw() const override
		{
			parent.gfx.CopySurfaceViewRegion( parent.moon );
			auto ud = parent.uranus2.GetDrawable();
			ud.Transform( Mat3::Translation( { 150.0f,150.0f } ) );
			parent.port.Draw( ud );
			auto sd = parent.shades.GetDrawable();
			sd.Transform( Mat3::Translation( { 110.0f,125.0f } ) );
			parent.port.Draw( sd );
			auto hd = parent.hitler.GetDrawable();
			hd.Transform( Mat3::Translation( parent.hitlerBasePos ) );
			parent.port.Draw( hd );
			auto ad = parent.awesome.GetDrawable();
			ad.Transform( Mat3::Translation( parent.hitlerBasePos + parent.facePos ) );
			parent.port.Draw( ad );
		}
	};
	class HitlerSpaz : public TimedPhase
	{
	public:
		HitlerSpaz( float endTime,IntroScreen& parent )
			:
			TimedPhase( endTime,parent )
		{}
		virtual void Draw() const override
		{
			parent.gfx.CopySurfaceViewRegion( parent.moon );
			auto ud = parent.uranus2.GetDrawable();
			ud.Transform( Mat3::Translation( { 150.0f,150.0f } ) );
			parent.port.Draw( ud );
			auto sd = parent.shades.GetDrawable();
			sd.Transform( Mat3::Translation( { 110.0f,125.0f } ) );
			parent.port.Draw( sd );
			const float theta = parent.rotSpeed * time;
			const Vec2 move = parent.moveVel * time;
			const float scale = 1.0f + parent.scaleBaseGrowth * time;
			const auto hit_trans = Mat3::Translation( parent.hitlerBasePos + move ) * Mat3::Rotation( theta )
				* Mat3::Scaling( scale );
			auto hd = parent.hitler.GetDrawable();
			hd.Transform( hit_trans );
			parent.port.Draw( hd );
			auto ad = parent.awesome.GetDrawable();
			ad.Transform( hit_trans * Mat3::Translation( parent.facePos ) );
			parent.port.Draw( ad );
			parent.gfx.Fade( unsigned char( 255.0f * GetAlphaComp() ) );
		}
	};
public:
	IntroScreen( D3DGraphics& gfx,InputSystem& input,MidiJukebox& jukebox,ScreenContainer* ctr )
		:
		Screen( ctr ),
		input( input ),
		gfx( gfx ),
		uranus( Surface::FromFile( L"uranus.png" ) ),
		hitler( L"hitler.png" ),
		awesome( L"hitawe.png" ),
		logo( Surface::FromFile( L"ChiliLogoScreen.png" ) ),
		port( gfx,{ 0.0f,float( gfx.GetHeight() - 1u ),0.0f,float( gfx.GetWidth() - 1u ) },
			gfx.GetViewRegion() ),
		hitlerBasePos( { 
			float( gfx.GetViewRegion().GetWidth() - hitler.GetTexture().GetWidth() / 2 ),
			float( gfx.GetViewRegion().GetHeight() - hitler.GetTexture().GetHeight() / 2 ) } ),
		moon( Surface::FromFile( L"moonsurf.png" ) ),
		uranus2( L"uranus2.png" ),
		shades( L"shades.png" ),
		jukebox( jukebox )
	{
		phases.emplace( new BlackWait( 1.0f,*this ) );
		phases.emplace( new ImageFadeIn( logo,0.6f,*this ) );
		phases.emplace( new ImageHold( logo,1.5f,*this ) );
		phases.emplace( new ImageFadeOut( logo,0.6f,*this ) );
		phases.emplace( new BlackWait( 0.8f,*this ) );
		phases.emplace( new PlayIntro( *this ) );
		phases.emplace( new ImageFadeIn( uranus,3.7f,*this ) );
		phases.emplace( new ImageHold( uranus,1.5f,*this ) );
		phases.emplace( new ImageFadeOut( uranus,0.6f,*this ) );
		phases.emplace( new BlackWait( 0.7f,*this ) );
		phases.emplace( new HitlerFadeIn( 3.05f,*this ) );
		phases.emplace( new HitlerAwesome( 2.0f,*this ) );
		phases.emplace( new HitlerSpaz( 1.5f,*this ) );
		phases.emplace( new BlackWait( 1.0f,*this ) );
	}
	virtual void Update( float dt ) override
	{
		while( dt >= 0.0f )
		{
			if( phases.size() == 0u )
			{
				ChangeScreen( std::make_unique<TitleScreen2>( gfx,input,jukebox,parent ) );
			}
			else if( (dt = phases.front()->Update( dt )) >= 0.0f )
			{
				phases.pop();
			}
		}
	}
	virtual void DrawPreBloom( D3DGraphics& gfx ) override
	{
	}
	virtual void DrawPostBloom( D3DGraphics& gfx ) override
	{
		phases.front()->Draw();
	}
	virtual void HandleInput() override
	{
		while( !input.kbd.KeyEmpty() )
		{
			const KeyEvent key = input.kbd.ReadKey();
		}
	}
private:
	D3DGraphics& gfx;
	InputSystem& input;
	MidiJukebox& jukebox;
	std::queue<std::unique_ptr<Phase>> phases;
	Viewport port;
	Surface uranus;
	Surface logo;
	Surface moon;
	TexturedQuad<TransparentRasterizer> shades;
	TexturedQuad<TranslucentRasterizer> uranus2;
	TexturedQuad<TranslucentRasterizer> hitler;
	TexturedQuad<TranslucentRasterizer> awesome;
	const Vec2 hitlerBasePos;
	const Vec2 facePos = {15.0f,-70.0f};
	const float rotSpeed = 15.0f;
	const float zoomOsc = 0.0f;
	const float zoomAmpFactor = 0.0f;
	const float scaleBaseGrowth = 3.0f;
	const Vec2 moveVel = { -250.0f,-180.0f };
};