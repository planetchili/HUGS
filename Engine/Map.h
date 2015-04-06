#pragma once
#include "PolyClosed.h"
#include "TriangleStrip.h"

class Map
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
		virtual void addCircle( const DL_CircleData& data ) override
		{
			startPosition = { (float)data.cx,(float)-data.cy };
		}
		operator std::vector< const Vec2 >&&()
		{
			return std::move( vertices );
		}
		Vec2 GetStartPosition() const
		{
			return startPosition;
		}
	private:
		std::vector< const Vec2 > vertices;
		Vec2 startPosition = {0.0f,0.0f};
	};
public:
	Map( std::string filename )
	{
		Loader loader( filename );
		pBoundaries = std::make_unique< PolyClosed >( loader );
		pModel = std::make_unique< TriangleStrip >( 
			pBoundaries->ExtractStripVertices( wallWidth ) );
		startPosition = loader.GetStartPosition();
	}
	TriangleStrip::Drawable GetDrawable( ) const
	{
		return pModel->GetDrawable();
	}
	void HandleCollision( CollidableCircle& obj )
	{
		pBoundaries->HandleCollision( obj );
	}
	Vec2 GetStartPosition() const
	{
		return startPosition;
	}

private:
	const float wallWidth = 40.0f;
	Vec2 startPosition;
	std::unique_ptr< PolyClosed > pBoundaries;
	std::unique_ptr< TriangleStrip > pModel;
};