#pragma once
#include "CollidableCircle.h"

class PhysicalCircle : public CollidableCircle
{
public:
	PhysicalCircle( float radius,float mass,float kDrag,Vec2 pos,Vec2 vel = { 0.0f,0.0f } )
		:
		CollidableCircle( radius,pos ),
		mass( mass ),
		vel( vel ),
		kDrag( kDrag )
	{}
	float GetMass() const
	{
		return mass;
	}
	Vec2 GetVel() const
	{
		return vel;
	}
	void ApplyImpulse( Vec2 i )
	{
		vel += i;
	}
	virtual void Rebound( Vec2 norm )
	{
		ApplyImpulse( -norm * ( norm * vel ) * 2.0f );
	}
	virtual void ApplyForce( Vec2 f )
	{
		netForce += f;
	}
	virtual void Update( float dt )
	{
		// 2nd order motion
		if( vel != Vec2{ 0.0f,0.0f } )
		{
			netForce -= Vec2( vel ).Normalize() * vel.LenSq() * kDrag;
		}
		vel += ( netForce / mass ) * dt;
		netForce = { 0.0f,0.0f };

		// 1st order motion
		pos += vel * dt;
	}

protected:
	Vec2 netForce = { 0.0f,0.0f };
	float mass;
	Vec2 vel;
	float kDrag; // drag coefficient
};