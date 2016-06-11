#pragma once
#include "PolyClosed.h"
#include "Camera.h"
#include "PhysicalCircle.h"
#include <array>
#include <numeric>
#include "Vertex.h"
#include "Observer.h"
#include "TrackRegionManager.h"
#include "TexturedQuad.h"
#include "SoundEffect.h"

class Ship : public PhysicalCircle, public Observable
{
public:
	class Drawable : public ::Drawable
	{
	public:
		Drawable( const Ship& parent )
			:
			parent( parent )
		{
			Transform( Mat3::Translation( parent.pos ) * 
				Mat3::Rotation( atan2( parent.dirNormal.y,parent.dirNormal.x ) + PI / 2.0f ) );
		}
		virtual void Rasterize( D3DGraphics& gfx ) const override
		{
			if( parent.thrusting )
			{
				const auto thrustTrans = trans * Mat3::Translation( parent.thrustOffset ) *
					Mat3::Stretching( parent.thrustScale * ( 1.0f + parent.thrustIntensityMod * 0.33f ),
					parent.thrustScale * ( 1.0f + parent.thrustIntensityMod ) );
				auto tc = parent.thrustColor;
				tc.x = unsigned char( float( tc.x ) * ( 1.0f + parent.thrustIntensityMod * 0.1f ) );
				auto t2c = parent.thrust2Color;
				t2c.x = unsigned char( float( t2c.x ) * ( 1.0f + parent.thrustIntensityMod * 0.04f ) );
				for( auto& s : parent.thrustStrips )
				{
					auto d = s.GetDrawable( tc );
					d.Transform( thrustTrans );
					d.Clip( clip );
					d.Rasterize( gfx );
				}
				for( auto& s : parent.thrust2Strips )
				{
					auto d = s.GetDrawable( t2c );
					d.Transform( thrustTrans );
					d.Clip( clip );
					d.Rasterize( gfx );
				}
			}

			auto shipQuad = parent.shipQuad.GetDrawable();
			shipQuad.Transform( trans );
			shipQuad.Clip( clip );
			shipQuad.Rasterize( gfx );

			const Vec2 shieldCenter = trans * Vec2 { 0.0f,0.0f };
			gfx.DrawCircle( shieldCenter,(int)parent.radius,parent.shieldColor );
		}
	private:
		const Ship& parent;
	};
private:
	class TrackSequencer : public Observable
	{
	public:
		TrackSequencer( const TrackRegionManager& tMan )
			:
			startRegion(tMan.GetStart()),
			endRegion(tMan.GetEnd()),
			curRegion(tMan.GetStart())
		{}
		void HitRegion( unsigned int uid )
		{
			if( uid == curRegion->GetID() );
			else if( uid == GetNextRegion()->GetID() )
			{
				if( uid == startRegion->GetID() && !isWrong )
				{
					Notify();
				}
				isWrong = false;
				curRegion = GetNextRegion();
			}
			else if( !isWrong )
			{
				isWrong = true;
				curRegion = GetPrevRegion();
			}
		}
	private:
		std::set< TrackRegion >::const_iterator GetNextRegion() const
		{
			if( std::next( curRegion ) == endRegion )
			{
				return startRegion;
			}
			else
			{
				return std::next( curRegion );
			}
		}
		std::set< TrackRegion >::const_iterator GetPrevRegion() const
		{
			if( curRegion == startRegion )
			{
				return std::prev( endRegion );
			}
			else
			{
				return std::prev( curRegion );
			}
		}
	private:
		bool isWrong = false;
		std::set< TrackRegion >::const_iterator curRegion;
		const std::set< TrackRegion >::const_iterator startRegion;
		const std::set< TrackRegion >::const_iterator endRegion;
	};
private:
	class LapTimer : public Observer
	{
	public:
		LapTimer( TrackSequencer& seq )
		{
			seq.AddObserver( *this );
		}
		float GetCurrentLapTime() const
		{
			return currentLapTime;
		}
		float GetTotalTime() const
		{
			return std::accumulate( lapTimes.begin(),lapTimes.end(),currentLapTime );
		}
		unsigned int GetLapCount() const
		{
			return unsigned int( lapTimes.size() );
		}
		virtual void OnNotify() override
		{
			lapTimes.push_back( currentLapTime );
			currentLapTime = 0.0f;
		}
		void Update( float dt )
		{
			currentLapTime += dt;
		}
		std::vector< float >::const_iterator begin() const
		{
			return lapTimes.begin();
		}
		std::vector< float >::const_iterator end() const
		{
			return lapTimes.end();
		}
	private:
		std::vector< float > lapTimes;
		float currentLapTime = 0.0f;
	};
public:
	Ship( const std::wstring& filename,const TrackRegionManager& tMan,Vec2 pos = { 0.0f,0.0f } )
		:
		PhysicalCircle( 50.0f,1.0f,0.001f,pos ),
		seq( tMan ),
		shipQuad( filename,1.33f,{ 0.0f,-3.0f } ),
		timer( seq ),
		collisionSound( { L"clsn1.wav",L"clsn2.wav",L"clsn3.wav" },0.037f,std::random_device()( ) ),
		thrusterSound( L"thrust_loop.wav",true ),
		thrustStrips( PolyClosed( "thrust.dxf" ).ExtractSolidStrips() ),
		thrust2Strips( PolyClosed( "thrust2.dxf" ).ExtractSolidStrips() ),
		thrustRng( std::random_device()() ),
		thrustIntensityDist( 0.0f,0.2f )
	{}
	Drawable GetDrawable() const
	{
		return Drawable( *this );
	}
	virtual void Update( float dt ) override
	{
		// thrust force
		if( thrusting )
		{
			ApplyForce( dirNormal * thrustForce );
		}

		timer.Update( dt );
		PhysicalCircle::Update( dt );

		thrustIntensityMod = thrustIntensityDist( thrustRng );
	}
	void FocusOn( Camera& cam ) const
	{
		cam.MoveTo( pos );
	}
	float GetShieldPercent() const
	{
		return shieldLevel;
	}
	void RegisterLapObserver( Observer& lapObs )
	{
		seq.AddObserver( lapObs );
	}
	void Thrust()
	{
		if( !thrusting )
		{
			thrusting = true;
			thrusterSound.Play( 1.0f,1.2f );
		}
	}
	void StopThrusting()
	{
		if( thrusting )
		{
			thrusting = false;
			thrusterSound.StopOne();
		}
	}
	void Spin( Vec2 n )
	{
		dirNormal = n;
	}
	virtual void Rebound( Vec2 normal ) override
	{
		const float velVolumeMax = 800.0f;
		const float volMin = 0.03f;

		const float velIncident = ( -vel * normal );
		const float volume = min( 1.0f,
			( 1.0f - volMin ) * ( velIncident / velVolumeMax ) + volMin ) * 1.7f;

		collisionSound.Play( volume );

		if( shieldLevel > 0.0f )
		{
			shieldLevel = max( 0.0f,shieldLevel - velIncident * kDamage );
		}
		else
		{
			StopThrusting();
			Notify();
		}
		PhysicalCircle::Rebound( normal );
	}
	const LapTimer& Timer() const
	{
		return timer;
	}
	void Track( unsigned int uid )
	{
		seq.HitRegion( uid );
	}

private:
	// sfx
	SoundEffect collisionSound;
	Sound thrusterSound;

	// rules stuff
	TrackSequencer seq;
	LapTimer timer;

	// stats
	float shieldLevel = 1.0f;
	const float kDamage = 0.0003f;

	// structural
	TexturedQuad shipQuad;
	const Color shieldColor = { GREEN,144u };

	// thruster
	std::vector<TriangleStrip> thrustStrips;
	std::vector<TriangleStrip> thrust2Strips;
	Vec2 thrustOffset = { 0.0f,35.0f };
	float thrustScale = 1.63f;
	float thrustIntensityMod;
	Color thrustColor = { 120u,50u,65u,160u };
	Color thrust2Color = { 255u,92u,128u,196u };
	std::mt19937 thrustRng;
	std::normal_distribution<float> thrustIntensityDist;

	// linear
	float thrustForce = 1200.0f;
	bool thrusting = false;

	// angular
	Vec2 dirNormal = { 1.0f,0.0f };
};