#pragma once
#include "Drawable.h"

class DrawTarget
{
public:
	virtual void Draw( Drawable& obj ) = 0;
};