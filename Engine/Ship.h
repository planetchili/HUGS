#pragma once
#include "PolyClosed.h"
#include "Camera.h"
#include "CollidableCircle.h"
#include <array>
#include "Vertex.h"
#include "Observer.h"

class Ship : public CollidableCircle, public Observable
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
			std::array<Vertex, 4 > quadTrans;
			const Mat3 shipTrans = trans * Mat3::Translation( -parent.shipCenter )
				* Mat3::Scaling( parent.shipScale );
			for (int i = 0; i < 4; i++)
			{
				quadTrans[i].t = parent.quad[i].t;
				quadTrans[i].v = shipTrans * parent.quad[i].v;
			}

			gfx.DrawTriangleTex(quadTrans[0], quadTrans[1], quadTrans[3], clip, 
				parent.shipTexture);
			gfx.DrawTriangleTex(quadTrans[1], quadTrans[2], quadTrans[3], clip,
				parent.shipTexture);

			const Vec2 shieldCenter = trans * Vec2 { 0.0f,0.0f };
			gfx.DrawCircle( shieldCenter,parent.shieldRadius,parent.shieldColor );
		}
	private:
		const Ship& parent;
	};
public:
	Ship( const std::wstring& filename,Vec2 pos = { 0.0f,0.0f } )
		:
		pos( pos ),
		shipTexture( Surface::FromFile( filename ) )
	{
		quad[0].v = { -80,-135.0f };
		quad[0].t = { 0.0f,0.0f };
		quad[1].v = { 79,-135.0f };
		quad[1].t = { 159.0f,0.0f };
		quad[2].v = { 79,134.0f };
		quad[2].t = { 159.0f,269.0f };
		quad[3].v = { -80,134.0f };
		quad[3].t = { 0.0f,269.0f };
	}
	Drawable GetDrawable() const
	{
		return Drawable( *this );
	}
	void Update( float dt )
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

		// linear (1st order then 0th order)
		vel += Vec2 { 0.0f,-1.0f }.Rotation( angle ) * accel * thrust * dt;
		if( vel.LenSq() > sq( maxSpeed ) )
		{
			vel *= maxSpeed / vel.Len();
		}
		pos += vel * dt;
	}
	void FocusOn( Camera& cam ) const
	{
		cam.MoveTo( pos );
	}
	float GetShieldPercent() const
	{
		return shieldLevel;
	}
	// control functions
	void Thrust()
	{
		thrust = 1.0f;
	}
	void StopThrusting()
	{
		thrust = 0.0f;
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
	// collidable interface
	virtual RectF GetAABB() const override
	{
		return RectF( pos.y - shieldRadius,pos.y + shieldRadius,
			pos.x - shieldRadius,pos.x + shieldRadius );
	}
	virtual Vec2 GetCenter() const override
	{
		return pos;
	}
	virtual float GetRadius() const override
	{
		return (float)shieldRadius;
	}
	virtual Vec2 GetVel() const override
	{
		return vel;
	}
	virtual void Rebound( Vec2 normal ) override
	{
		if( shieldLevel > 0.0f )
		{
			shieldLevel = max( 0.0f,
				shieldLevel - ( ( -vel * normal ) / maxSpeed ) * maxDamage );
		}
		else
		{
			Notify();
		}
		vel -= normal * ( vel * normal ) * 2.0f;
	}


private:
	// stats
	float shieldLevel = 1.0f;
	const float maxDamage = 0.2f;

	// structural
	Surface shipTexture;
	const float shipScale = 0.27f;
	std::array<Vertex, 4> quad;
	const Vec2 shipCenter = {0.0f,6.0f};
	const int shieldRadius = 50;
	const Color shieldColor = GREEN;

	// linear
	Vec2 pos;
	Vec2 vel = { 0.0f,0.0f };
	const float accel = 0.2f * 60.0f * 60.0f;
	const float maxSpeed = 450.0f;
	float thrust = 0.0f;

	// angular
	float angle = 0.0f;
	float angVel = 0.0f;
	const float maxAngVel = 2.5f * PI;
	const float angAccel = 0.004f * 60.0f * 60.0f;
	float angAccelDir = 0.0f;
};