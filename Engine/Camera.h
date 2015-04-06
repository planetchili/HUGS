#pragma once
#include "DrawTarget.h"
#include "Vec2.h"

class Camera : public DrawTarget
{
public:
	Camera( DrawTarget& next,float width,float height )
		:
		next( next ),
		pos( { 0.0f,0.0f } ),
		toCenter( { width / 2.0f,height / 2.0f } )
	{}
	virtual void Draw( Drawable& obj ) override
	{
		obj.Transform( Mat3::Translation( -pos ) );
		next.Draw( obj );
	}
	void MoveTo( Vec2 newPos )
	{
		pos = newPos - toCenter;
	}

private:
	DrawTarget& next;
	Vec2 pos;
	Vec2 toCenter;
};