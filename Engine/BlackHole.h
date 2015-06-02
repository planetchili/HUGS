#pragma once
#include "Drawable.h"
#include "CollidableCircle.h"
#include "PhysicalCircle.h"
#include "D3DGraphics.h"
#include "TexturedQuad.h"
#include "ChiliMath.h"
#include <array>

class BlackHole : public CollidableCircle
{
public:
	BlackHole( Vec2 pos )
		:
		CollidableCircle( 800.0f,pos ),
		holeQuad(L"bhole.png",2.3f),
		bias( kGravity / sq( radius ) )
	{}
	BlackHole( BlackHole&& donor )
		:
		CollidableCircle( donor ),
		bias( donor.bias ),
		angle( donor.angle ),
		holeQuad( std::move( donor.holeQuad ) )
	{}
	TexturedQuad::Drawable GetDrawable() const
	{
		TexturedQuad::Drawable drawable = holeQuad.GetDrawable();
		drawable.Transform( Mat3::Translation( pos ) * Mat3::Rotation( angle ) );
		return drawable;
	}
	void Update( float dt )
	{
		angle += angVel * dt;
		// clamp angle to within 2pi
		angle = fmodf( angle,2.0f * PI );
	}
protected:
	virtual void HandleCollision( PhysicalCircle& obj ) override
	{
		const Vec2 displacement = pos - obj.GetCenter();
		const float distSq = displacement.LenSq();
		if( distSq <= sq( radius ) )
		{
			obj.ApplyForce( displacement.GetNorm() * ( kGravity / distSq - bias ) );
		}
	}
private:
	const float kGravity = 100000000.0f;
	const float bias;
	TexturedQuad holeQuad;
	const float angVel = -PI / 2.0f;
	float angle = 0.0f;
};