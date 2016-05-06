#pragma once
#include "DrawTarget.h"
#include "Vec2.h"

class Viewport : public DrawTarget
{
public:
	Viewport( DrawTarget& next,const RectF& clipRect,const RectF& viewRect )
		:
		next( next ),
		clip( clipRect ),
		view( viewRect )
	{}
	virtual void Draw( Drawable& obj ) override
	{
		obj.Transform( Mat3::Translation( { (float)view.left,(float)view.top } ) );
		obj.Clip( clip );
		next.Draw( obj );
	}
	float GetWidth() const
	{
		return view.GetWidth();
	}
	float GetHeight() const
	{
		return view.GetHeight();
	}
	float GetClipWidth() const
	{
		return clip.GetWidth();
	}
	float GetClipHeight() const
	{
		return clip.GetHeight();
	}

private:
	DrawTarget& next;
	RectF clip;
	RectF view;
};