#pragma once
#include "Screen.h"
#include "InputSystem.h"
#include "GameScreen.h"
#include "ChiliMath.h"
#include "CountScreen.h"
#include "Menu.h"
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
			disRot( 0.0f,stdRotation )
		{}
		void Step()
		{
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
		static void SeedColor( unsigned long seed )
		{
			gen.seed( seed );
		}
		static void SetColor( Color newColor )
		{
			c = newColor;
		}
		static void MutateColor()
		{
			std::uniform_int_distribution<unsigned int> dis6( 0,5 );
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
		static Color c;
		static std::mt19937 gen;
		TitleScreen& parent;
		const float stdRotation = PI / 10.0f;
		const float brushRadius = 8.0f;
		Vec2 vel = { 5.0f,0.0f };
		Vec2 pos = { 0.0f,350.0f };
		std::normal_distribution<float> disRot;
	};
	class StartMenu : public Menu
	{
	private:
		class ExitButton : public Button
		{
		public:
			ExitButton( StartMenu& m )
				:
				Button( m,L"Exit" ),
				m( m )
			{
			}
		private:
			virtual void OnPress() override
			{
				m.OnExit();
			}
		private:
			StartMenu& m;
		};
		class StartButton : public Button
		{
		public:
			StartButton( StartMenu& m )
				:
				Button( m,L"New Game" ),
				screen( m.parent )
			{}
		private:
			virtual void OnPress() override
			{
				screen.NewGame();
			}
		private:
			TitleScreen& screen;
		};
	public:
		StartMenu( TitleScreen& parent )
			:
			Menu( { 512,560 },400,15,40,{ 210,18,18,30 },
			L"Arial",BLACK,{ 80u,128,128,255 },{ 70u,52,52,72 },2 ),
			parent( parent )
		{
			AddItem( std::make_unique<StartButton>( *this ) );
			AddItem( std::make_unique<ExitButton>( *this ) );
			Finalize();
		}
	private:
		virtual void OnExit() override
		{
			parent.pMenu.release();
		}
	private:
		TitleScreen& parent;
	};
public:
	TitleScreen( D3DGraphics& gfx,InputSystem& input,ScreenContainer* ctr )
		:
		Screen( ctr ),
		timesFont( L"Times New Roman",100 ),
		arialFont( L"Arial",30 ),
		input( input ),
		gfx( gfx ),
		funk( gfx.GetWidth(),gfx.GetHeight() )
	{
		std::uniform_int_distribution<unsigned int> cd( 0u,255u );
		Brush::SetColor( Color { 186u,
			unsigned char( cd( rd ) ),
			unsigned char( cd( rd ) ),
			unsigned char( cd( rd ) ) } );
		Brush::SeedColor( rd() );
		brushes.reserve( nBrushes );
		for( int i = 0; i < nBrushes; i++ )
		{
			brushes.emplace_back( *this );
		}
		funk.Clear( BLACK );
	}
	virtual void Update( float dt ) override
	{
		Brush::MutateColor();
		for( Brush& b : brushes )
		{
			b.Step();
		}
	}
	virtual void DrawPreBloom( D3DGraphics& gfx ) override
	{
		gfx.CopySurface( funk );
		if( pMenu )
		{
			pMenu->Draw( gfx );
		}
	}
	virtual void DrawPostBloom( D3DGraphics& gfx ) override
	{
		gfx.DrawString( L"H.U.G.S.",{ 250.0f,300.0f },timesFont,BLACK );
		if( !pMenu )
		{
			gfx.DrawString( L"PRESS ENTER",{ 380.0f,500.0f },arialFont,BLACK );
		}
	}
	virtual void HandleInput() override
	{
		const KeyEvent key = input.kbd.ReadKey();

		if( pMenu )
		{
			pMenu->HandleInput( key );
		}
		else if( key.GetCode() == VK_RETURN && key.IsPress() )
		{
			pMenu = std::make_unique<StartMenu>( *this );
		}
	}
private:
	void NewGame()
	{
		ChangeScreen( std::make_unique< CountScreen >(
			std::make_unique< GameScreen >( gfx,input,nullptr ),parent ) );
	}
private:
	D3DGraphics& gfx;
	InputSystem& input;
	Font timesFont;
	Font arialFont;
	Surface funk;
	std::random_device rd;
	std::vector< Brush > brushes;
	const int nBrushes = 50;
	std::unique_ptr<StartMenu> pMenu;
};