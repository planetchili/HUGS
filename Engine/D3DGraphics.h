/****************************************************************************************** 
 *	Chili DirectX Framework Version 14.03.22											  *	
 *	D3DGraphics.h																		  *
 *	Copyright 2014 PlanetChili <http://www.planetchili.net>								  *
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

#include <d3d9.h>
#include "GdiPlusManager.h"
#include "Vec2.h"
#include "Colors.h"
#include "DrawTarget.h"
#include "Vertex.h"
#include "Surface.h"
#include "BloomProcessor.h"

#define DEFAULT_VIEW_WIDTH 1024u
#define DEFAULT_VIEW_HEIGHT 768u

class D3DGraphics : public DrawTarget
{
public:
	D3DGraphics( HWND hWnd,unsigned int viewWidth = DEFAULT_VIEW_WIDTH,
		unsigned int viewHeight = DEFAULT_VIEW_HEIGHT );
	~D3DGraphics();
	void BeginFrame();
	void EndFrame();
	inline unsigned int GetWidth() const
	{
		return sysBuffer.GetWidth();
	}
	inline unsigned int GetHeight() const
	{
		return sysBuffer.GetHeight();
	}
	inline RectI GetViewRegion() const
	{
		return RectI { 
			int( BloomProcessor::GetFringeSize() ),
			int( GetHeight() - BloomProcessor::GetFringeSize() ),
			int( BloomProcessor::GetFringeSize() ),
			int( GetWidth() - BloomProcessor::GetFringeSize() )
		};
	}
	inline virtual void Draw( Drawable& obj ) override
	{
		obj.Rasterize( *this );
	}
	inline void PutPixel( int x,int y,Color c )
	{
		sysBuffer.PutPixel( x,y,c );
	}
	inline void PutPixelAlpha( int x,int y,Color c )
	{
		sysBuffer.PutPixelAlpha( x,y,c );
	}
	inline Color GetPixel( int x,int y ) const
	{
		return sysBuffer.GetPixel( x,y );
	}
	inline void CopySurface( const Surface& src )
	{
		sysBuffer.Copy( src );
	}
	inline void ProcessBloom()
	{
		processor.Go();
	}
	
	template< typename T >
	inline void DrawRectangleAlpha( const _Rect<T>& rect,Color c )
	{
		DrawRectangle( (int)rect.left,(int)rect.right,(int)rect.top,(int)rect.bottom,c,
			&D3DGraphics::PutPixelAlpha );
	}
	template< typename T >
	inline void DrawRectangleAlpha( _Vec2< T > topLeft,_Vec2< T > bottomRight,Color c )
	{
		DrawRectangle( (int)topLeft.x,(int)bottomRight.x,(int)topLeft.y,(int)bottomRight.y,c,
			&D3DGraphics::PutPixelAlpha );
	}
	inline void DrawRectangleAlpha( int left,int right,int top,int bottom,Color c )
	{
		DrawRectangle( left,right,top,bottom,c,&D3DGraphics::PutPixelAlpha );
	}
	template< typename T >
	inline void DrawRectangle( const _Rect<T>& rect,Color c )
	{
		DrawRectangle( (int)rect.left,(int)rect.right,(int)rect.top,(int)rect.bottom,c );
	}
	template< typename T >
	inline void DrawRectangle( _Vec2< T > topLeft,_Vec2< T > bottomRight,Color c )
	{
		DrawRectangle( (int)topLeft.x,(int)bottomRight.x,(int)topLeft.y,(int)bottomRight.y,c );
	}
	void DrawRectangle( int left,int right,int top,int bottom,Color c,void( D3DGraphics::* )( int,int,Color ) = &D3DGraphics::PutPixel );
	
	template< typename T >
	inline void DrawLine( _Vec2< T > pt1,_Vec2< T > pt2,Color c )
	{
		DrawLine( (int)pt1.x,(int)pt1.y,(int)pt2.x,(int)pt2.y,c );
	}
	void DrawLine( int x1,int y1,int x2,int y2,Color c );
	void DrawLineClip( Vec2 p0,Vec2 p1,Color color,const RectF& clip );
	
	template< typename T >
	inline void DrawCircle( _Vec2< T > center,int radius,Color c )
	{
		DrawCircle( (int)center.x,(int)center.y,radius,c );
	}
	void DrawCircle( int centerX,int centerY,int radius,Color c );
	
	inline void DrawString( const std::wstring& string,Vec2 pt,const Font& font,Color c = WHITE )
	{
		sysBuffer.DrawString( string,pt,font,c );
	}
	inline void DrawString( const std::wstring& string,const RectF &rect,const Font& font,Color c = WHITE,Font::Alignment a = Font::Center )
	{
		sysBuffer.DrawString( string,rect,font,c,a );
	}
	
	void DrawTriangle( Vec2 v0,Vec2 v1,Vec2 v2,const RectI& clip,Color c );
	void DrawTriangleTex( Vertex v0,Vertex v1,Vertex v2,const RectI& clip,const Surface &tex );

private:
	void DrawFlatTopTriangleTex( Vertex v0,Vertex v1,Vertex v2,const RectI& clip,const Surface &tex );
	void DrawFlatBottomTriangleTex( Vertex v0,Vertex v1,Vertex v2,const RectI& clip,const Surface &tex );
	void DrawFlatTriangle( float yStart,float yEnd,float m0,float b0,float m1,float b1,const RectI& clip,Color c );
private:
	GdiPlusManager		gdiManager;
	const Color			FILLVALUE =		BLACK;
	IDirect3D9*			pDirect3D;
	IDirect3DDevice9*	pDevice;
	IDirect3DSurface9*	pBackBuffer;
	TextSurface			sysBuffer;
	__declspec(align(16)) BloomProcessor processor;
};