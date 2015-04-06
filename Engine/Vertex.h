#pragma once
#include "Vec2.h"

class Vertex
{
public:
	Vertex() {}
	Vertex(const Vec2 &V, const Vec2 &T)
	{
		v = V;
		t = T;
	}
	Vertex& operator=(const Vertex& rhs)
	{
		v = rhs.v;
		t = rhs.t;
		return *this;
	}
	Vertex& Swap(Vertex& rhs)
	{
		const Vertex temp = *this;
		*this = rhs;
		rhs = temp;
		return *this;
	}

	Vec2 v;
	Vec2 t;
};

