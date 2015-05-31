#pragma once
#include "Vec2.h"
#include "Rect.h"

class CollidableCircle
{
public:
	CollidableCircle( float radius,Vec2 pos )
		:
		radius( radius ),
		pos( pos )
	{}
	RectF GetAABB() const
	{
		return RectF( pos.y - radius,pos.y + radius,
			pos.x - radius,pos.x + radius );
	}
	float GetRadius() const
	{
		return radius;
	}
	Vec2 GetCenter() const
	{
		return pos;
	}
	void TestCollision( class PhysicalCircle& other );
protected:
	virtual void HandleCollision( class PhysicalCircle& other ) {}
protected:
	float radius;
	Vec2 pos;
};