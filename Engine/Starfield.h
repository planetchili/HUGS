#include "Camera.h"
#include "Viewport.h"
#include "D3DGraphics.h"
#include <random>

class Starscape
{
public:
	class Drawable : public ::Drawable
	{
	public:
		Drawable( const Starscape& parent )
			:
			parent( parent )
		{}
		virtual void Rasterize( D3DGraphics& gfx ) const override
		{
			for( const auto& f : parent.fields )
			{
				f.Draw( gfx );
			}
		}
	private:
		const Starscape& parent;
	};
public:
	Starscape( const Camera& cam,Viewport& port )
		:
		cam( cam ),
		port( port ),
		oldCamPos( { FLT_MAX,FLT_MAX } )
	{
		std::random_device rd;
		std::mt19937 rng( rd() );
		fields.emplace_back( 0.77f,rng,180,port );
		fields.emplace_back( 0.71f,rng,195,port );
		fields.emplace_back( 0.65f,rng,215,port );
	}
	void LockToCam()
	{
		oldCamPos = cam.GetPos();
	}
	void Update()
	{
		const Vec2 delta = cam.GetPos() - oldCamPos;
		oldCamPos = cam.GetPos();
		for( auto& f : fields )
		{
			f.Update( port,delta );
		}
	}
	Drawable GetDrawable()
	{
		return Drawable( *this );
	}
private:
	class Starfield
	{
	private:
		class Star
		{
		public:
			Star( Vec2 pos,Color color )
				:
				pos( pos ),
				color( color )
			{}
		public:
			Vec2 pos;
			Color color;
		};
	public:
		Starfield( float depthFactor,std::mt19937& rng,size_t nStars,const Viewport& port )
			:
			depthFactor( depthFactor )
		{
			std::uniform_real_distribution<float> x_dist( 0.0f,port.GetClipWidth() );
			std::uniform_real_distribution<float> y_dist( 0.0f,port.GetClipHeight() );
			std::uniform_int_distribution<int> c_dist( 80u,255u );

			for( size_t i = 0; i < nStars; i++ )
			{
				const unsigned char color = c_dist( rng );
				stars.emplace_back( Vec2 { x_dist( rng ),y_dist( rng ) },
					Color { 255u,color,color,color } );
			}
		}
		void Update( const Viewport& port,Vec2 camDelta )
		{
			const auto ScrollStarComponent = [=]( float comp,float limit )
			{
				if( comp > limit )
				{
					comp -= limit;
				}
				else if( comp < 0.0f )
				{
					comp += limit;
				}
				return comp;
			};

			const Vec2 starDelta = -( camDelta * depthFactor );
			const float right = port.GetClipWidth();
			const float bottom = port.GetClipHeight();
			
			for( auto& s : stars )
			{
				const Vec2 newPos = s.pos + starDelta;
				s.pos.x = ScrollStarComponent( newPos.x,right );
				s.pos.y = ScrollStarComponent( newPos.y,bottom );
			}
		}
		void Draw( D3DGraphics& gfx ) const
		{
			for( const auto& s : stars )
			{
				gfx.PutPixel( int( s.pos.x ),int( s.pos.y ),s.color );
			}
		}
	private:
		std::vector<Star> stars;
		float depthFactor;
	};
private:
	const Camera& cam;
	Viewport& port;
	std::vector<Starfield> fields;
	Vec2 oldCamPos;
};