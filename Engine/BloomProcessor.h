#pragma once
#include "Surface.h"
#include "ChiliMath.h"

class BloomProcessor
{
public:
	BloomProcessor( Surface& input )
		:
		input( input ),
		hBuffer( input.GetWidth() / 4,input.GetHeight() / 4 ),
		vBuffer( input.GetWidth() / 4,input.GetHeight() / 4 )
	{
		float kernelFloat[diameter];
		for( int x = 0; x < diameter; x++ )
		{
			kernelFloat[x] = gaussian( std::fabs( float( x - int( GetKernelCenter() ) ) ),
				float( diameter / 6.0f ) );
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
			for( size_t x = 0u; x < width - diameter + 1; x++ )
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
	void VerticalPass()
	{
		const size_t centerKernel = GetKernelCenter();
		const size_t width = vBuffer.GetWidth();
		const size_t height = vBuffer.GetHeight();
		const size_t fringe = diameter / 2u;

		for( size_t x = fringe; x < width - fringe; x++ )
		{
			for( size_t y = 0u; y < height - diameter + 1; y++ )
			{
				unsigned int r = 0;
				unsigned int g = 0;
				unsigned int b = 0;
				const Color* pBuffer = &vBuffer.GetBufferConst()[y * width + x];
				for( size_t i = 0; i < diameter; i++,
					pBuffer += width )
				{
					const Color c = *pBuffer;
					const unsigned int coef = kernel[i];
					r += c.r * coef;
					g += c.g * coef;
					b += c.b * coef;
				}
				hBuffer.GetBuffer()[( y + centerKernel ) * width + x] =
				{
					unsigned char( r / sumKernel ),
					unsigned char( g / sumKernel ),
					unsigned char( b / sumKernel )
				};
			}
		}
	}
	void UpsizeBlendPass()
	{
		Color* const pOutputBuffer = input.GetBuffer();
		const Color* const pInputBuffer = hBuffer.GetBufferConst();
		const size_t inFringe = diameter / 2u;
		const size_t inWidth = hBuffer.GetWidth();
		const size_t inHeight = hBuffer.GetHeight();
		const size_t inTopLeft = ( inWidth + 1u ) * inFringe;
		const size_t outFringe = GetFringeSize();
		const size_t outWidth = input.GetWidth();
		const size_t outRight = outWidth - outFringe;
		const size_t outBottom = input.GetHeight() - outFringe;
		const size_t outTopLeft = ( outWidth + 1u ) * outFringe;

		auto AddSaturate = []( Color* pOut,unsigned int inr,unsigned int ing,unsigned int inb )
		{
			*pOut = { 
				unsigned char( min( inr + pOut->r,255u ) ),
				unsigned char( min( ing + pOut->g,255u ) ),
				unsigned char( min( inb + pOut->b,255u ) )
			};
		};

		// top two rows
		{
			// top left block
			{
				const unsigned int r = pInputBuffer[inTopLeft].r;
				const unsigned int g = pInputBuffer[inTopLeft].g;
				const unsigned int b = pInputBuffer[inTopLeft].b;
				AddSaturate( &pOutputBuffer[outTopLeft],r,g,b );
				AddSaturate( &pOutputBuffer[outTopLeft + 1],r,g,b );
				AddSaturate( &pOutputBuffer[outTopLeft + outWidth],r,g,b );
				AddSaturate( &pOutputBuffer[outTopLeft + outWidth + 1],r,g,b );
			}
		}

		for( size_t y = outFringe + 2u; y < outBottom - 2u; y += 4u )
		{
			const size_t baseY = ( y - 2u ) / 4u;
			for( size_t x = outFringe + 2u; x < outRight - 2u; x += 4u )
			{
				const size_t baseX = ( x - 2u ) / 4u;
				const Color p0 = pInputBuffer[baseY * inWidth + baseX];
				const Color p1 = pInputBuffer[baseY * inWidth + baseX + 1u];
				const Color p2 = pInputBuffer[( baseY + 1u ) * inWidth + baseX];
				const Color p3 = pInputBuffer[( baseY + 1u ) * inWidth + baseX + 1u];
				const Color d0 = pOutputBuffer[y * outWidth + x];
				const Color d1 = pOutputBuffer[y * outWidth + x + 1u];
				const Color d2 = pOutputBuffer[y * outWidth + x + 2u];
				const Color d3 = pOutputBuffer[y * outWidth + x + 3u];
				const Color d4 = pOutputBuffer[( y + 1u ) * outWidth + x];
				const Color d5 = pOutputBuffer[( y + 1u ) * outWidth + x + 1u];
				const Color d6 = pOutputBuffer[( y + 1u ) * outWidth + x + 2u];
				const Color d7 = pOutputBuffer[( y + 1u ) * outWidth + x + 3u];
				const Color d8 = pOutputBuffer[( y + 2u ) * outWidth + x];
				const Color d9 = pOutputBuffer[( y + 2u ) * outWidth + x + 1u];
				const Color d10 = pOutputBuffer[( y + 2u ) * outWidth + x + 2u];
				const Color d11 = pOutputBuffer[( y + 2u ) * outWidth + x + 3u];
				const Color d12 = pOutputBuffer[( y + 3u ) * outWidth + x];
				const Color d13 = pOutputBuffer[( y + 3u ) * outWidth + x + 1u];
				const Color d14 = pOutputBuffer[( y + 3u ) * outWidth + x + 2u];
				const Color d15 = pOutputBuffer[( y + 3u ) * outWidth + x + 3u];


				unsigned int lr1 = p0.r * 224u + p2.r * 32u;
				unsigned int lg1 = p0.g * 224u + p2.g * 32u;
				unsigned int lb1 = p0.b * 224u + p2.b * 32u;
				unsigned int rr1 = p1.r * 224u + p3.r * 32u;
				unsigned int rg1 = p1.g * 224u + p3.g * 32u;
				unsigned int rb1 = p1.b * 224u + p3.b * 32u;

				pOutputBuffer[y * outWidth + x] =
				{ unsigned char( min( ( lr1 * 224u + rr1 * 32u ) / 65536u + d0.r,255u ) ),
				unsigned char( min( ( lg1 * 224u + rg1 * 32u ) / 65536u + d0.g,255u ) ),
				unsigned char( min( ( lb1 * 224u + rb1 * 32u ) / 65536u + d0.b,255u ) ) };

				pOutputBuffer[y * outWidth + x + 1u] =
				{ unsigned char( min( ( lr1 * 160u + rr1 * 96u ) / 65536u + d1.r,255u ) ),
				unsigned char( min( ( lg1 * 160u + rg1 * 96u ) / 65536u + d1.g,255u ) ),
				unsigned char( min( ( lb1 * 160u + rb1 * 96u ) / 65536u + d1.b,255u ) ) };

				pOutputBuffer[y * outWidth + x + 2u] =
				{ unsigned char( min( ( lr1 * 96u + rr1 * 160u ) / 65536u + d2.r,255u ) ),
				unsigned char( min( ( lg1 * 96u + rg1 * 160u ) / 65536u + d2.g,255u ) ),
				unsigned char( min( ( lb1 * 96u + rb1 * 160u ) / 65536u + d2.b,255u ) ) };

				pOutputBuffer[y * outWidth + x + 3u] =
				{ unsigned char( min( ( lr1 * 32u + rr1 * 224u ) / 65536u + d3.r,255u ) ),
				unsigned char( min( ( lg1 * 32u + rg1 * 224u ) / 65536u + d3.g,255u ) ),
				unsigned char( min( ( lb1 * 32u + rb1 * 224u ) / 65536u + d3.b,255u ) ) };

				lr1 = p0.r * 160u + p2.r * 96u;
				lg1 = p0.g * 160u + p2.g * 96u;
				lb1 = p0.b * 160u + p2.b * 96u;
				rr1 = p1.r * 160u + p3.r * 96u;
				rg1 = p1.g * 160u + p3.g * 96u;
				rb1 = p1.b * 160u + p3.b * 96u;

				pOutputBuffer[( y + 1u ) * outWidth + x] =
				{ unsigned char( min( ( lr1 * 224u + rr1 * 32u ) / 65536u + d4.r,255u ) ),
				unsigned char( min( ( lg1 * 224u + rg1 * 32u ) / 65536u + d4.g,255u ) ),
				unsigned char( min( ( lb1 * 224u + rb1 * 32u ) / 65536u + d4.b,255u ) ) };

				pOutputBuffer[( y + 1u ) * outWidth + x + 1u] =
				{ unsigned char( min( ( lr1 * 160u + rr1 * 96u ) / 65536u + d5.r,255u ) ),
				unsigned char( min( ( lg1 * 160u + rg1 * 96u ) / 65536u + d5.g,255u ) ),
				unsigned char( min( ( lb1 * 160u + rb1 * 96u ) / 65536u + d5.b,255u ) ) };

				pOutputBuffer[( y + 1u ) * outWidth + x + 2u] =
				{ unsigned char( min( ( lr1 * 96u + rr1 * 160u ) / 65536u + d6.r,255u ) ),
				unsigned char( min( ( lg1 * 96u + rg1 * 160u ) / 65536u + d6.g,255u ) ),
				unsigned char( min( ( lb1 * 96u + rb1 * 160u ) / 65536u + d6.b,255u ) ) };

				pOutputBuffer[( y + 1u ) * outWidth + x + 3u] =
				{ unsigned char( min( ( lr1 * 32u + rr1 * 224u ) / 65536u + d7.r,255u ) ),
				unsigned char( min( ( lg1 * 32u + rg1 * 224u ) / 65536u + d7.g,255u ) ),
				unsigned char( min( ( lb1 * 32u + rb1 * 224u ) / 65536u + d7.b,255u ) ) };

				lr1 = p0.r * 96u + p2.r * 160u;
				lg1 = p0.g * 96u + p2.g * 160u;
				lb1 = p0.b * 96u + p2.b * 160u;
				rr1 = p1.r * 96u + p3.r * 160u;
				rg1 = p1.g * 96u + p3.g * 160u;
				rb1 = p1.b * 96u + p3.b * 160u;

				pOutputBuffer[( y + 2u ) * outWidth + x] =
				{ unsigned char( min( ( lr1 * 224u + rr1 * 32u ) / 65536u + d8.r,255u ) ),
				unsigned char( min( ( lg1 * 224u + rg1 * 32u ) / 65536u + d8.g,255u ) ),
				unsigned char( min( ( lb1 * 224u + rb1 * 32u ) / 65536u + d8.b,255u ) ) };

				pOutputBuffer[( y + 2u ) * outWidth + x + 1u] =
				{ unsigned char( min( ( lr1 * 160u + rr1 * 96u ) / 65536u + d9.r,255u ) ),
				unsigned char( min( ( lg1 * 160u + rg1 * 96u ) / 65536u + d9.g,255u ) ),
				unsigned char( min( ( lb1 * 160u + rb1 * 96u ) / 65536u + d9.b,255u ) ) };

				pOutputBuffer[( y + 2u ) * outWidth + x + 2u] =
				{ unsigned char( min( ( lr1 * 96u + rr1 * 160u ) / 65536u + d10.r,255u ) ),
				unsigned char( min( ( lg1 * 96u + rg1 * 160u ) / 65536u + d10.g,255u ) ),
				unsigned char( min( ( lb1 * 96u + rb1 * 160u ) / 65536u + d10.b,255u ) ) };

				pOutputBuffer[( y + 2u ) * outWidth + x + 3u] =
				{ unsigned char( min( ( lr1 * 32u + rr1 * 224u ) / 65536u + d11.r,255u ) ),
				unsigned char( min( ( lg1 * 32u + rg1 * 224u ) / 65536u + d11.g,255u ) ),
				unsigned char( min( ( lb1 * 32u + rb1 * 224u ) / 65536u + d11.b,255u ) ) };

				lr1 = p0.r * 32u + p2.r * 224u;
				lg1 = p0.g * 32u + p2.g * 224u;
				lb1 = p0.b * 32u + p2.b * 224u;
				rr1 = p1.r * 32u + p3.r * 224u;
				rg1 = p1.g * 32u + p3.g * 224u;
				rb1 = p1.b * 32u + p3.b * 224u;

				pOutputBuffer[( y + 3u ) * outWidth + x] =
				{ unsigned char( min( ( lr1 * 224u + rr1 * 32u ) / 65536u + d12.r,255u ) ),
				unsigned char( min( ( lg1 * 224u + rg1 * 32u ) / 65536u + d12.g,255u ) ),
				unsigned char( min( ( lb1 * 224u + rb1 * 32u ) / 65536u + d12.b,255u ) ) };

				pOutputBuffer[( y + 3u ) * outWidth + x + 1u] =
				{ unsigned char( min( ( lr1 * 160u + rr1 * 96u ) / 65536u + d13.r,255u ) ),
				unsigned char( min( ( lg1 * 160u + rg1 * 96u ) / 65536u + d13.g,255u ) ),
				unsigned char( min( ( lb1 * 160u + rb1 * 96u ) / 65536u + d13.b,255u ) ) };

				pOutputBuffer[( y + 3u ) * outWidth + x + 2u] =
				{ unsigned char( min( ( lr1 * 96u + rr1 * 160u ) / 65536u + d14.r,255u ) ),
				unsigned char( min( ( lg1 * 96u + rg1 * 160u ) / 65536u + d14.g,255u ) ),
				unsigned char( min( ( lb1 * 96u + rb1 * 160u ) / 65536u + d14.b,255u ) ) };

				pOutputBuffer[( y + 3u ) * outWidth + x + 3u] =
				{ unsigned char( min( ( lr1 * 32u + rr1 * 224u ) / 65536u + d15.r,255u ) ),
				unsigned char( min( ( lg1 * 32u + rg1 * 224u ) / 65536u + d15.g,255u ) ),
				unsigned char( min( ( lb1 * 32u + rb1 * 224u ) / 65536u + d15.b,255u ) ) };
			}
		}
	}
	void Go()
	{
		DownsizePass();
		if( count % 100 == 0 )
		{
			input.Save( L"frame_a_init" + std::to_wstring( count ) + L".bmp" );
			hBuffer.Save( L"frame_b_down" + std::to_wstring( count ) + L".bmp" );
		}
		HorizontalPass();
		VerticalPass();
		if( count % 100 == 0 )
		{
			vBuffer.Save( L"frame_c_horz" + std::to_wstring( count ) + L".bmp" );
			hBuffer.Save( L"frame_d_vert" + std::to_wstring( count ) + L".bmp" );
		}
		UpsizeBlendPass();
		if( count % 100 == 0 )
		{
			input.Save( L"frame_e_finl" + std::to_wstring( count ) + L".bmp" );
		}
		count++;
	}
	static unsigned int GetFringeSize()
	{
		return (diameter / 2u) * 4u;
	}
private:
	static unsigned int GetKernelCenter()
	{
		return ( diameter - 1 ) / 2;
	}
private:
	unsigned int count = 0u;
	static const unsigned int diameter = 16u;
	unsigned char kernel[diameter];
	unsigned int sumKernel = 0u;
	Surface& input;
	Surface hBuffer;
	Surface vBuffer;
};