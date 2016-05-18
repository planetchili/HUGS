#pragma once
#include "Vec2.h"
#include "Colors.h"
#include "D3DGraphics.h"
#include "dxflib\dl_creationadapter.h"
#include "dxflib\dl_dxf.h"
#include "Mat3.h"
#include "Drawable.h"
#include "TriangleStrip.h"
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
			auto pDxf = std::make_unique< DL_Dxf >();
			pDxf->in( filename,this );
		}
		virtual void addVertex( const DL_VertexData& data ) override
		{
			vertices.push_back( { (float)data.x,-(float)data.y } );
		}
		operator std::vector< Vec2 > && ( )
		{
			return std::move( vertices );
		}
	private:
		std::vector< Vec2 > vertices;
	};
public:
	class Drawable : public ::Drawable
	{
	public:
		Drawable( const PolyClosed& parent,Color color )
			:
			parent( parent ),
			color( color )
		{}
		virtual void Rasterize( D3DGraphics& gfx ) const override
		{
			for( auto i = parent.vertices.begin(),end = parent.vertices.end() - 1;
				i != end; i++ )
			{
				gfx.DrawLineClip( trans * *i,trans * *( i + 1 ),color,clip );
			}
			gfx.DrawLineClip( trans * parent.vertices.back(),
				trans * parent.vertices.front(),color,clip );
		}
	private:
		const Color color;
		const PolyClosed& parent;
	};
public:
	PolyClosed( std::initializer_list< Vec2 > vList,float facingCoefficient = -1.0f )
		:
		vertices( vList ),
		facingCoefficient( facingCoefficient )
	{
		RemoveDuplicates();
		MakeClockwise();
	}
	PolyClosed( std::string filename,float facingCoefficient = -1.0f )
		:
		vertices( Loader( filename ) ),
		facingCoefficient( facingCoefficient )
	{
		RemoveDuplicates();
		MakeClockwise();
	}
	PolyClosed( std::vector< Vec2 >&& vList,float facingCoefficient = -1.0f )
		:
		vertices( std::move( vList ) ),
		facingCoefficient( facingCoefficient )
	{
		RemoveDuplicates();
		MakeClockwise();
	}
	Drawable GetDrawable( Color color ) const
	{
		return Drawable( *this,color );
	}
	TriangleStrip GenerateWallStrip( const float width ) const
	{
		std::vector< Vec2 > stripVertices;

		for( auto i = vertices.begin(),end = vertices.end() - 2;
			i != end; i++ )
		{
			stripVertices.push_back( *( i + 1 ) );
			const Vec2 n0 = ( *( i + 1 ) - *( i + 0 ) ).CCW90().Normalize()
				* facingCoefficient;
			const Vec2 n1 = ( *( i + 2 ) - *( i + 1 ) ).CCW90().Normalize()
				* facingCoefficient;
			const Vec2 b = ( n0 + n1 ).Normalize();
			const float k = width / ( b * n0 );
			const Vec2 q = *( i + 1 ) + ( b * k );
			stripVertices.push_back( q );
		}
		{
			stripVertices.push_back( vertices.back() );
			const Vec2 n0 = ( vertices.back() - *( vertices.end() - 2 ) ).CCW90().Normalize()
				* facingCoefficient;
			const Vec2 n1 = ( vertices.front() - vertices.back() ).CCW90().Normalize()
				* facingCoefficient;
			const Vec2 b = ( n0 + n1 ).Normalize();
			const float k = width / ( b * n0 );
			const Vec2 q = vertices.back() + ( b * k );
			stripVertices.push_back( q );
		}
		{
			stripVertices.push_back( vertices.front() );
			const Vec2 n0 = ( vertices.front() - vertices.back() ).CCW90().Normalize()
				* facingCoefficient;
			const Vec2 n1 = ( *( vertices.begin() + 1 ) - vertices.front() ).CCW90().Normalize()
				* facingCoefficient;
			const Vec2 b = ( n0 + n1 ).Normalize();
			const float k = width / ( b * n0 );
			const Vec2 q = vertices.front() + ( b * k );
			stripVertices.push_back( q );
		}
		stripVertices.push_back( stripVertices[0] );
		stripVertices.push_back( stripVertices[1] );

		return TriangleStrip( std::move( stripVertices ) );
	}
	static float MakeInwardCoefficient()
	{
		return 1.0f;
	}
	static float MakeOutwardCoefficient()
	{
		return -1.0f;
	}
	std::vector< TriangleStrip > ExtractSolidStrips()
	{
		// make the list of input vertices
		std::list< Vec2 > inputVerts( vertices.begin(),vertices.end() );
		inputVerts.reverse();

		// make the vector of output triangle strips
		std::vector< TriangleStrip > strips;

		// lambda to consume vertices of polyclosed and generate a triangle strip
		const auto ConsumeVertices = [&inputVerts]()
		{
			// need at least 1 triangle in the set
			assert( inputVerts.size() > 2 );
			auto sizo = inputVerts.size();

			// make the output vertice vector
			std::vector< Vec2 > vertices;

			// lambda function to check if vertex is prunable
			auto IsPrunable = [&inputVerts]( std::list< Vec2 >::const_iterator b ) -> bool
			{
				// initialize iterators to the three vertices of the triangle
				auto a = std::prev( b );
				auto c = std::next( b );
				// return false if not convex to the polygon
				if( !CornerIsConvex( *a,*b,*c ) )
				{
					return false;
				}
				// return false if there is a vertex before the triangle inside
				for( auto iPre = inputVerts.cbegin(); iPre != a; iPre++ )
				{
					if( TriangleContainsPoint( *a,*b,*c,*iPre ) )
					{
						return false;
					}
				}
				// return false if there is a vertex after the triangle inside
				for( auto iPost = std::next( c ),end = inputVerts.cend(); iPost != end; iPost++ )
				{
					if( TriangleContainsPoint( *a,*b,*c,*iPost ) )
					{
						return false;
					}
				}
				// if we get this far, vertex pointed to by iterator b is prunable
				return true;
			};

			for( auto end = std::prev( inputVerts.end() ),
				i = std::next( inputVerts.begin() ); i != end; i++ )
			{
				if( IsPrunable( i ) )
				{
					// i is prunable, start the triangle strip
					// add the first triangle
					vertices.push_back( *i );
					vertices.push_back( *std::prev( i ) );
					vertices.push_back( *std::next( i ) );
					// remove vertice i from the list and iterate ccw
					inputVerts.erase( std::next( --i ) );
					// keep adding to strip while there are still triangles left
					while( true )
					{
						// ccw iteration
						// if size is less than 3 (number of vertices needed for triangle)
						size_t size = inputVerts.size();
						if( size < 3 )
						{
							// quit the while loop
							break;
						}
						// if ccw vertice is at extent
						auto begin = inputVerts.begin();
						if( i == begin )
						{
							// rotate list so that begin is at the middle
							inputVerts.splice( begin,inputVerts,
								std::next( begin,size / 2 + size % 2 ),inputVerts.end() );
						}
						// if ccw vertice is not prunable
						if( !IsPrunable( i ) )
						{
							// quit the while loop
							break;
						}
						else
						{
							// add new vertice (triangle)
							vertices.push_back( *std::prev( i ) );
							// remove vertice i from the list and iterate ccw
							inputVerts.erase( std::prev( ++i ) );
						}

						// cw iteration
						// if size is less than 3 (number of vertices needed for triangle)
						size = inputVerts.size();
						if( size < 3 )
						{
							// quit the while loop
							break;
						}
						// if cw vertice is at extent
						auto last = std::prev( inputVerts.end() );
						if( i == last )
						{
							// rotate list so that last is at the middle
							begin = inputVerts.begin();
							inputVerts.splice( begin,std::move( inputVerts ),
								std::next( begin,size / 2 ),inputVerts.end() );
						}
						// if cw vertice is not prunable
						if( !IsPrunable( i ) )
						{
							// quit the while loop
							break;
						}
						else
						{
							// add new vertice (triangle)
							vertices.push_back( *std::next( i ) );
							// remove vertice i from the list and iterate ccw
							inputVerts.erase( std::next( --i ) );
						}
					}
					// quit the for loop
					break;
				}
			}
			// return the triangle strip
			return TriangleStrip( std::move( vertices ) );
		};

		// keep extracting strips from the vertice list until no more triangles remain
		while( inputVerts.size() > 2 )
		{
			strips.push_back( std::move( ConsumeVertices() ) );
		}

		// return the vector of strips
		return std::move( strips );
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
			std::reverse( vertices.begin(),vertices.end() );
		}
	}
	void RemoveDuplicates()
	{
		for( auto i = vertices.begin(),end = std::prev( vertices.end() ); i != end; )
		{
			if( ( *i - *std::next( i ) ).Len() < fuseThreshold )
			{
				i = vertices.erase( i );
				end = std::prev( vertices.end() );
			}
			else
			{
				i++;
			}
		}
		if( ( vertices.back() - vertices.front() ).Len() < fuseThreshold )
		{
			vertices.pop_back();
		}
	}

protected:
	const float fuseThreshold = 0.01f;
	// +1.0 = inward -1.0 = outward
	const float facingCoefficient;
	std::vector< Vec2 > vertices;
};