#pragma once

#include "Colors.h"
#include <gdiplus.h>
#include <string>
#include <assert.h>
#pragma comment( lib,"gdiplus.lib" )


class Surface
{
public:
	Surface( unsigned int width,unsigned int height,unsigned int pitch )
		:
		buffer( new Color[ pitch * height ] ),
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
	~Surface()
	{
		if( buffer != nullptr )
		{
			delete[] buffer;
		}
	}
	void Save( const std::wstring filename )
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

		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		Gdiplus::GdiplusStartup( &gdiplusToken,&gdiplusStartupInput,NULL );

		{
			Gdiplus::Bitmap bitmap( width,height,pitch * sizeof( Color ),PixelFormat32bppARGB,(BYTE*)buffer );
			CLSID bmpID;
			GetEncoderClsid( L"image/bmp",&bmpID );
			bitmap.Save( filename.c_str(),&bmpID,NULL );
		}

		Gdiplus::GdiplusShutdown( gdiplusToken );
	}
	inline void Clear( Color fillValue  )
	{
		memset( buffer,fillValue,pitch * height * sizeof( Color ) );
	}
	inline void Present( const unsigned int pitch,BYTE* const buffer ) const
	{
		for( unsigned int y = 0; y < height; y++ )
		{
			memcpy( &buffer[pitch * y],&(this->buffer)[this->pitch * y],sizeof(Color)* width );
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
	static Surface FromFile( const std::wstring name )
	{
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		Gdiplus::GdiplusStartup( &gdiplusToken,&gdiplusStartupInput,NULL );

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

		Gdiplus::GdiplusShutdown( gdiplusToken );
		return Surface( width,height,pitch,buffer );
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
private:
	Color* const buffer;
	const unsigned int width;
	const unsigned int height;
	const unsigned int pitch;
};