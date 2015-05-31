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
		CollidableCircle( 500.0f,pos ),
		holeQuad(L"bhole.png",2.3f),
		bias( kGravity / sq( radius ) )
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
		float distSq = displacement.LenSq();
		if( distSq <= sq( radius ) )
		{
			distSq = min( distSq,sq( minDist ) );
			obj.ApplyForce( displacement.GetNorm() * ( kGravity / distSq ) );
		}
	}
private:
	const float kGravity = 1200.0f;
	const float minDist = 1.0f;
	const float bias;
	const TexturedQuad holeQuad;
	float angle = 0.0f;
	const float angVel = 3.14f / 2.2f;
};