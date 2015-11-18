#pragma once
#include "Surface.h"
#include "ChiliMath.h"

class BloomProcessor
{
public:
	BloomProcessor( const Surface& input )
		:
		input( input ),
		hBuffer( input.GetWidth() / 4,input.GetHeight() / 4 ),
		vBuffer( input.GetWidth() / 4,input.GetHeight() / 4 )
	{
		float kernelFloat[diameter];
		for( int x = 0; x < diameter; x++ )
		{
			kernelFloat[x] = gaussian( std::fabs( float( x - GetKernelCenter() ) ),float( diameter / 6.0f ) );
		}
		for( int x = 0; x < diameter; x++ )
		{
			kernel[x] = unsigned char( 255 * ( kernelFloat[x]
				/ kernelFloat[GetKernelCenter()] ) );
		}
		for( int x = 0; x < diameter; x++ )
		{
			sumKernel += kernel[x];
		}
		hBuffer.Clear( BLACK );
		vBuffer.Clear( BLACK );
	}
	void DownsizePass()
	{
		const Color* const pInputBuffer = input.GetBufferConst();
		Color* const pOutputBuffer = hBuffer.GetBuffer();
		const size_t inWidth = input.GetWidth();
		const size_t inHeight = input.GetHeight();
		const size_t outWidth = hBuffer.GetWidth();
		const size_t outHeight = hBuffer.GetHeight();

		for( size_t y = 0; y < outHeight; y++ )
		{
			for( size_t x = 0; x < outWidth; x++ )
			{
				const Color p0  = pInputBuffer[( y * 4 )     * inWidth + x * 4];
				const Color p1  = pInputBuffer[( y * 4 )     * inWidth + x * 4 + 1];
				const Color p2  = pInputBuffer[( y * 4 )     * inWidth + x * 4 + 2];
				const Color p3  = pInputBuffer[( y * 4 )     * inWidth + x * 4 + 3];
				const Color p4  = pInputBuffer[( y * 4 + 1 ) * inWidth + x * 4];
				const Color p5  = pInputBuffer[( y * 4 + 1 ) * inWidth + x * 4 + 1];
				const Color p6  = pInputBuffer[( y * 4 + 1 ) * inWidth + x * 4 + 2];
				const Color p7  = pInputBuffer[( y * 4 + 1 ) * inWidth + x * 4 + 3];
				const Color p8  = pInputBuffer[( y * 4 + 2 ) * inWidth + x * 4];
				const Color p9  = pInputBuffer[( y * 4 + 2 ) * inWidth + x * 4 + 1];
				const Color p10 = pInputBuffer[( y * 4 + 2 ) * inWidth + x * 4 + 2];
				const Color p11 = pInputBuffer[( y * 4 + 2 ) * inWidth + x * 4 + 3];
				const Color p12 = pInputBuffer[( y * 4 + 3 ) * inWidth + x * 4];
				const Color p13 = pInputBuffer[( y * 4 + 3 ) * inWidth + x * 4 + 1];
				const Color p14 = pInputBuffer[( y * 4 + 3 ) * inWidth + x * 4 + 2];
				const Color p15 = pInputBuffer[( y * 4 + 3 ) * inWidth + x * 4 + 3];
				const unsigned int x0 = p0.x;
				const unsigned int x1 = p1.x;
				const unsigned int x2 = p2.x;
				const unsigned int x3 = p3.x;
				const unsigned int x4 = p0.x;
				const unsigned int x5 = p5.x;
				const unsigned int x6 = p6.x;
				const unsigned int x7 = p7.x;
				const unsigned int x8 = p8.x;
				const unsigned int x9 = p9.x;
				const unsigned int x10 = p10.x;
				const unsigned int x11 = p11.x;
				const unsigned int x12 = p12.x;
				const unsigned int x13 = p13.x;
				const unsigned int x14 = p14.x;
				const unsigned int x15 = p15.x;
				pOutputBuffer[y * outWidth + x] =
				{ unsigned char( ( p0.r * x0 + p1.r * x1 + p2.r * x2 + p3.r * x3 + p4.r * x4 + p5.r * x5 + p6.r * x6 + p7.r * x7 + p8.r * x8 + p9.r * x9 + p10.r * x10 + p11.r * x11 + p12.r * x12 + p13.r * x13 + p14.r * x14 + p15.r * x15 ) / ( 16 * 255 ) ),
				  unsigned char( ( p0.g * x0 + p1.g * x1 + p2.g * x2 + p3.g * x3 + p4.g * x4 + p5.g * x5 + p6.g * x6 + p7.g * x7 + p8.g * x8 + p9.g * x9 + p10.g * x10 + p11.g * x11 + p12.g * x12 + p13.g * x13 + p14.g * x14 + p15.g * x15 ) / ( 16 * 255 ) ),
				  unsigned char( ( p0.b * x0 + p1.b * x1 + p2.b * x2 + p3.b * x3 + p4.b * x4 + p5.b * x5 + p6.b * x6 + p7.b * x7 + p8.b * x8 + p9.b * x9 + p10.b * x10 + p11.b * x11 + p12.b * x12 + p13.b * x13 + p14.b * x14 + p15.b * x15 ) / ( 16 * 255 ) ) };
			}
		}
	}
	void HorizontalPass()
	{
		const size_t centerKernel = GetKernelCenter();
		const size_t width = hBuffer.GetWidth();
		const size_t height = hBuffer.GetHeight();

		for( size_t y = 0u; y < height; y++ )
		{
			for( size_t x = 0u; x < width - diameter; x++ )
			{
				unsigned int r = 0;
				unsigned int g = 0;
				unsigned int b = 0;
				const Color* const pBuffer = &hBuffer.GetBufferConst()[y * width + x];
				for( size_t i = 0; i < diameter; i++ )
				{
					const Color c = pBuffer[i];
					const unsigned int coef = kernel[i];
					r += c.r * coef;
					g += c.g * coef;
					b += c.b * coef;
				}
				vBuffer.GetBuffer()[y * width + x + centerKernel] =
				{
					unsigned char( r / sumKernel ),
					unsigned char( g / sumKernel ),
					unsigned char( b / sumKernel )
				};
			}
		}
	}
	void Go()
	{
		DownsizePass();
		HorizontalPass();
		if( ++count % 20 == 0 )
		{
			hBuffer.Save( L"h_frame" + std::to_wstring( count ) + L".bmp" );
			vBuffer.Save( L"v_frame" + std::to_wstring( count ) + L".bmp" );
		}
	}
private:
	static unsigned int GetKernelCenter()
	{
		return ( diameter - 1 ) / 2;
	}
private:
	unsigned int count = 1u;
	static const unsigned int diameter = 16u;
	unsigned char kernel[diameter];
	unsigned int sumKernel = 0u;
	const Surface& input;
	Surface hBuffer;
	Surface vBuffer;
};