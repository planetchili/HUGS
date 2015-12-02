#pragma once

#include "Colors.h"
#include "Font.h"
#include <gdiplus.h>
#include <string>
#include <assert.h>
#pragma comment( lib,"gdiplus.lib" )


class Surface
{
public:
	// pitch is in PIXELS (4-byte units)
	Surface( unsigned int width,unsigned int height,unsigned int pitch )
		:
		buffer( (Color* const)_aligned_malloc( size_t( height * pitch * sizeof( Color ) ),16u ) ),
		width( width ),
		height( height ),
		pitch( pitch )
	{}
	Surface( unsigned int width,unsigned int height ) // 16-byte alignment
		:
		Surface( width,height,GetPitch( width,16 ) )
	{}
	Surface( Surface&& source )
		:
		buffer( source.buffer ),
		width( source.width ),
		height( source.height ),
		pitch( source.pitch )
	{
		const_cast<Color*>( source.buffer ) = nullptr;
	}
	Surface( Surface& ) = delete;
	Surface& operator=( Surface&& donor )
	{
		width = donor.width;
		height = donor.height;
		pitch = donor.pitch;
		const_cast<Color*>( buffer ) = donor.buffer;
		const_cast<Color*>( donor.buffer ) = nullptr;
		return *this;
	}
	Surface& operator=( const Surface& ) = delete;
	~Surface()
	{
		if( buffer != nullptr )
		{
			_aligned_free( buffer );
			const_cast<Color*>( buffer ) = nullptr;
		}
	}
	inline void Clear( Color fillValue  )
	{
		memset( buffer,fillValue,pitch * height * sizeof( Color ) );
	}
	inline void Present( const size_t backBufferPitch,BYTE* const backBuffer ) const
	{
		const size_t rowWidthBytes = sizeof( Color ) * width;
		for( unsigned int y = 0; y < height; y++ )
		{
			memcpy( &backBuffer[backBufferPitch * y],&buffer[pitch * y],rowWidthBytes );
		}
	}
	inline void PresentRegion( const RectI& region,const size_t backBufferPitch,
		BYTE* const backBuffer ) const
	{
		const size_t rowWidthBytes = sizeof( Color ) * ( region.right - region.left );
		for( unsigned int y = unsigned int( region.top ); y < unsigned int( region.bottom ); y++ )
		{
			memcpy( &backBuffer[backBufferPitch * ( y - region.top )],
				&buffer[pitch * y + region.left],rowWidthBytes );
		}
	}
	inline void PutPixel( unsigned int x,unsigned int y,Color c )
	{
		assert( x >= 0 );
		assert( y >= 0 );
		assert( x < width );
		assert( y < height );
		buffer[y * pitch + x] = c;
	}
	inline void PutPixelAlpha( unsigned int x,unsigned int y,Color c )
	{
		assert( x >= 0 );
		assert( y >= 0 );
		assert( x < width );
		assert( y < height );
		// load source pixel
		const Color d = GetPixel( x,y );

		// blend channels
		const unsigned char rsltRed = ( c.r * c.x + d.r * ( 255 - c.x ) ) / 255;
		const unsigned char rsltGreen = ( c.g * c.x + d.g * ( 255 - c.x ) ) / 255;
		const unsigned char rsltBlue = ( c.b * c.x + d.b * ( 255 - c.x ) ) / 255;

		// pack channels back into pixel and fire pixel onto surface
		PutPixel( x,y,{ rsltRed,rsltGreen,rsltBlue } );
	}
	inline Color GetPixel( unsigned int x,unsigned int y ) const
	{
		assert( x >= 0 );
		assert( y >= 0 );
		assert( x < width );
		assert( y < height );
		return buffer[y * pitch + x];
	}
	inline unsigned int GetWidth() const
	{
		return width;
	}
	inline unsigned int GetHeight() const
	{
		return height;
	}
	inline unsigned int GetPitch() const
	{
		return pitch;
	}
	inline Color* const GetBuffer()
	{
		return buffer;
	}
	inline const Color* const GetBufferConst() const
	{
		return buffer;
	}
	static Surface FromFile( const std::wstring& name )
	{
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int pitch = 0;
		Color* buffer = nullptr;

		{
			Gdiplus::Bitmap bitmap( name.c_str() );
			pitch = width = bitmap.GetWidth();
			height = bitmap.GetHeight();
			buffer = new Color[pitch * height];

			for( unsigned int y = 0; y < height; y++ )
			{
				for( unsigned int x = 0; x < width; x++ )
				{
					Gdiplus::Color c;
					bitmap.GetPixel( x,y,&c );
					buffer[y * pitch + x] = c.GetValue();
				}
			}
		}

		return Surface( width,height,pitch,buffer );
	}
	void Save( const std::wstring& filename ) const
	{
		auto GetEncoderClsid = []( const WCHAR* format,CLSID* pClsid ) -> int
		{
			UINT  num = 0;          // number of image encoders
			UINT  size = 0;         // size of the image encoder array in bytes

			Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

			Gdiplus::GetImageEncodersSize( &num,&size );
			if( size == 0 )
				return -1;  // Failure

			pImageCodecInfo = ( Gdiplus::ImageCodecInfo* )( malloc( size ) );
			if( pImageCodecInfo == NULL )
				return -1;  // Failure

			GetImageEncoders( num,size,pImageCodecInfo );

			for( UINT j = 0; j < num; ++j )
			{
				if( wcscmp( pImageCodecInfo[j].MimeType,format ) == 0 )
				{
					*pClsid = pImageCodecInfo[j].Clsid;
					free( pImageCodecInfo );
					return j;  // Success
				}
			}

			free( pImageCodecInfo );
			return -1;  // Failure
		};

		{
			Gdiplus::Bitmap bitmap( width,height,pitch * sizeof( Color ),PixelFormat32bppARGB,(BYTE*)buffer );
			CLSID bmpID;
			GetEncoderClsid( L"image/bmp",&bmpID );
			bitmap.Save( filename.c_str(),&bmpID,NULL );
		}
	}
	void Copy( const Surface& src )
	{
		assert( width == src.width );
		assert( height == src.height );
		if( pitch == src.pitch )
		{
			memcpy( buffer,src.buffer,pitch * height * sizeof( Color ) );
		}
		else
		{
			for( unsigned int y = 0; y < height; y++ )
			{
				memcpy( &buffer[pitch * y],&( src.buffer )[pitch * y],sizeof( Color )* width );
			}
		}
	}
private:
	static unsigned int GetPitch( unsigned int width,unsigned int byteAlignment )
	{
		assert( byteAlignment % 4 == 0 );
		const unsigned int pixelAlignment = byteAlignment / sizeof( Color );
		return width + ( pixelAlignment - width % pixelAlignment ) % pixelAlignment;
	}
	Surface( unsigned int width,unsigned int height,unsigned int pitch,Color* const buffer )
		:
		width( width ),
		height( height ),
		buffer( buffer ),
		pitch( pitch )
	{}
protected:
	Color* const buffer;
	unsigned int width;
	unsigned int height;
	// this pitch value is the pitch in PIXELS (I know, kinda dumb but...)
	unsigned int pitch;
};

class TextSurface : public Surface
{
public:
	TextSurface( unsigned int width,unsigned int height )
		:
		Surface( width,height ),
		bitmap( width,height,pitch * sizeof( Color ),
		PixelFormat32bppRGB,(byte*)buffer ),
		g( &bitmap )
	{
		g.SetTextRenderingHint( Gdiplus::TextRenderingHintAntiAlias );
	}
	void DrawString( const std::wstring& string,Vec2 pt,const Font& font,Color c )
	{
		Gdiplus::Color textColor( c.r,c.g,c.b );
		Gdiplus::SolidBrush textBrush( textColor );
		g.DrawString( string.c_str(),-1,font,
			Gdiplus::PointF( pt.x,pt.y ),&textBrush );
	}
	void DrawString( const std::wstring& string,const RectF& rect,const Font& font,
		Color c = WHITE,Font::Alignment a = Font::Center )
	{
		Gdiplus::StringFormat format;
		switch( a )
		{
		case Font::Left:
			format.SetAlignment( Gdiplus::StringAlignmentNear );
			break;
		case Font::Right:
			format.SetAlignment( Gdiplus::StringAlignmentFar );
			break;
		case Font::Center:
		default:
			format.SetAlignment( Gdiplus::StringAlignmentCenter );
			break;
		}
		Gdiplus::Color textColor( c.r,c.g,c.b );
		Gdiplus::SolidBrush textBrush( textColor );
		g.DrawString( string.c_str(),-1,font,
			Gdiplus::RectF( rect.left,rect.top,rect.GetWidth(),rect.GetHeight() ),
			&format,
			&textBrush );
	}
	TextSurface( const TextSurface& ) = delete;
	TextSurface( TextSurface&& ) = delete;
	TextSurface& operator=( const TextSurface& ) = delete;
	TextSurface& operator=( TextSurface&& ) = delete;
private:
	Gdiplus::Bitmap	bitmap;
	Gdiplus::Graphics g;
};