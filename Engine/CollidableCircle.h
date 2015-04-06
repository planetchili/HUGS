#pragma once

class CollidableCircle
{
public:
	virtual RectF GetAABB() const = 0;
	virtual Vec2 GetVel() const = 0;
	virtual float GetRadius() const = 0;
	virtual Vec2 GetCenter() const = 0;
	virtual void Rebound( Vec2 normal ) = 0;
};