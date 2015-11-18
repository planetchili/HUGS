/******************************************************************************************
*	Chili DirectX Framework Version 14.03.22											  *
*	ChiliMath.h																			  *
*	Copyright 2014 PlanetChili.net <http://www.planetchili.net>							  *
*																						  *
*	This file is part of The Chili DirectX Framework.									  *
*																						  *
*	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
*	it under the terms of the GNU General Public License as published by				  *
*	the Free Software Foundation, either version 3 of the License, or					  *
*	(at your option) any later version.													  *
*																						  *
*	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
*	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
*	GNU General Public License for more details.										  *
*																						  *
*	You should have received a copy of the GNU General Public License					  *
*	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
******************************************************************************************/
#pragma once
#include <math.h>

#define PI 3.14159265f

template <typename T>
inline auto sq(T val) -> decltype( val * val )
{
	return val * val;
}

template <typename T> 
inline T sgn( T val )
{
	return (T)( (T)0 <= val ) - ( val < (T)0 );
}

template <class T> T gaussian( T x,T o )
{
	return ( ( T )1.0 / sqrt( ( T )2.0 * PI * sq( o ) ) ) * exp( -sq( x ) / ( ( T )2.0 * sq( o ) ) );
}

template <typename T>
inline T wrapzero( T val,T upper )
{
	const T modded = fmod( val,upper );
	return modded >= ( T )0.0 ? 
		modded :
		upper - modded;
}

template<>
inline int wrapzero( int val,int upper )
{
	return val >= 0 ?
		val % upper :
		upper - ( -val ) % upper;
}