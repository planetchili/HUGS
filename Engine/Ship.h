#pragma once
#include "PolyClosed.h"
#include "Camera.h"
#include "PhysicalCircle.h"
#include <array>
#include "Vertex.h"
#include "Observer.h"
#include "TrackRegionManager.h"
#include "TexturedQuad.h"

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
			Transform( Mat3::Translation( parent.pos ) * Mat3::Rotation( parent.angle ) );
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
public:
	Ship( const std::wstring& filename,const TrackRegionManager& tMan,Vec2 pos = { 0.0f,0.0f } )
		:
		PhysicalCircle( 50.0f,1.0f,0.001f,pos ),
		seq( tMan ),
		shipQuad( filename,0.27f,{ 0.0f,6.0f } )
	{}
	Drawable GetDrawable() const
	{
		return Drawable( *this );
	}
	virtual void Update( float dt ) override
	{
		// angular (1st order then 0th order)
		// deccel faster than accel
   		if( angAccelDir == 0.0f )
		{
			if( abs( angVel ) <= angAccel * dt )
			{
				angVel = 0.0f;
			}
			else
			{
				angVel -= copysign( angAccel,angVel ) * dt;
			}
		}
		angVel += angAccel * angAccelDir * dt;
		if( abs( angVel ) > maxAngVel )
		{
			angVel = copysign( maxAngVel,angVel );
		}
		angle += angVel * dt;
		// clamp angle to within 2pi
		angle = fmodf( angle,2.0f * PI );

		// thrust force
		if( thrusting )
		{
			ApplyForce( Vec2 { 0.0f,-1.0f }.Rotation( angle ) * thrustForce );
		}

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
	// control functions
	void Thrust()
	{
		thrusting = true;
	}
	void StopThrusting()
	{
		thrusting = false;
	}
	void Spin( float dir )
	{
		angAccelDir = copysign( 1.0f,dir );
	}
	void StopSpinning( float dir )
	{
		if( angAccelDir == copysign( 1.0f,dir ) )
		{
			angAccelDir = 0.0f;
		}
	}
	virtual void Rebound( Vec2 normal ) override
	{
		if( shieldLevel > 0.0f )
		{
			shieldLevel = max( 0.0f,
				shieldLevel - ( -vel * normal ) * kDamage );
		}
		else
		{
			Notify();
		}
		PhysicalCircle::Rebound( normal );
	}
	void Track( unsigned int uid )
	{
		seq.HitRegion( uid );
	}

private:
	// rules stuff
	TrackSequencer seq;

	// stats
	float shieldLevel = 1.0f;
	const float kDamage = 0.0003f;

	// structural
	TexturedQuad shipQuad;
	const Color shieldColor = GREEN;

	// linear
	float thrustForce = 1200.0f;
	bool thrusting = false;

	// angular
	float angle = 0.0f;
	float angVel = 0.0f;
	const float maxAngVel = 2.5f * PI;
	const float angAccel = 0.004f * 60.0f * 60.0f;
	float angAccelDir = 0.0f;
};