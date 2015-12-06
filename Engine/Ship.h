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
		shipQuad( filename,0.27f,{ 0.0f,6.0f } ),
		timer( seq ),
		collisionSound( { L"clsn1.wav",L"clsn2.wav",L"clsn3.wav" },0.037f,std::random_device()( ) )
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
		thrusting = true;
	}
	void StopThrusting()
	{
		thrusting = false;
	}
	void Spin( Vec2 n )
	{
		dirNormal = n;
	}
	virtual void Rebound( Vec2 normal ) override
	{
		const float velVolumeMax = 800.0f;
		const float volMin = 0.1f;

		const float velIncident = ( -vel * normal );
		const float volume = min( 1.0f,
			( 1.0f - volMin ) * ( velIncident / velVolumeMax ) + volMin );

		collisionSound.Play( volume );

		if( shieldLevel > 0.0f )
		{
			shieldLevel = max( 0.0f,shieldLevel - velIncident * kDamage );
		}
		else
		{
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

	// rules stuff
	TrackSequencer seq;
	LapTimer timer;

	// stats
	float shieldLevel = 1.0f;
	const float kDamage = 0.0003f;

	// structural
	TexturedQuad shipQuad;
	const Color shieldColor = { GREEN,160u };

	// linear
	float thrustForce = 1200.0f;
	bool thrusting = false;

	// angular
	Vec2 dirNormal = { 1.0f,0.0f };
};