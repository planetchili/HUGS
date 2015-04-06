#pragma once
#include "Vec2.h"
#include "Colors.h"
#include "D3DGraphics.h"
#include "dxflib\dl_creationadapter.h"
#include "dxflib\dl_dxf.h"
#include "Mat3.h"
#include "Drawable.h"
#include "CollidableCircle.h"
#include <vector>
#include <memory>
#include <algorithm>


class PolyClosed
{
private:
	class Loader : public DL_CreationAdapter
	{
	public:
		Loader( std::string filename )
		{
			auto pDxf = std::make_unique< DL_Dxf >( );
			pDxf->in( filename,this );
		}
		virtual void addVertex( const DL_VertexData& data ) override
		{
			vertices.push_back( { (float)data.x,-(float)data.y } );
		}
		operator std::vector< const Vec2 >&&() 
		{
			return std::move( vertices );
		}
	private:
		std::vector< const Vec2 > vertices;
	};
public:
	class Drawable : public ::Drawable
	{
	public:
		Drawable( const PolyClosed& parent )
			:
			parent( parent )
		{}
		virtual void Rasterize( D3DGraphics& gfx ) const override
		{
			for( auto i = parent.vertices.begin( ),end = parent.vertices.end( ) - 1; 
				i != end; i++ )
			{
				gfx.DrawLineClip( trans * *i,trans * *( i + 1 ),parent.color,clip );
			}
			gfx.DrawLineClip( trans * parent.vertices.back( ),
				trans * parent.vertices.front( ),parent.color,clip );
		}
	private:
		const PolyClosed& parent;
	};
public:
	PolyClosed( std::initializer_list< Vec2 > vList,Color color = WHITE )
		:
		vertices( vList ),
		color( color )
	{
		MakeClockwise();
	}
	PolyClosed( std::string filename,Color color = WHITE )
		:
		vertices( Loader( filename ) ),
		color( color )
	{
		MakeClockwise();
	}
	void HandleCollision( CollidableCircle& obj )
	{
		const RectF objAABB = obj.GetAABB();
		const Vec2 c = obj.GetCenter();
		const float r = obj.GetRadius();

		Vec2 prev = vertices.back();
		for( auto i = vertices.begin(),end = vertices.end(); i < end; i++ )
		{
			const Vec2 cur = *i;
			const RectF lineAABB( prev,cur );
			if( objAABB.Overlaps( lineAABB ) )
			{
				const Vec2 objVel = obj.GetVel();
				const Vec2 lineNormal = ( cur - prev ).CW90().Normalize();
				if( objVel * lineNormal < 0.0f )
				{
					const std::vector< Vec2 > points = CalculateIntersectionPoints( c,cur,prev,r );
					if( points.size() == 2 )
					{
						const bool cons0 = lineAABB.Contains( points[0] );
						const bool cons1 = lineAABB.Contains( points[1] );

						if( cons0 != cons1 && !lineAABB.Contains( points[0].MidpointWith( points[1] ) ) )
						{
							const Vec2 d1 = c - prev;
							const Vec2 d2 = c - cur;
							const float dSquared1 = d1.LenSq( );
							const float dSquared2 = d2.LenSq( );
							float dSquaredClosest;
							Vec2 dClosest;
							if( dSquared1 <= dSquared2 )
							{
								dClosest = d1;
								dSquaredClosest = dSquared1;
							}
							else
							{
								dClosest = d2;
								dSquaredClosest = dSquared2;
							}

							if( dClosest * objVel < 0.0f )
							{
								obj.Rebound( dClosest / sqrt( dSquaredClosest ) );
							}
						}
						else if( cons0 || cons1 )
						{
							obj.Rebound( lineNormal );
						}
					}
				}
			}
			prev = cur;
		}
	}
	Drawable GetDrawable() const
	{
		return Drawable( *this );
	}
	std::vector< const Vec2 > ExtractStripVertices( const float width ) const
	{
		std::vector< const Vec2 > strip;

		for( auto i = vertices.begin(),end = vertices.end() - 2;
			i != end; i++ )
		{
			strip.push_back( *( i + 1 ) );
			const Vec2 n0 = ( *( i + 1 ) - *( i + 0 ) ).CCW90().Normalize();
			const Vec2 n1 = ( *( i + 2 ) - *( i + 1 ) ).CCW90().Normalize();
			const Vec2 b = ( n0 + n1 ).Normalize();
			const float k = width / (b * n0);
			const Vec2 q = *(i + 1) + (b * k);
			strip.push_back( q );
		}
		{
			strip.push_back( vertices.back() );
			const Vec2 n0 = ( vertices.back() - *( vertices.end() - 2 ) ).CCW90().Normalize();
			const Vec2 n1 = ( vertices.front() - vertices.back() ).CCW90().Normalize();
			const Vec2 b = ( n0 + n1 ).Normalize();
			const float k = width / ( b * n0 );
			const Vec2 q = vertices.back() + ( b * k );
			strip.push_back( q );
		}
		{
			strip.push_back( vertices.front() );
			const Vec2 n0 = ( vertices.front() - vertices.back() ).CCW90().Normalize();
			const Vec2 n1 = ( *( vertices.begin() + 1) - vertices.front() ).CCW90().Normalize();
			const Vec2 b = ( n0 + n1 ).Normalize();
			const float k = width / ( b * n0 );
			const Vec2 q = vertices.front() + ( b * k );
			strip.push_back( q );
		}
		strip.push_back( strip[0] );
		strip.push_back( strip[1] );

		return strip;
	}
private:
	bool IsClockwiseWinding() const
	{
		float area = 0.0f;
		Vec2 prev = vertices.back();
		for( auto i = vertices.begin(),end = vertices.end();
			i != end; i++ )
		{
			const Vec2 cur = *i;
			area += ( cur.x - prev.x ) * ( cur.y + prev.y );
			prev = cur;
		}
		return area <= 0.0f;
	}
	void MakeClockwise()
	{
		if( !IsClockwiseWinding() )
		{
			std::reverse( this->vertices.begin(),this->vertices.end() );
		}
	}

private:
	Color color;
	std::vector< const Vec2 > vertices;
};