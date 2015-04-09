/****************************************************************************************** 
 *	Chili DirectX Framework Version 14.03.22											  *	
 *	D3DGraphics.cpp																		  *
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
#include "D3DGraphics.h"
#include "ChiliMath.h"
#include <assert.h>
#include <functional>
#pragma comment( lib,"d3d9.lib" )

D3DGraphics::D3DGraphics( HWND hWnd )
	:
pDirect3D( NULL ),
pDevice( NULL ),
pBackBuffer( NULL ),
sysBuffer( SCREENWIDTH,SCREENHEIGHT )
{
	HRESULT result;
	
	pDirect3D = Direct3DCreate9( D3D_SDK_VERSION );
	assert( pDirect3D != NULL );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp,sizeof( d3dpp ) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    result = pDirect3D->CreateDevice( D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,&d3dpp,&pDevice );
	assert( !FAILED( result ) );

	result = pDevice->GetBackBuffer( 0,0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer );
	assert( !FAILED( result ) );
}

D3DGraphics::~D3DGraphics()
{
	if( pDevice )
	{
		pDevice->Release();
		pDevice = NULL;
	}
	if( pDirect3D )
	{
		pDirect3D->Release();
		pDirect3D = NULL;
	}
	if( pBackBuffer )
	{
		pBackBuffer->Release();
		pBackBuffer = NULL;
	}
}

void D3DGraphics::BeginFrame()
{
	sysBuffer.Clear( FILLVALUE );
}

void D3DGraphics::EndFrame()
{
	HRESULT result;
	D3DLOCKED_RECT backRect;

	result = pBackBuffer->LockRect( &backRect,NULL,NULL );
	assert( !FAILED( result ) );

	sysBuffer.Present( backRect.Pitch,(BYTE*)backRect.pBits );

	result = pBackBuffer->UnlockRect( );
	assert( !FAILED( result ) );

	result = pDevice->Present( NULL,NULL,NULL,NULL );
	assert( !FAILED( result ) );
}

void D3DGraphics::PutPixel( int x,int y,Color c )
{
	sysBuffer.PutPixel( x,y,c );
}

void D3DGraphics::PutPixelAlpha( int x,int y,Color c )
{
	// load source pixel
	const Color d = sysBuffer.GetPixel( x,y );

	// blend channels
	const unsigned char rsltRed = ( c.r * c.x + d.r * ( 255 - c.x ) ) / 255;
	const unsigned char rsltGreen = ( c.g * c.x + d.g * ( 255 - c.x ) ) / 255;
	const unsigned char rsltBlue = ( c.b * c.x + d.b * ( 255 - c.x ) ) / 255;

	// pack channels back into pixel and fire pixel onto backbuffer
	sysBuffer.PutPixel( x,y,{ rsltRed,rsltGreen,rsltBlue } );
}

Color D3DGraphics::GetPixel( int x,int y ) const
{
	return sysBuffer.GetPixel( x,y );
}

// inclusive (can never decide which is better)
void D3DGraphics::DrawRectangle( int left,int right,int top,int bottom,Color c )
{
	for( int x = left; x <= right; x++ )
	{
		for( int y = top; y <= bottom; y++ )
		{
			PutPixel( x,y,c );
		}
	}
}

// Cohen–Sutherland clipping algorithm clips a line from
// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with 
// diagonal from (xmin, ymin) to (xmax, ymax).
void D3DGraphics::DrawLineClip( Vec2 p0,Vec2 p1,Color color,const RectF& clip )
{
	enum OutCode
	{
		INSIDE = 0, // 0000
		LEFT = 1,   // 0001
		RIGHT = 2,  // 0010
		BOTTOM = 4, // 0100
		TOP = 8     // 1000
	};

	std::function< OutCode( float,float ) > ComputeOutCode =
		[ &clip ]( float x,float y ) -> OutCode
	{
		OutCode code = INSIDE;   // initialised as being inside of clip window

		if( x < clip.left )           // to the left of clip window
			code = (OutCode)(code | LEFT);
		else if( x > clip.right )      // to the right of clip window
			code = (OutCode)(code | RIGHT);
		if( y < clip.top )           // below the clip window
			code = (OutCode)(code | BOTTOM);
		else if( y > clip.bottom )      // above the clip window
			code = (OutCode)(code | TOP);

		return code;
	};

	// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
	OutCode outcode0 = ComputeOutCode( p0.x,p0.y );
	OutCode outcode1 = ComputeOutCode( p1.x,p1.y );
	bool accept = false;

	while( true )
	{
		if( !(outcode0 | outcode1) )
		{ // Bitwise OR is 0. Trivially accept and get out of loop
			accept = true;
			break;
		}
		else if( outcode0 & outcode1 )
		{ // Bitwise AND is not 0. Trivially reject and get out of loop
			break;
		}
		else
		{
			// failed both tests, so calculate the line segment to clip
			// from an outside point to an intersection with clip edge
			float x,y;

			// At least one endpoint is outside the clip rectangle; pick it.
			OutCode outcodeOut = outcode0 ? outcode0 : outcode1;

			// Now find the intersection point;
			// use formulas y = p0.y + slope * (x - p0.x), x = p0.x + (1 / slope) * (y - p0.y)
			if( outcodeOut & TOP )
			{           // point is above the clip rectangle
				x = p0.x + (p1.x - p0.x) * (clip.bottom - p0.y) / (p1.y - p0.y);
				y = clip.bottom;
			}
			else if( outcodeOut & BOTTOM )
			{ // point is below the clip rectangle
				x = p0.x + (p1.x - p0.x) * (clip.top - p0.y) / (p1.y - p0.y);
				y = clip.top;
			}
			else if( outcodeOut & RIGHT )
			{  // point is to the right of clip rectangle
				y = p0.y + (p1.y - p0.y) * (clip.right - p0.x) / (p1.x - p0.x);
				x = clip.right;
			}
			else if( outcodeOut & LEFT )
			{   // point is to the left of clip rectangle
				y = p0.y + (p1.y - p0.y) * (clip.left - p0.x) / (p1.x - p0.x);
				x = clip.left;
			}

			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if( outcodeOut == outcode0 )
			{
				p0.x = x;
				p0.y = y;
				outcode0 = ComputeOutCode( p0.x,p0.y );
			}
			else
			{
				p1.x = x;
				p1.y = y;
				outcode1 = ComputeOutCode( p1.x,p1.y );
			}
		}
	}
	if( accept )
	{
		DrawLine( (int)(p0.x + 0.5),(int)(p0.y + 0.5),(int)(p1.x + 0.5),(int)(p1.y + 0.5),color );
	}
}

void D3DGraphics::DrawLine( int x1,int y1,int x2,int y2,Color c )
{	
	const int dx = x2 - x1;
	const int dy = y2 - y1;

	if( dy == 0 && dx == 0 )
	{
		PutPixel( x1,y1,c );
	}
	else if( abs( dy ) > abs( dx ) )
	{
		if( dy < 0 )
		{
			int temp = x1;
			x1 = x2;
			x2 = temp;
			temp = y1;
			y1 = y2;
			y2 = temp;
		}
		const float m = (float)dx / (float)dy;
		const float b = x1 - m*y1;
		for( int y = y1; y <= y2; y = y + 1 )
		{
			int x = (int)(m*y + b + 0.5f);
			PutPixel( x,y,c );
		}
	}
	else
	{
		if( dx < 0 )
		{
			int temp = x1;
			x1 = x2;
			x2 = temp;
			temp = y1;
			y1 = y2;
			y2 = temp;
		}
		const float m = (float)dy / (float)dx;
		const float b = y1 - m*x1;
		for( int x = x1; x <= x2; x = x + 1 )
		{
			int y = (int)(m*x + b + 0.5f);
			PutPixel( x,y,c );
		}
	}
}

void D3DGraphics::DrawCircle( int centerX,int centerY,int radius,Color color )
{
	int rSquared = sq( radius );
	int xPivot = (int)( radius * 0.70710678118f + 0.5f );
	for( int x = 0; x <= xPivot; x++ )
	{
		int y = (int)(sqrt( (float)( rSquared - sq( x ) ) ) + 0.5f);
		PutPixel( centerX + x,centerY + y,color );
		PutPixel( centerX - x,centerY + y,color );
		PutPixel( centerX + x,centerY - y,color );
		PutPixel( centerX - x,centerY - y,color );
		PutPixel( centerX + y,centerY + x,color );
		PutPixel( centerX - y,centerY + x,color );
		PutPixel( centerX + y,centerY - x,color );
		PutPixel( centerX - y,centerY - x,color );
	}
}

void D3DGraphics::DrawFlatTriangle( float y0,float y1,float m0,float b0,float m1,float b1,const RectI& clip,Color c )
{
	const int yStart = max( (int)( y0 + 0.5f ),clip.top );
	const int yEnd = min( (int)( y1 + 0.5f ),clip.bottom + 1 );

	for( int y = yStart; y < yEnd; y++ )
	{
		const int xStart = max( int( m0 * ( float( y ) + 0.5f ) + b0 + 0.5f ),clip.left );
		const int xEnd   = min( int( m1 * ( float( y ) + 0.5f ) + b1 + 0.5f ),clip.right + 1 );

		for( int x = xStart; x < xEnd; x++ )
		{
			PutPixel( x,y,c );
		}
	}
}

void D3DGraphics::DrawTriangle( Vec2 v0,Vec2 v1,Vec2 v2,const RectI& clip,Color c )
{
	if( v1.y < v0.y ) v0.Swap( v1 );
	if( v2.y < v1.y ) v1.Swap( v2 );
	if( v1.y < v0.y ) v0.Swap( v1 );

	if( v0.y == v1.y )
	{
		if( v1.x < v0.x ) v0.Swap( v1 );
		const float m1 = ( v0.x - v2.x ) / ( v0.y - v2.y );
		const float m2 = ( v1.x - v2.x ) / ( v1.y - v2.y );
		float b1 = v0.x - m1 * v0.y;
		float b2 = v1.x - m2 * v1.y;
		DrawFlatTriangle( v1.y,v2.y,m1,b1,m2,b2,clip,c );
	}
	else if( v1.y == v2.y )
	{
		if( v2.x < v1.x ) v1.Swap( v2 );
		const float m0 = ( v0.x - v1.x ) / ( v0.y - v1.y );
		const float m1 = ( v0.x - v2.x ) / ( v0.y - v2.y );
		float b0 = v0.x - m0 * v0.y;
		float b1 = v0.x - m1 * v0.y;
		DrawFlatTriangle( v0.y,v1.y,m0,b0,m1,b1,clip,c );
	}
	else
	{
		const float m0 = ( v0.x - v1.x ) / ( v0.y - v1.y );
		const float m1 = ( v0.x - v2.x ) / ( v0.y - v2.y );
		const float m2 = ( v1.x - v2.x ) / ( v1.y - v2.y );
		float b0 = v0.x - m0 * v0.y;
		float b1 = v0.x - m1 * v0.y;
		float b2 = v1.x - m2 * v1.y;

		const float qx = m1 * v1.y + b1;

		if( qx < v1.x )
		{
			DrawFlatTriangle( v0.y,v1.y,m1,b1,m0,b0,clip,c );
			DrawFlatTriangle( v1.y,v2.y,m1,b1,m2,b2,clip,c );
		}
		else
		{
			DrawFlatTriangle( v0.y,v1.y,m0,b0,m1,b1,clip,c );
			DrawFlatTriangle( v1.y,v2.y,m2,b2,m1,b1,clip,c );
		}
	}
}

void D3DGraphics::DrawTriangleTex( Vertex v0,Vertex v1,Vertex v2,const RectI& clip,const Surface &tex )
{
	if (v1.v.y < v0.v.y) v0.Swap(v1);
	if (v2.v.y < v1.v.y) v1.Swap(v2);
	if (v1.v.y < v0.v.y) v0.Swap(v1);

	if (v0.v.y == v1.v.y)
	{
		if (v1.v.x < v0.v.x) v0.Swap(v1);
		DrawFlatTopTriangleTex(v0, v1, v2, clip, tex);
	}
	else if (v1.v.y == v2.v.y)
	{
		if (v2.v.x < v1.v.x) v1.Swap(v2);
		DrawFlatBottomTriangleTex(v0, v1, v2, clip, tex);
	}
	else
	{
		// Calculate intermediate vertex point on major segment
		const Vec2 v = { ((v2.v.x - v0.v.x) / (v2.v.y - v0.v.y)) * 
			(v1.v.y - v0.v.y) + v0.v.x, v1.v.y };

		// Calculate uv coordinates for intermediate vertex point
		const Vec2 t = v0.t + (v2.t - v0.t) * ((v.y - v0.v.y) / (v2.v.y - v0.v.y));

		// Compose intermediate vertex
		const Vertex vi = { v, t };

		// If major right
		if (v1.v.x < vi.v.x)
		{
			DrawFlatBottomTriangleTex(v0, v1, vi, clip, tex);
			DrawFlatTopTriangleTex(v1, vi, v2, clip, tex);
		}
		else
		{
			DrawFlatBottomTriangleTex(v0, vi, v1, clip, tex);
			DrawFlatTopTriangleTex(vi, v1, v2, clip, tex);
		}
	}
}

void D3DGraphics::DrawFlatTopTriangleTex( Vertex v0,Vertex v1,Vertex v2,const RectI& clip,const Surface &tex)
{
	// To do: Replace v2 - v0 with precalculated height
	// calulcate slopes in screen space
	float m0 = (v2.v.x - v0.v.x) / (v2.v.y - v0.v.y);
	float m1 = (v2.v.x - v1.v.x) / (v2.v.y - v1.v.y);

	// calculate start and end scanlines
	const int yStart = max((int)ceilf(v0.v.y), clip.top);
	const int yEnd = min((int)ceilf(v2.v.y) - 1, clip.bottom);

	// calculate uv edge unit steps
	const Vec2 tEdgeStepL = (v2.t - v0.t) / (v2.v.y - v0.v.y);
	const Vec2 tEdgeStepR = (v2.t - v1.t) / (v2.v.y - v0.v.y);

	// calculate uv edge prestep
	Vec2 tEdgeL = v0.t + tEdgeStepL * (float(yStart) - v1.v.y);
	Vec2 tEdgeR = v1.t + tEdgeStepR * (float(yStart) - v1.v.y);

	for (int y = yStart; y <= yEnd; y++, tEdgeL += tEdgeStepL, tEdgeR += tEdgeStepR)
	{
		// caluclate start and end points
		const float px0 = m0 * (float(y) - v0.v.y) + v0.v.x;
		const float px1 = m1 * (float(y) - v1.v.y) + v1.v.x;

		// calculate start and end pixels
		const int xStart = max((int)ceilf(px0), clip.left);
		const int xEnd = min((int)ceilf(px1) - 1, clip.right);

		// calculate uv scanline step
		const Vec2 tScanStep = (tEdgeR - tEdgeL) / (px1 - px0);
		
		// calculate uv point prestep
		Vec2 t = tEdgeL + tScanStep * (float(xStart) - px0);

		for (int x = xStart; x <= xEnd; x++, t += tScanStep)
		{
			const Color texel = tex.GetPixel(
				(unsigned int)(t.x + 0.5f), (unsigned int)(t.y + 0.5f));
			if (texel.x == 255)
			{
				PutPixel(x, y, texel);
			}			
		}
	}
}

void D3DGraphics::DrawFlatBottomTriangleTex( Vertex v0,Vertex v1,Vertex v2,const RectI& clip,const Surface &tex )
{
	// To do: Replace v2 - v0 with precalculated height
	// calulcate slopes in screen space
	float m0 = (v1.v.x - v0.v.x) / (v1.v.y - v0.v.y);
	float m1 = (v2.v.x - v0.v.x) / (v2.v.y - v0.v.y);

	// calculate start and end scanlines
	const int yStart = max((int)ceilf(v0.v.y), clip.top);
	const int yEnd = min((int)ceilf(v2.v.y) - 1, clip.bottom);

	// calculate uv edge unit steps
	const Vec2 tEdgeStepL = (v1.t - v0.t) / (v1.v.y - v0.v.y);
	const Vec2 tEdgeStepR = (v2.t - v0.t) / (v1.v.y - v0.v.y);

	// calculate uv edge prestep
	Vec2 tEdgeL = v0.t + tEdgeStepL * (float(yStart) - v0.v.y);
	Vec2 tEdgeR = v0.t + tEdgeStepR * (float(yStart) - v0.v.y);

	for (int y = yStart; y <= yEnd; y++, tEdgeL += tEdgeStepL, tEdgeR += tEdgeStepR)
	{
		// caluclate start and end points
		const float px0 = m0 * (float(y) - v0.v.y) + v0.v.x;
		const float px1 = m1 * (float(y) - v0.v.y) + v0.v.x;

		// calculate start and end pixels
		const int xStart = max((int)ceilf(px0), clip.left);
		const int xEnd = min((int)ceilf(px1) - 1, clip.right);

		// calculate uv scanline step
		const Vec2 tScanStep = (tEdgeR - tEdgeL) / (px1 - px0);

		// calculate uv point prestep
		Vec2 t = tEdgeL + tScanStep * (float(xStart) - px0);

		for (int x = xStart; x <= xEnd; x++, t += tScanStep)
		{
			const Color texel = tex.GetPixel(
				(unsigned int)(t.x + 0.5f), (unsigned int)(t.y + 0.5f));
			if (texel.x == 255)
			{
				PutPixel(x, y, texel);
			}
		}
	}
}

