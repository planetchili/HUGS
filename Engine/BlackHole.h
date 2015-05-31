#pragma once
#include "Drawable.h"
#include "CollidableCircle.h"
#include "PhysicalCircle.h"
#include "D3DGraphics.h"
#include <array>

class BlackHole : public CollidableCircle
{
public:
	class Drawable : public ::Drawable
	{
	public:
		Drawable( const BlackHole& parent )
			:
			parent( parent )
		{
			Transform( Mat3::Translation( parent.pos ) * Mat3::Rotation( parent.angle ) );
		}
		virtual void Rasterize( D3DGraphics& gfx ) const override
		{
			std::array<Vertex,4 > quadTrans;
			const Mat3 holeTrans = trans * Mat3::Scaling( parent.holeScale );
			for( int i = 0; i < 4; i++ )
			{
				quadTrans[i].t = parent.quad[i].t;
				quadTrans[i].v = holeTrans * parent.quad[i].v;
			}

			gfx.DrawTriangleTex( quadTrans[0],quadTrans[1],quadTrans[3],clip,
				parent.holeTexture );
			gfx.DrawTriangleTex( quadTrans[1],quadTrans[2],quadTrans[3],clip,
				parent.holeTexture );
		}
	private:
		const BlackHole& parent;
	};
public:
	BlackHole( Vec2 pos )
		:
		CollidableCircle( 400.0f,pos ),
		holeTexture( Surface::FromFile( L"bhole.png" ) )
	{
		quad[0].v = { -50.0f,-50.0f };
		quad[0].t = { 0.0f,0.0f };
		quad[1].v = { 49.0f,-50.0f };
		quad[1].t = { 99.0f,0.0f };
		quad[2].v = { 49.0f,49.0f };
		quad[2].t = { 99.0f,99.0f };
		quad[3].v = { -50.0f,49.0f };
		quad[3].t = { 0.0f,99.0f };
	}
	Drawable GetDrawable() const
	{
		return Drawable( *this );
	}
	void Update( float dt )
	{
		angle += angVel * dt;
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
	Surface holeTexture;
	const float holeScale = 1.5f;
	std::array<Vertex,4> quad;
	float angle = 0.0f;
	const float angVel = 3.14f / 2.2f;
};