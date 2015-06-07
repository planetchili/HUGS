#pragma once
#include <memory>
#include "D3DGraphics.h"

class ScreenContainer
{
	friend class Screen;
protected:
	std::unique_ptr< class Screen > pScreen;
};

class Screen
{
public:
	Screen( ScreenContainer& parent )
		:
		parent( parent )
	{}
	virtual void Update( float dt ) = 0;
	virtual void Draw( D3DGraphics& gfx ) = 0;
	void EndFrame()
	{
		if( onDeck )
		{
			parent.pScreen = std::move( onDeck );
		}
	}
protected:
	void ChangeScreen( std::unique_ptr< Screen > pNewScreen )
	{
		if( !onDeck )
		{
			onDeck = std::move( pNewScreen );
		}
	}
protected:
	ScreenContainer& parent;
private:
	std::unique_ptr< Screen > onDeck;
};