#pragma once
#include "Mat3.h"
#include "Rect.h"

class Drawable
{
public:
	Drawable()
		:
		trans( Mat3::Identity() ),
		clip( -INT_MAX,(INT_MAX - 100),-INT_MAX,(INT_MAX - 100) )
	{
	}
	void Transform( const Mat3& mat )
	{
		trans = mat * trans;
	}
	void Clip( const RectF& rect )
	{
		clip.ClipTo( rect );
	}
	virtual void Rasterize( class D3DGraphics& gfx ) const = 0;
protected:
	Mat3 trans;
	RectF clip;
};