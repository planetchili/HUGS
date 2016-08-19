#pragma once
#include "Screen.h"
#include "InputSystem.h"
#include "GameScreen.h"
#include "ChiliMath.h"
#include "CountScreen.h"
#include "Menu.h"
#include <random>
#include <vector>

class TitleScreen2 : public Screen
{
public:
	class Rectangle
	{
	public:
		class Drawable : public ::Drawable
		{
		public:
			Drawable( const Rectangle& parent )
				:
				parent( parent )
			{}
			virtual void Rasterize( D3DGraphics& gfx ) const override
			{
				const float xFactor = clip.GetWidth();
				const float yFactor = clip.GetHeight();

				Color c;
				if( parent.z > parent.fullInZ )
				{
					const float alpha = 1.0f - (parent.z - parent.fullInZ) / parent.fadeInZDist;
					c = { parent.c.x,
						unsigned char( alpha * float( parent.c.r ) ),
						unsigned char( alpha * float( parent.c.g ) ),
						unsigned char( alpha * float( parent.c.b ) ) };
				}
				else
				{
					c = parent.c;
				}

				auto rectPerView = parent.ToScreenCoords( xFactor,yFactor );
				rectPerView.ClipTo( clip );
				gfx.DrawRectangle<SolidRasterizer>( rectPerView,c );
			}
		private:
			const Rectangle& parent;
		};
	public:
		Rectangle( RectF rect,Color c,float startZ,float maxZ )
			:
			rect( rect ),
			c( c ),
			z( startZ ),
			fullInZ( maxZ - fadeInZDist )
		{}
		float Advance( float dz )
		{
			return z -= dz;
		}
		Drawable GetDrawable() const
		{
			return Drawable( *this );
		}
	private:
		RectI ToScreenCoords( float width,float height ) const
		{
			const float zInv = 1.0f / z;
			const float hWidth = width * 0.5f;
			const float hHeight = height * 0.5f;
			return { 
				int( -rect.top * zInv + hHeight ),
				int( -rect.bottom * zInv + hHeight ),
				int( rect.left * zInv + hWidth ),
				int( rect.right * zInv + hWidth ) };
		}
	private:
		static constexpr float fadeInZDist = 12.0f;
		RectF rect;
		float z;
		Color c;
		float fullInZ;
	};
private:
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
			TitleScreen2& screen;
		};
	public:
		StartMenu( TitleScreen2& parent )
			:
			Menu( { 512,560 },400,15,40,{ 210u,10,10,18 },
			L"Arial",BLACK,{ 24u,128,128,255 },{ 16u,52,52,72 },2 ),
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
		TitleScreen2& parent;
	};
public:
	TitleScreen2( D3DGraphics& gfx,InputSystem& input,MidiJukebox& jukebox,ScreenContainer* ctr )
		:
		Screen( ctr ),
		timesFont( L"Times New Roman",100 ),
		arialFont( L"Arial",30 ),
		input( input ),
		gfx( gfx ),
		port( gfx,{ 0.0f,float( gfx.GetHeight() - 1u ),0.0f,float( gfx.GetWidth() - 1u ) },gfx.GetViewRegion()),
		jukebox( jukebox )
	{
		const float dz = (startZ - minZ) / maxRects;
		for( float z = startZ; z > minZ; z -= dz )
		{
			AddRect( z );
		}
	}
	virtual void Update( float dt ) override
	{
		for( auto i = rects.begin(), e = rects.end(); i != e; )
		{
			if( i->Advance( speedZ * dt ) < minZ )
			{
				i = rects.erase( i );
				continue;
			}
			else
			{
				i++;
			}
		}

		if( rects.size() < maxRects )
		{
			AddRect();
		}

		fadeInTimeCounter += dt;
	}
	virtual void DrawPreBloom( D3DGraphics& gfx ) override
	{
		for( const auto& r : rects )
		{
			port.Draw( r.GetDrawable() );
		}
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

		if( fadeInTimeCounter < fadeInTime )
		{
			gfx.Fade( unsigned char( 255.0f * sq( fadeInTimeCounter / fadeInTime ) ) );
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
	void AddRect( float z = startZ )
	{
		const auto width = dist_width( rng );
		const auto height = dist_height( rng );
		const Vec2 pos = { dist_x( rng ),dist_y( rng ) };
		rects.emplace_front(
			RectF{ pos.y + height / 2.0f,
			pos.y - height / 2.0f,
			pos.x - width / 2.0f,
			pos.x + width / 2.0f },Color{ 64u,128u,128u,180u },z,startZ );
	}
	void NewGame()
	{
		jukebox.GetSong( L"blt.mid" ).Stop();
		ChangeScreen( std::make_unique< CountScreen >(
			std::make_unique< GameScreen >( gfx,input,jukebox,nullptr ),parent ) );
	}
private:
	D3DGraphics& gfx;
	InputSystem& input;
	MidiJukebox& jukebox;
	Viewport port;
	Font timesFont;
	Font arialFont;
	std::unique_ptr<StartMenu> pMenu;
	std::list<Rectangle> rects;
	static constexpr int maxRects = 65;
	static constexpr float speedZ = 12.0f;
	static constexpr float startZ = 40.0f;
	static constexpr float minZ = 0.02f;
	static constexpr float meanWidth = 350.0f;
	static constexpr float devWidth = 50.0f;
	static constexpr float meanHeight = 900.0f;
	static constexpr float devHeight = 200.0f;
	static constexpr float devX = 4000.0f;
	static constexpr float devY = 3000.0f;
	static constexpr float fadeInTime = 4.0f;
	float fadeInTimeCounter = 0.0f;
	std::mt19937 rng;
	std::normal_distribution<float> dist_width = std::normal_distribution<float>( meanWidth,devWidth );
	std::normal_distribution<float> dist_height = std::normal_distribution<float>( meanHeight,devHeight );
	std::normal_distribution<float> dist_x = std::normal_distribution<float>( 0.0f,devX );
	std::normal_distribution<float> dist_y = std::normal_distribution<float>( 0.0f,devY );
};
