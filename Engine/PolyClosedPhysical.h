#pragma once
#include "PolyClosed.h"
#include "PhysicalCircle.h"


class PolyClosedPhysical : public PolyClosed
{
public:
	PolyClosedPhysical( std::initializer_list< Vec2 > vList,float facingCoefficient )
		:
		PolyClosed( vList,facingCoefficient )
	{}
	PolyClosedPhysical( std::string filename,float facingCoefficient )
		:
		PolyClosed( filename,facingCoefficient )
	{}
	PolyClosedPhysical( std::vector< Vec2 >&& vList,float facingCoefficient )
		:
		PolyClosed( std::move( vList ),facingCoefficient )
	{}
	void TestCollision( PhysicalCircle& obj ) const
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
				const Vec2 lineNormal = ( cur - prev ).CW90().Normalize() * facingCoefficient;
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
								HandleCollision( obj,dClosest / sqrt( dSquaredClosest ) );
							}
						}
						else if( cons0 || cons1 )
						{
							HandleCollision( obj,lineNormal );
						}
					}
				}
			}
			prev = cur;
		}
	}
protected:
	virtual void HandleCollision( PhysicalCircle& obj,Vec2 normal ) const = 0;
};