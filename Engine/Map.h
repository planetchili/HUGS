#pragma once
#include "PolyClosed.h"
#include "TriangleStrip.h"

class Map
{
private:
	class Loader : public DL_CreationAdapter
	{
	public:
		Loader( std::string filename,Map& parent )
			:
			parent( parent )
		{
			auto pDxf = std::make_unique< DL_Dxf >();
			pDxf->in( filename,this );
			// @#@ talk about lack of error checking (both boundaries defined etc.)
		}
		virtual void addVertex( const DL_VertexData& data ) override
		{
			vertices.push_back( { (float)data.x,-(float)data.y } );
		}
		virtual void addCircle( const DL_CircleData& data ) override
		{
			parent.startPosition = { (float)data.cx,(float)-data.cy };
		}
		// @#@ talk about how figured out api functions without reference (this intelli)
		virtual void addPolyline( const DL_PolylineData& data ) override
		{
			addingPolyline = true;
			vertices.reserve( data.number );
		}
		virtual void endEntity() override
		{
			if( addingPolyline )
			{
				addingPolyline = false;
				if( attributes.getLayer() == "innerboundary" )
				{
					parent.pInnerBoundary = std::make_unique< PolyClosed >(
						std::move( vertices ),PolyClosed::MakeOutwardCoefficient() );
					parent.pInnerModel = std::make_unique< TriangleStrip >(
						parent.pInnerBoundary->ExtractStripVertices( parent.wallWidth ) );
				}
				else if( attributes.getLayer() == "outerboundary" )
				{
					parent.pOuterBoundary = std::make_unique< PolyClosed >(
						std::move( vertices ),PolyClosed::MakeInwardCoefficient() );
					parent.pOuterModel = std::make_unique< TriangleStrip >(
						parent.pOuterBoundary->ExtractStripVertices( parent.wallWidth ) );
				}
				else
				{
					vertices.clear();
				}
			}
		}
	private:
		bool addingPolyline = false;
		Map& parent;
		std::vector< const Vec2 > vertices;
	};
	// @#@ copied and pasted from ship
	class Drawable : public ::Drawable
	{
	public:
		Drawable( const Map& parent )
			:
			parent( parent )
		{}
		virtual void Rasterize( D3DGraphics& gfx ) const override
		{
			TriangleStrip::Drawable innerDrawable = parent.pInnerModel->GetDrawable();
			TriangleStrip::Drawable outerDrawable = parent.pOuterModel->GetDrawable();
			innerDrawable.Transform( trans );
			outerDrawable.Transform( trans );
			innerDrawable.Clip( clip );
			outerDrawable.Clip( clip );
			innerDrawable.Rasterize( gfx );
			outerDrawable.Rasterize( gfx );
		}
	private:
		const Map& parent;
	};
public:
	Map( std::string filename )
	{
		Loader loader( filename,*this );
	}
	Drawable GetDrawable( ) const
	{
		return Drawable( *this );
	}
	void HandleCollision( CollidableCircle& obj )
	{
		pOuterBoundary->HandleCollision( obj );
		pInnerBoundary->HandleCollision( obj );
	}
	Vec2 GetStartPosition() const
	{
		return startPosition;
	}

private:
	const float wallWidth = 40.0f;
	Vec2 startPosition;
	std::unique_ptr< PolyClosed > pInnerBoundary;
	std::unique_ptr< TriangleStrip > pInnerModel;
	std::unique_ptr< PolyClosed > pOuterBoundary;
	std::unique_ptr< TriangleStrip > pOuterModel;
};