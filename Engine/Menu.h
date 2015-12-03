#include "D3DGraphics.h"
#include "Keyboard.h"
#include <vector>
#include <memory>
#include <string>
#include "Rect.h"

class Menu
{
protected:
	class Item
	{
		friend Menu;
	protected:
		Item( const Menu& m )
			:
			borders( m.CalculateItemBorders() ),
			font( m.buttonFont )
		{}
		virtual void Update( float ) = 0;
		virtual void Draw( D3DGraphics& gfx ) const = 0;
		virtual void HandleInput( const KeyEvent& ) = 0;
		bool IsSelected() const
		{
			return selected;
		}
	protected:
		const RectI borders;
		const Font& font;
	private:
		bool selected = false;
	};
	class Button : public Item
	{
	protected:
		Button( const Menu& m,const std::wstring& text )
			:
			Item( m ),
			text( text ),
			colorSelected( m.selectedColor ),
			colorDeselected( m.deselectedColor ),
			textColor( m.textColor )
		{}
		virtual void Update( float ) override {}
		virtual void Draw( D3DGraphics& gfx ) const override
		{
			if( IsSelected() )
			{
				gfx.DrawRectangle( borders,colorSelected );
			}
			else
			{
				gfx.DrawRectangle( borders,colorDeselected );
			}
			gfx.DrawString( text,borders,font,textColor );
		}
		virtual void HandleInput( const KeyEvent& e ) override
		{
			if( e.GetCode() == VK_RETURN && e.IsPress() )
			{
				OnPress();
			}
		}
		virtual void OnPress() = 0;
	protected:
		std::wstring text;
		Color colorSelected;
		Color colorDeselected;
		Color textColor;
	};
public:
	void HandleInput( const KeyEvent& e )
	{
		assert( finalized );
		if( e.IsPress() )
		{
			if( e.GetCode() == VK_UP )
			{
				( *pos )->selected = false;
				if( pos == items.begin() )
				{
					pos = items.end() - 1;
				}
				else
				{
					pos--;
				}
				( *pos )->selected = true;
				return;
			}
			else if( e.GetCode() == VK_DOWN )
			{
				( *pos )->selected = false;
				pos++;
				if( pos == items.end() )
				{
					pos = items.begin();
				}
				( *pos )->selected = true;
				return;
			}
			else if( e.GetCode() == VK_ESCAPE )
			{
				OnExit();
				return;
			}
		}
		(*pos)->HandleInput( e );
	}
	void Draw( D3DGraphics& gfx ) const
	{
		assert( finalized );
		gfx.DrawRectangleAlpha( borders,backColor );
		for( const auto& pItem : items )
		{
			pItem->Draw( gfx );
		}
	}
protected:
	Menu( Vei2 center,int buttonWidth,int padding,int buttonHeight,Color bgc,
		const std::wstring& font,Color textColor,Color selectedColor,
		Color deselectedColor,int nItems = 0 )
		:
		borders( CalculateBorders( center,nItems,buttonWidth,buttonHeight,padding ) ),
		backColor( bgc ),
		padding( padding ),
		buttonHeight( buttonHeight ),
		buttonFont( font,float( buttonHeight - textPadding * 2 - 13 ) ),
		textColor( textColor ),
		selectedColor( selectedColor ),
		deselectedColor( deselectedColor )
	{
		items.reserve( nItems );
	}
	void AddItem( std::unique_ptr<Item> item )
	{
		assert( !finalized );
		items.emplace_back( std::move( item ) );
	}
	void Finalize()
	{
		assert( !finalized );
		pos = items.begin();
		items.front()->selected = true;
		finalized = true;
	}
	virtual void OnExit() = 0;
private:
	RectI CalculateItemBorders() const
	{
		int i = int( items.size() );
		return {
			borders.top + padding + ( buttonHeight + padding ) * i,
			borders.top + ( buttonHeight + padding ) * ( i + 1 ),
			borders.left + padding,
			borders.right - padding
		};
	}
	static RectI CalculateBorders( Vei2 center,int nItems,int buttonWidth,int buttonHeight,int padding )
	{
		const int halfHeight = ( ( buttonHeight + padding ) * nItems + padding ) / 2;
		const int halfWidth = ( buttonWidth / 2 ) + padding;
		return {
			center.y - halfHeight,
			center.y + halfHeight,
			center.x - halfWidth,
			center.x + halfWidth
		};
	}
private:
	bool finalized = false;
	std::vector<std::unique_ptr<Item>>::iterator pos;
	std::vector<std::unique_ptr<Item>> items;
	const RectI borders;
	const Color backColor;
	const int textPadding = 0;
	const int padding;
	const int buttonHeight;
	const Font buttonFont;
	const Color textColor;
	const Color selectedColor;
	const Color deselectedColor;
};