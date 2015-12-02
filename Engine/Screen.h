#pragma once
#include <memory>
#include "D3DGraphics.h"
#include <exception>

class ScreenContainer
{
	friend class Screen;
protected:
	std::unique_ptr< class Screen > pScreen;
};

class Screen
{
public:
	class Change : public std::exception
	{};
public:
	Screen( ScreenContainer* parent )
		:
		parent( parent )
	{}
	virtual void HandleInput() = 0;
	virtual void Update( float dt ) = 0;
	virtual void DrawPreBloom( D3DGraphics& gfx ) = 0;
	virtual void DrawPostBloom( D3DGraphics& gfx ) = 0;
protected:
	void SetOtherParent( Screen& other,ScreenContainer* newParent ) const
	{
		other.parent = parent;
	}
	void ChangeScreen( std::unique_ptr< Screen > pNewScreen )
	{
		parent->pScreen = std::move( pNewScreen );
		throw Change();
	}
protected:
	ScreenContainer* parent;
};