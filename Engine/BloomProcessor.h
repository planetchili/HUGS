#pragma once
#include "Surface.h"
#include "ChiliMath.h"
#include <immintrin.h>
#include "FrameTimer.h"
#include <fstream>
#include <functional>
#include "Cpuid.h"

#define BLOOM_PROCESSOR_USE_SSE true

class BloomProcessor
{
public:
	BloomProcessor( Surface& input )
		:
		input( input ),
		hBuffer( input.GetWidth() / 4,input.GetHeight() / 4 ),
		vBuffer( input.GetWidth() / 4,input.GetHeight() / 4 ),
		log( L"bloomlog.txt" )
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
		hBuffer.Clear( BLACK );
		vBuffer.Clear( BLACK );

		// setup function pointers
		if( BLOOM_PROCESSOR_USE_SSE )
		{
			if( InstructionSet::SSSE3() )
			{
				// std::mem_fn only needed as a workaround for MSVC bug :/
				DownsizePassFunc = std::mem_fn( &BloomProcessor::_DownsizePassSSSE3 );
				HorizontalPassFunc = std::mem_fn( &BloomProcessor::_HorizontalPassSSSE3 );
			}
			else
			{
				DownsizePassFunc = std::mem_fn( &BloomProcessor::_DownsizePassSSE2 );
				HorizontalPassFunc = std::mem_fn( &BloomProcessor::_HorizontalPassSSE2 );
			}
		}
		else
		{
			DownsizePassFunc = std::mem_fn( &BloomProcessor::_DownsizePassX86 );
			HorizontalPassFunc = std::mem_fn( &BloomProcessor::_HorizontalPassX86 );
		}
	}
	void DownsizePass()
	{
		DownsizePassFunc( this );
	}
	void HorizontalPass()
	{
		_HorizontalPassSSSE3();
		//HorizontalPassFunc( this );
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
					unsigned char( min( r / divisorKernel,255u ) ),
					unsigned char( min( g / divisorKernel,255u ) ),
					unsigned char( min( b / divisorKernel,255u ) )
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
		const size_t inBottom = inHeight - inFringe;
		const size_t inTopLeft = ( inWidth + 1u ) * inFringe;
		const size_t inTopRight = inWidth * ( inFringe + 1u ) - inFringe - 1u;
		const size_t inBottomLeft = inWidth * ( inBottom - 1u ) + inFringe;
		const size_t inBottomRight = inWidth * inBottom - inFringe - 1u;
		const size_t outFringe = GetFringeSize();
		const size_t outWidth = input.GetWidth();
		const size_t outRight = outWidth - outFringe;
		const size_t outBottom = input.GetHeight() - outFringe;
		const size_t outTopLeft = ( outWidth + 1u ) * outFringe;
		const size_t outTopRight = outWidth * ( outFringe + 1u ) - outFringe - 1u;
		const size_t outBottomLeft = outWidth * ( outBottom - 1u ) + outFringe;
		const size_t outBottomRight = outWidth * outBottom - outFringe - 1u;

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
				AddSaturate( &pOutputBuffer[outTopLeft + 1u],r,g,b );
				AddSaturate( &pOutputBuffer[outTopLeft + outWidth],r,g,b );
				AddSaturate( &pOutputBuffer[outTopLeft + outWidth + 1u],r,g,b );
			}

			// center
			{
				Color* const pOutUpper = &pOutputBuffer[outFringe * outWidth];
				Color* const pOutLower = &pOutputBuffer[( outFringe + 1u ) * outWidth];
				const Color* const pIn = &pInputBuffer[inFringe * inWidth];
				for( size_t x = outFringe + 2u; x < outRight - 2u; x += 4u )
				{
					const size_t baseX = ( x - 2u ) / 4u;
					const unsigned int r0 = pIn[baseX].r;
					const unsigned int g0 = pIn[baseX].g;
					const unsigned int b0 = pIn[baseX].b;
					const unsigned int r1 = pIn[baseX + 1u].r;
					const unsigned int g1 = pIn[baseX + 1u].g;
					const unsigned int b1 = pIn[baseX + 1u].b;
					{
						const unsigned int r = ( r0 * 224u + r1 * 32u ) / 256u;
						const unsigned int g = ( g0 * 224u + g1 * 32u ) / 256u;
						const unsigned int b = ( b0 * 224u + b1 * 32u ) / 256u;
						AddSaturate( &pOutUpper[x],r,g,b );
						AddSaturate( &pOutLower[x],r,g,b );
					}
					{
						const unsigned int r = ( r0 * 160u + r1 * 96u ) / 256u;
						const unsigned int g = ( g0 * 160u + g1 * 96u ) / 256u;
						const unsigned int b = ( b0 * 160u + b1 * 96u ) / 256u;
						AddSaturate( &pOutUpper[x + 1u],r,g,b );
						AddSaturate( &pOutLower[x + 1u],r,g,b );
					}
					{
						const unsigned int r = ( r0 * 96u + r1 * 160u ) / 256u;
						const unsigned int g = ( g0 * 96u + g1 * 160u ) / 256u;
						const unsigned int b = ( b0 * 96u + b1 * 160u ) / 256u;
						AddSaturate( &pOutUpper[x + 2u],r,g,b );
						AddSaturate( &pOutLower[x + 2u],r,g,b );
					}
					{
						const unsigned int r = ( r0 * 32u + r1 * 224u ) / 256u;
						const unsigned int g = ( g0 * 32u + g1 * 224u ) / 256u;
						const unsigned int b = ( b0 * 32u + b1 * 224u ) / 256u;
						AddSaturate( &pOutUpper[x + 3u],r,g,b );
						AddSaturate( &pOutLower[x + 3u],r,g,b );
					}
				}
			}

			// top right block
			{
				const unsigned int r = pInputBuffer[inTopRight].r;
				const unsigned int g = pInputBuffer[inTopRight].g;
				const unsigned int b = pInputBuffer[inTopRight].b;
				AddSaturate( &pOutputBuffer[outTopRight - 1u],r,g,b );
				AddSaturate( &pOutputBuffer[outTopRight],r,g,b );
				AddSaturate( &pOutputBuffer[outTopRight + outWidth - 1u],r,g,b );
				AddSaturate( &pOutputBuffer[outTopRight + outWidth],r,g,b );
			}
		}

		// center rows
		for( size_t y = outFringe + 2u; y < outBottom - 2u; y += 4u )
		{
			const size_t baseY = ( y - 2u ) / 4u;

			// first two pixels
			{
				const unsigned int r0 = pInputBuffer[baseY * inWidth + inFringe].r;
				const unsigned int g0 = pInputBuffer[baseY * inWidth + inFringe].g;
				const unsigned int b0 = pInputBuffer[baseY * inWidth + inFringe].b;
				const unsigned int r1 = pInputBuffer[( baseY + 1u ) * inWidth + inFringe].r;
				const unsigned int g1 = pInputBuffer[( baseY + 1u ) * inWidth + inFringe].g;
				const unsigned int b1 = pInputBuffer[( baseY + 1u ) * inWidth + inFringe].b;
				{
					const unsigned int r = ( r0 * 224u + r1 * 32u ) / 256u;
					const unsigned int g = ( g0 * 224u + g1 * 32u ) / 256u;
					const unsigned int b = ( b0 * 224u + b1 * 32u ) / 256u;
					AddSaturate( &pOutputBuffer[y * outWidth + outFringe],r,g,b );
					AddSaturate( &pOutputBuffer[y * outWidth + outFringe + 1u],r,g,b );
				}
				{
					const unsigned int r = ( r0 * 160u + r1 * 96u ) / 256u;
					const unsigned int g = ( g0 * 160u + g1 * 96u ) / 256u;
					const unsigned int b = ( b0 * 160u + b1 * 96u ) / 256u;
					AddSaturate( &pOutputBuffer[( y + 1u ) * outWidth + outFringe],r,g,b );
					AddSaturate( &pOutputBuffer[( y + 1u ) * outWidth + outFringe + 1u],r,g,b );
				}
				{
					const unsigned int r = ( r0 * 96u + r1 * 160u ) / 256u;
					const unsigned int g = ( g0 * 96u + g1 * 160u ) / 256u;
					const unsigned int b = ( b0 * 96u + b1 * 160u ) / 256u;
					AddSaturate( &pOutputBuffer[( y + 2u ) * outWidth + outFringe],r,g,b );
					AddSaturate( &pOutputBuffer[( y + 2u ) * outWidth + outFringe + 1u],r,g,b );
				}
				{
					const unsigned int r = ( r0 * 32u + r1 * 224u ) / 256u;
					const unsigned int g = ( g0 * 32u + g1 * 224u ) / 256u;
					const unsigned int b = ( b0 * 32u + b1 * 224u ) / 256u;
					AddSaturate( &pOutputBuffer[( y + 3u ) * outWidth + outFringe],r,g,b );
					AddSaturate( &pOutputBuffer[( y + 3u ) * outWidth + outFringe + 1u],r,g,b );
				}
			}

			// center pixels
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

			// last two pixels
			{
				const unsigned int r0 = pInputBuffer[( baseY + 1u ) * inWidth - inFringe - 2u].r;
				const unsigned int g0 = pInputBuffer[( baseY + 1u ) * inWidth - inFringe - 2u].g;
				const unsigned int b0 = pInputBuffer[( baseY + 1u ) * inWidth - inFringe - 2u].b;
				const unsigned int r1 = pInputBuffer[( baseY + 2u ) * inWidth - inFringe - 1u].r;
				const unsigned int g1 = pInputBuffer[( baseY + 2u ) * inWidth - inFringe - 1u].g;
				const unsigned int b1 = pInputBuffer[( baseY + 2u ) * inWidth - inFringe - 1u].b;
				{
					const unsigned int r = ( r0 * 224u + r1 * 32u ) / 256u;
					const unsigned int g = ( g0 * 224u + g1 * 32u ) / 256u;
					const unsigned int b = ( b0 * 224u + b1 * 32u ) / 256u;
					AddSaturate( &pOutputBuffer[( y + 1 ) * outWidth - outFringe - 2u],r,g,b );
					AddSaturate( &pOutputBuffer[( y + 1 ) * outWidth - outFringe - 1u],r,g,b );
				}
				{
					const unsigned int r = ( r0 * 160u + r1 * 96u ) / 256u;
					const unsigned int g = ( g0 * 160u + g1 * 96u ) / 256u;
					const unsigned int b = ( b0 * 160u + b1 * 96u ) / 256u;
					AddSaturate( &pOutputBuffer[( y + 2 ) * outWidth - outFringe - 2u],r,g,b );
					AddSaturate( &pOutputBuffer[( y + 2 ) * outWidth - outFringe - 1u],r,g,b );
				}
				{
					const unsigned int r = ( r0 * 96u + r1 * 160u ) / 256u;
					const unsigned int g = ( g0 * 96u + g1 * 160u ) / 256u;
					const unsigned int b = ( b0 * 96u + b1 * 160u ) / 256u;
					AddSaturate( &pOutputBuffer[( y + 3 ) * outWidth - outFringe - 2u],r,g,b );
					AddSaturate( &pOutputBuffer[( y + 3 ) * outWidth - outFringe - 1u],r,g,b );
				}
				{
					const unsigned int r = ( r0 * 32u + r1 * 224u ) / 256u;
					const unsigned int g = ( g0 * 32u + g1 * 224u ) / 256u;
					const unsigned int b = ( b0 * 32u + b1 * 224u ) / 256u;
					AddSaturate( &pOutputBuffer[( y + 4 ) * outWidth - outFringe - 2u],r,g,b );
					AddSaturate( &pOutputBuffer[( y + 4 ) * outWidth - outFringe - 1u],r,g,b );
				}
			}
		}

		// bottom two rows
		{
			// bottom left block
			{
				const unsigned int r = pInputBuffer[inBottomLeft].r;
				const unsigned int g = pInputBuffer[inBottomLeft].g;
				const unsigned int b = pInputBuffer[inBottomLeft].b;
				AddSaturate( &pOutputBuffer[outBottomLeft - outWidth],r,g,b );
				AddSaturate( &pOutputBuffer[outBottomLeft - outWidth + 1u],r,g,b );
				AddSaturate( &pOutputBuffer[outBottomLeft],r,g,b );
				AddSaturate( &pOutputBuffer[outBottomLeft + 1u],r,g,b );
			}

			// center
			{
				Color* const pOutUpper = &pOutputBuffer[( outBottom - 2u ) * outWidth];
				Color* const pOutLower = &pOutputBuffer[( outBottom - 1u ) * outWidth];
				const Color* const pIn = &pInputBuffer[( inBottom - 1u ) * inWidth];
				for( size_t x = outFringe + 2u; x < outRight - 2u; x += 4u )
				{
					const size_t baseX = ( x - 2u ) / 4u;
					const unsigned int r0 = pIn[baseX].r;
					const unsigned int g0 = pIn[baseX].g;
					const unsigned int b0 = pIn[baseX].b;
					const unsigned int r1 = pIn[baseX + 1u].r;
					const unsigned int g1 = pIn[baseX + 1u].g;
					const unsigned int b1 = pIn[baseX + 1u].b;
					{
						const unsigned int r = ( r0 * 224u + r1 * 32u ) / 256u;
						const unsigned int g = ( g0 * 224u + g1 * 32u ) / 256u;
						const unsigned int b = ( b0 * 224u + b1 * 32u ) / 256u;
						AddSaturate( &pOutUpper[x],r,g,b );
						AddSaturate( &pOutLower[x],r,g,b );
					}
					{
						const unsigned int r = ( r0 * 160u + r1 * 96u ) / 256u;
						const unsigned int g = ( g0 * 160u + g1 * 96u ) / 256u;
						const unsigned int b = ( b0 * 160u + b1 * 96u ) / 256u;
						AddSaturate( &pOutUpper[x + 1u],r,g,b );
						AddSaturate( &pOutLower[x + 1u],r,g,b );
					}
					{
						const unsigned int r = ( r0 * 96u + r1 * 160u ) / 256u;
						const unsigned int g = ( g0 * 96u + g1 * 160u ) / 256u;
						const unsigned int b = ( b0 * 96u + b1 * 160u ) / 256u;
						AddSaturate( &pOutUpper[x + 2u],r,g,b );
						AddSaturate( &pOutLower[x + 2u],r,g,b );
					}
					{
						const unsigned int r = ( r0 * 32u + r1 * 224u ) / 256u;
						const unsigned int g = ( g0 * 32u + g1 * 224u ) / 256u;
						const unsigned int b = ( b0 * 32u + b1 * 224u ) / 256u;
						AddSaturate( &pOutUpper[x + 3u],r,g,b );
						AddSaturate( &pOutLower[x + 3u],r,g,b );
					}
				}
			}

			// bottom right block
			{
				const unsigned int r = pInputBuffer[inBottomRight].r;
				const unsigned int g = pInputBuffer[inBottomRight].g;
				const unsigned int b = pInputBuffer[inBottomRight].b;
				AddSaturate( &pOutputBuffer[outBottomRight - outWidth - 1u],r,g,b );
				AddSaturate( &pOutputBuffer[outBottomRight - outWidth],r,g,b );
				AddSaturate( &pOutputBuffer[outBottomRight - 1u],r,g,b );
				AddSaturate( &pOutputBuffer[outBottomRight],r,g,b );
			}
		}
	}
	void Go()
	{
		//input.Save( L"shot_0pre.bmp" );
		DownsizePass();
		//hBuffer.Save( L"shot_1down.bmp" );
		timer.StartFrame();
		HorizontalPass();
		if( timer.StopFrame() )
		{
			log << timer.GetAvg() << std::endl;
		}
		//vBuffer.Save( L"shot_2h.bmp" );
		VerticalPass();
		//hBuffer.Save( L"shot_3v.bmp" );
		UpsizeBlendPass();
		//input.Save( L"shot_4post.bmp" );
	}
	static unsigned int GetFringeSize()
	{
		return ( diameter / 2u ) * 4u;
	}
private:
	static unsigned int GetKernelCenter()
	{
		return ( diameter - 1 ) / 2;
	}
	void _DownsizePassSSSE3()
	{
		// surface height needs to be a multiple of 4
		assert( input.GetHeight() % 4u == 0u );

		// useful constants
		const __m128i zero = _mm_setzero_si128();
		const __m128i bloomShufLo = _mm_set_epi8(
			128u,128u,128u,7u,128u,7u,128u,7u,
			128u,128u,128u,3u,128u,3u,128u,3u );
		const __m128i bloomShufHi = _mm_set_epi8(
			128u,128u,128u,15u,128u,15u,128u,15u,
			128u,128u,128u,11u,128u,11u,128u,11u );

		// subroutine
		const auto ProcessRow = [=]( __m128i row )
		{
			// unpack byte channels of 2 upper and 2 lower pixels to words
			const __m128i chanLo = _mm_unpacklo_epi8( row,zero );
			const __m128i chanHi = _mm_unpackhi_epi8( row,zero );

			// broadcast bloom value to all channels in the same pixel
			const __m128i bloomLo = _mm_shuffle_epi8( row,bloomShufLo );
			const __m128i bloomHi = _mm_shuffle_epi8( row,bloomShufHi );

			// multiply bloom with color channels
			const __m128i prodLo = _mm_mullo_epi16( chanLo,bloomLo );
			const __m128i prodHi = _mm_mullo_epi16( chanHi,bloomHi );

			// predivide channels by 16
			const __m128i predivLo = _mm_srli_epi16( prodLo,4u );
			const __m128i predivHi = _mm_srli_epi16( prodHi,4u );

			// add upper and lower 2-pixel groups and return result
			return _mm_add_epi16( predivLo,predivHi );
		};

		for( size_t yIn = 0u,yOut = 0u; yIn < size_t( input.GetHeight() ); yIn += 4u,yOut++ )
		{
			// initialize input pointers
			const __m128i* pRow0 = reinterpret_cast<const __m128i*>(
				&input.GetBufferConst()[input.GetPitch() * yIn] );
			const __m128i* pRow1 = reinterpret_cast<const __m128i*>(
				&input.GetBufferConst()[input.GetPitch() * ( yIn + 1u )] );
			const __m128i* pRow2 = reinterpret_cast<const __m128i*>(
				&input.GetBufferConst()[input.GetPitch() * ( yIn + 2u )] );
			const __m128i* pRow3 = reinterpret_cast<const __m128i*>(
				&input.GetBufferConst()[input.GetPitch() * ( yIn + 3u )] );
			// initialize output pointer
			Color* pOut = &hBuffer.GetBuffer()[hBuffer.GetPitch() * yOut];
			// row end pointer
			const __m128i* const pRowEnd = pRow1;

			for( ; pRow0 < pRowEnd; pRow0++,pRow1++,pRow2++,pRow3++,pOut++ )
			{
				// load pixels
				const __m128i row0 = _mm_load_si128( pRow0 );
				const __m128i row1 = _mm_load_si128( pRow1 );
				const __m128i row2 = _mm_load_si128( pRow2 );
				const __m128i row3 = _mm_load_si128( pRow3 );

				// process rows and sum results
				__m128i sum = ProcessRow( row0 );
				sum = _mm_add_epi16( sum,ProcessRow( row1 ) );
				sum = _mm_add_epi16( sum,ProcessRow( row2 ) );
				sum = _mm_add_epi16( sum,ProcessRow( row3 ) );

				// add high and low pixel channel sums
				sum = _mm_add_epi16( sum,_mm_srli_si128( sum,8u ) );

				// divide channel sums by 64
				sum = _mm_srli_epi16( sum,6u );

				// pack word channels to bytes and store in output buffer
				*pOut = _mm_cvtsi128_si32( _mm_packus_epi16( sum,sum ) );
			}
		}
	}
	void _DownsizePassSSE2()
	{		// surface height needs to be a multiple of 4
		assert( input.GetHeight() % 4u == 0u );

		// useful constants
		const __m128i zero = _mm_setzero_si128();

		// subroutine
		const auto ProcessRow = [&zero]( __m128i row )
		{
			// unpack byte channels of 2 upper and 2 lower pixels to words
			const __m128i chanLo = _mm_unpacklo_epi8( row,zero );
			const __m128i chanHi = _mm_unpackhi_epi8( row,zero );

			// broadcast bloom value to all channels in the same pixel
			const __m128i bloomLo = _mm_shufflehi_epi16( _mm_shufflelo_epi16(
				chanLo,_MM_SHUFFLE( 3u,3u,3u,3u ) ),_MM_SHUFFLE( 3u,3u,3u,3u ) );
			const __m128i bloomHi = _mm_shufflehi_epi16( _mm_shufflelo_epi16(
				chanHi,_MM_SHUFFLE( 3u,3u,3u,3u ) ),_MM_SHUFFLE( 3u,3u,3u,3u ) );

			// multiply bloom with color channels
			const __m128i prodLo = _mm_mullo_epi16( chanLo,bloomLo );
			const __m128i prodHi = _mm_mullo_epi16( chanHi,bloomHi );

			// predivide channels by 16
			const __m128i predivLo = _mm_srli_epi16( prodLo,4u );
			const __m128i predivHi = _mm_srli_epi16( prodHi,4u );

			// add upper and lower 2-pixel groups and return result
			return _mm_add_epi16( predivLo,predivHi );
		};

		for( size_t yIn = 0u,yOut = 0u; yIn < size_t( input.GetHeight() ); yIn += 4u,yOut++ )
		{
			// initialize input pointers
			const __m128i* pRow0 = reinterpret_cast<const __m128i*>(
				&input.GetBufferConst()[input.GetPitch() * yIn] );
			const __m128i* pRow1 = reinterpret_cast<const __m128i*>(
				&input.GetBufferConst()[input.GetPitch() * ( yIn + 1u )] );
			const __m128i* pRow2 = reinterpret_cast<const __m128i*>(
				&input.GetBufferConst()[input.GetPitch() * ( yIn + 2u )] );
			const __m128i* pRow3 = reinterpret_cast<const __m128i*>(
				&input.GetBufferConst()[input.GetPitch() * ( yIn + 3u )] );
			// initialize output pointer
			Color* pOut = &hBuffer.GetBuffer()[hBuffer.GetPitch() * yOut];
			// row end pointer
			const __m128i* const pRowEnd = pRow1;

			for( ; pRow0 < pRowEnd; pRow0++,pRow1++,pRow2++,pRow3++,pOut++ )
			{
				// load pixels
				const __m128i row0 = _mm_load_si128( pRow0 );
				const __m128i row1 = _mm_load_si128( pRow1 );
				const __m128i row2 = _mm_load_si128( pRow2 );
				const __m128i row3 = _mm_load_si128( pRow3 );

				// process rows and sum results
				__m128i sum = ProcessRow( row0 );
				sum = _mm_add_epi16( sum,ProcessRow( row1 ) );
				sum = _mm_add_epi16( sum,ProcessRow( row2 ) );
				sum = _mm_add_epi16( sum,ProcessRow( row3 ) );

				// add high and low pixel channel sums
				sum = _mm_add_epi16( sum,_mm_srli_si128( sum,8u ) );

				// divide channel sums by 256
				sum = _mm_srli_epi16( sum,8u );

				// pack word channels to bytes and store in output buffer
				*pOut = _mm_cvtsi128_si32( _mm_packus_epi16( sum,sum ) );
			}
		}
	}
	void _DownsizePassX86()
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
				const Color p0 = pInputBuffer[( y * 4 )     * inWidth + x * 4];
				const Color p1 = pInputBuffer[( y * 4 )     * inWidth + x * 4 + 1];
				const Color p2 = pInputBuffer[( y * 4 )     * inWidth + x * 4 + 2];
				const Color p3 = pInputBuffer[( y * 4 )     * inWidth + x * 4 + 3];
				const Color p4 = pInputBuffer[( y * 4 + 1 ) * inWidth + x * 4];
				const Color p5 = pInputBuffer[( y * 4 + 1 ) * inWidth + x * 4 + 1];
				const Color p6 = pInputBuffer[( y * 4 + 1 ) * inWidth + x * 4 + 2];
				const Color p7 = pInputBuffer[( y * 4 + 1 ) * inWidth + x * 4 + 3];
				const Color p8 = pInputBuffer[( y * 4 + 2 ) * inWidth + x * 4];
				const Color p9 = pInputBuffer[( y * 4 + 2 ) * inWidth + x * 4 + 1];
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
	void _HorizontalPassSSSE3Ex()
	{
		// useful constants
		const __m128i zero = _mm_setzero_si128();
		const __m128i coefMaskStart = _mm_set_epi8(
			0x80u,0x01u,0x80u,0x01u,0x80u,0x01u,0x80u,0x01u,
			0x80u,0x00u,0x80u,0x00u,0x80u,0x00u,0x80u,0x00u );
		const __m128i coefMaskDelta = _mm_set_epi8(
			0x00u,0x02u,0x00u,0x02u,0x00u,0x02u,0x00u,0x02u,
			0x00u,0x02u,0x00u,0x02u,0x00u,0x02u,0x00u,0x02u );
		// load coefficient bytes
		const __m128i coef = _mm_load_si128( ( __m128i* )kernel );

		// routines
		auto Process8Pixels = [=]( const __m128i srclo,const __m128i srchi,__m128i& coefMask )
		{
			// for accumulating sum of high and low pixels in srclo and srchi
			__m128i sum;

			// process low pixels of srclo
			{
				// unpack two pixel byte->word components into src from lo end of srclo
				const __m128i src = _mm_unpacklo_epi8( srclo,zero );

				// broadcast and unpack coefficients to top and bottom 4 words
				const __m128i co = _mm_shuffle_epi8( coef,coefMask );

				// increment mask indices by 2
				coefMask = _mm_add_epi8( coefMask,coefMaskDelta );

				// multiply pixel components by coefficients
				const __m128i prod = _mm_mullo_epi16( co,src );

				// predivide by 16 and accumulate
				sum = _mm_srli_epi16( prod,4 );
			}
			// process high pixels of srclo
			{
				// unpack two pixel byte->word components into src from lo end of srclo
				const __m128i src = _mm_unpackhi_epi8( srclo,zero );

				// broadcast and unpack coefficients to top and bottom 4 words
				const __m128i co = _mm_shuffle_epi8( coef,coefMask );

				// increment mask indices by 2
				coefMask = _mm_add_epi8( coefMask,coefMaskDelta );

				// multiply pixel components by coefficients
				const __m128i prod = _mm_mullo_epi16( co,src );

				// predivide by 16 and accumulate
				const __m128i prediv = _mm_srli_epi16( prod,4 );
				sum = _mm_add_epi16( sum,prediv );
			}
			// process low pixels of srchi
			{
				// unpack two pixel byte->word components into src from lo end of srchi
				const __m128i src = _mm_unpacklo_epi8( srchi,zero );

				// broadcast and unpack coefficients to top and bottom 4 words
				const __m128i co = _mm_shuffle_epi8( coef,coefMask );

				// increment mask indices by 2
				coefMask = _mm_add_epi8( coefMask,coefMaskDelta );

				// multiply pixel components by coefficients
				const __m128i prod = _mm_mullo_epi16( co,src );

				// predivide by 16 and accumulate
				const __m128i prediv = _mm_srli_epi16( prod,4 );
				sum = _mm_add_epi16( sum,prediv );
			}
			// process high pixels of srchi
			{
				// unpack two pixel byte->word components into src from lo end of srchi
				const __m128i src = _mm_unpackhi_epi8( srchi,zero );

				// broadcast and unpack coefficients to top and bottom 4 words
				const __m128i co = _mm_shuffle_epi8( coef,coefMask );

				// increment mask indices by 2
				coefMask = _mm_add_epi8( coefMask,coefMaskDelta );

				// multiply pixel components by coefficients
				const __m128i prod = _mm_mullo_epi16( co,src );

				// predivide by 16 and accumulate
				const __m128i prediv = _mm_srli_epi16( prod,4 );
				sum = _mm_add_epi16( sum,prediv );
			}
			return sum;
		};

		// indexing constants
		const size_t centerKernel = GetKernelCenter();
		const size_t width = hBuffer.GetWidth();
		const size_t height = hBuffer.GetHeight();

		for( size_t y = 0u; y < height; y++ )
		{
			// setup pointers
			const __m128i* pIn = reinterpret_cast<const __m128i*>(
				&hBuffer.GetBufferConst()[y * hBuffer.GetPitch()] );
			const __m128i* const pEnd = reinterpret_cast<const __m128i*>(
				&hBuffer.GetBufferConst()[( y + 1 ) * hBuffer.GetPitch()] );
			Color* pOut = &vBuffer.GetBuffer()[y * vBuffer.GetPitch() + GetKernelCenter()];

			// preload input pixels for convolution window
			__m128i src0 = _mm_load_si128( pIn );
			pIn++;
			__m128i src1 = _mm_load_si128( pIn );
			pIn++;
			__m128i src2 = _mm_load_si128( pIn );
			pIn++;
			__m128i src3 = _mm_load_si128( pIn );
			pIn++;

			for( ; pIn < pEnd; pIn++ )
			{
				// on-deck pixels for shifting into convolution window
				__m128i deck = _mm_load_si128( pIn );

				for( size_t i = 0u; i < 4u; i++,pOut++ )
				{
					// setup coefficient shuffle mask
					__m128i coefMask = coefMaskStart;

					// process convolution window and accumulate
					__m128i sum16 = Process8Pixels( src0,src1,coefMask );
					sum16 = _mm_add_epi16( sum16,Process8Pixels( src2,src3,coefMask ) );

					// add low and high accumulators
					sum16 = _mm_add_epi16( sum16,_mm_srli_si128( sum16,8 ) );

					// divide by 64 (16 x 64 = 1024 in total / 2x overdrive factor)
					sum16 = _mm_srli_epi16( sum16,6 );

					// pack result and output to buffer
					*pOut = _mm_cvtsi128_si32( _mm_packus_epi16( sum16,sum16 ) );

					// 640-bit shift--from deck down to src0
					src0 = _mm_alignr_epi8( src1,src0,4 );
					src1 = _mm_alignr_epi8( src2,src1,4 );
					src2 = _mm_alignr_epi8( src3,src2,4 );
					src3 = _mm_alignr_epi8( deck,src3,4 );
					deck = _mm_srli_si128( deck,4 );
				}
			}
			// final pixel end of row
			{
				// setup coefficient shuffle mask
				__m128i coefMask = coefMaskStart;

				// process convolution window and accumulate
				__m128i sum16 = Process8Pixels( src0,src1,coefMask );
				sum16 = _mm_add_epi16( sum16,Process8Pixels( src2,src3,coefMask ) );

				// add low and high accumulators
				sum16 = _mm_add_epi16( sum16,_mm_srli_si128( sum16,8 ) );

				// divide by 64 (16 x 64 = 1024 in total / 2x overdrive factor)
				sum16 = _mm_srli_epi16( sum16,6 );

				// pack result and output to buffer
				*pOut = _mm_cvtsi128_si32( _mm_packus_epi16( sum16,sum16 ) );
			}
		}

	}
	void _HorizontalPassSSSE3()
	{
		// useful constants
		const __m128i zero = _mm_setzero_si128();

		// routines
		auto Process8Pixels = [zero]( const __m128i srclo,const __m128i srchi,const __m128i coef )
		{
			// for accumulating sum of high and low pixels in srclo and srchi
			__m128i sum;

			// process low pixels of srclo
			{
				// unpack two pixel byte->word components into src from lo end of srclo
				const __m128i src = _mm_unpacklo_epi8( srclo,zero );

				// broadcast coefficients 1,0 to top and bottom 4 words
				// (first duplicate WORD coeffients in low DWORDS, then shuffle by DWORDS)
				const __m128i co = _mm_shuffle_epi32( _mm_shufflelo_epi16(
					coef,_MM_SHUFFLE( 1,1,0,0 ) ),_MM_SHUFFLE( 1,1,0,0 ) );

				// multiply pixel components by coefficients
				const __m128i prod = _mm_mullo_epi16( co,src );

				// predivide by 16 and accumulate
				sum = _mm_srli_epi16( prod,4 );
			}
			// process high pixels of srclo
			{
				// unpack two pixel byte->word components into src from lo end of srclo
				const __m128i src = _mm_unpackhi_epi8( srclo,zero );

				// broadcast coefficients 1,0 to top and bottom 4 words
				// (first duplicate WORD coeffients in low DWORDS, then shuffle by DWORDS)
				const __m128i co = _mm_shuffle_epi32( _mm_shufflelo_epi16(
					coef,_MM_SHUFFLE( 3,3,2,2 ) ),_MM_SHUFFLE( 1,1,0,0 ) );

				// multiply pixel components by coefficients
				const __m128i prod = _mm_mullo_epi16( co,src );

				// predivide by 16 and accumulate
				const __m128i prediv = _mm_srli_epi16( prod,4 );
				sum = _mm_add_epi16( sum,prediv );
			}
			// process low pixels of srchi
			{
				// unpack two pixel byte->word components into src from lo end of srchi
				const __m128i src = _mm_unpacklo_epi8( srchi,zero );

				// broadcast coefficients 1,0 to top and bottom 4 words
				// (first duplicate WORD coeffients in low DWORDS, then shuffle by DWORDS)
				const __m128i co = _mm_shuffle_epi32( _mm_shufflehi_epi16(
					coef,_MM_SHUFFLE( 1,1,0,0 ) ),_MM_SHUFFLE( 3,3,2,2 ) );

				// multiply pixel components by coefficients
				const __m128i prod = _mm_mullo_epi16( co,src );

				// predivide by 16 and accumulate
				const __m128i prediv = _mm_srli_epi16( prod,4 );
				sum = _mm_add_epi16( sum,prediv );
			}
			// process high pixels of srchi
			{
				// unpack two pixel byte->word components into src from lo end of srchi
				const __m128i src = _mm_unpackhi_epi8( srchi,zero );

				// broadcast coefficients 1,0 to top and bottom 4 words
				// (first duplicate WORD coeffients in low DWORDS, then shuffle by DWORDS)
				const __m128i co = _mm_shuffle_epi32( _mm_shufflehi_epi16(
					coef,_MM_SHUFFLE( 3,3,2,2 ) ),_MM_SHUFFLE( 3,3,2,2 ) );

				// multiply pixel components by coefficients
				const __m128i prod = _mm_mullo_epi16( co,src );

				// predivide by 16 and accumulate
				const __m128i prediv = _mm_srli_epi16( prod,4 );
				sum = _mm_add_epi16( sum,prediv );
			}
			return sum;
		};

		// indexing constants
		const size_t centerKernel = GetKernelCenter();
		const size_t width = hBuffer.GetWidth();
		const size_t height = hBuffer.GetHeight();

		// load coefficent bytes and unpack to words
		const __m128i coef = _mm_load_si128( ( __m128i* )kernel );
		const __m128i coefLo = _mm_unpacklo_epi8( coef,zero );
		const __m128i coefHi = _mm_unpackhi_epi8( coef,zero );

		for( size_t y = 0u; y < height; y++ )
		{
			// setup pointers
			const __m128i* pIn = reinterpret_cast<const __m128i*>(
				&hBuffer.GetBufferConst()[y * hBuffer.GetPitch()] );
			const __m128i* const pEnd = reinterpret_cast<const __m128i*>(
				&hBuffer.GetBufferConst()[( y + 1 ) * hBuffer.GetPitch()] );
			Color* pOut = &vBuffer.GetBuffer()[y * vBuffer.GetPitch() + GetKernelCenter()];

			// preload input pixels for convolution window
			__m128i src0 = _mm_load_si128( pIn );
			pIn++;
			__m128i src1 = _mm_load_si128( pIn );
			pIn++;
			__m128i src2 = _mm_load_si128( pIn );
			pIn++;
			__m128i src3 = _mm_load_si128( pIn );
			pIn++;

			for( ; pIn < pEnd; pIn++ )
			{
				// on-deck pixels for shifting into convolution window
				__m128i deck = _mm_load_si128( pIn );

				for( size_t i = 0u; i < 4u; i++,pOut++ )
				{
					// process convolution window and accumulate
					__m128i sum16 = Process8Pixels( src0,src1,coefLo );
					sum16 = _mm_add_epi16( sum16,Process8Pixels( src2,src3,coefHi ) );

					// add low and high accumulators
					sum16 = _mm_add_epi16( sum16,_mm_srli_si128( sum16,8 ) );

					// divide by 64 (16 x 64 = 1024 in total / 2x overdrive factor)
					sum16 = _mm_srli_epi16( sum16,6 );

					// pack result and output to buffer
					*pOut = _mm_cvtsi128_si32( _mm_packus_epi16( sum16,sum16 ) );

					// 640-bit shift--from deck down to src0
					src0 = _mm_alignr_epi8( src1,src0,4 );
					src1 = _mm_alignr_epi8( src2,src1,4 );
					src2 = _mm_alignr_epi8( src3,src2,4 );
					src3 = _mm_alignr_epi8( deck,src3,4 );
					deck = _mm_srli_si128( deck,4 );
				}
			}
			// final pixel end of row
			{
				// process convolution window and accumulate
				__m128i sum16 = Process8Pixels( src0,src1,coefLo );
				sum16 = _mm_add_epi16( sum16,Process8Pixels( src2,src3,coefHi ) );

				// add low and high accumulators
				sum16 = _mm_add_epi16( sum16,_mm_srli_si128( sum16,8 ) );

				// divide by 64 (16 x 64 = 1024 in total / 2x overdrive factor)
				sum16 = _mm_srli_epi16( sum16,6 );

				// pack result and output to buffer
				*pOut = _mm_cvtsi128_si32( _mm_packus_epi16( sum16,sum16 ) );
			}
		}

	}
	void _HorizontalPassSSE2()
	{
		// useful constants
		const __m128i zero = _mm_setzero_si128();
		// masks for shifting through convolution window
		const __m128i maskHi = _mm_set_epi32( 0xFFFFFFFF,0x00000000,0x00000000,0x00000000 );
		const __m128i maskLo = _mm_set_epi32( 0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF );

		// routines
		auto Process8Pixels = [=]( const __m128i srclo,const __m128i srchi,const __m128i coef )
		{
			// for accumulating sum of high and low pixels in srclo and srchi
			__m128i sum;

			// process low pixels of srclo
			{
				// unpack two pixel byte->word components into src from lo end of srclo
				const __m128i src = _mm_unpacklo_epi8( srclo,zero );

				// broadcast coefficients 1,0 to top and bottom 4 words
				// (first duplicate WORD coeffients in low DWORDS, then shuffle by DWORDS)
				const __m128i co = _mm_shuffle_epi32( _mm_shufflelo_epi16(
					coef,_MM_SHUFFLE( 1,1,0,0 ) ),_MM_SHUFFLE( 1,1,0,0 ) );

				// multiply pixel components by coefficients
				const __m128i prod = _mm_mullo_epi16( co,src );

				// predivide by 16 and accumulate
				sum = _mm_srli_epi16( prod,4 );
			}
			// process high pixels of srclo
			{
				// unpack two pixel byte->word components into src from lo end of srclo
				const __m128i src = _mm_unpackhi_epi8( srclo,zero );

				// broadcast coefficients 1,0 to top and bottom 4 words
				// (first duplicate WORD coeffients in low DWORDS, then shuffle by DWORDS)
				const __m128i co = _mm_shuffle_epi32( _mm_shufflelo_epi16(
					coef,_MM_SHUFFLE( 3,3,2,2 ) ),_MM_SHUFFLE( 1,1,0,0 ) );

				// multiply pixel components by coefficients
				const __m128i prod = _mm_mullo_epi16( co,src );

				// predivide by 16 and accumulate
				const __m128i prediv = _mm_srli_epi16( prod,4 );
				sum = _mm_add_epi16( sum,prediv );
			}
			// process low pixels of srchi
			{
				// unpack two pixel byte->word components into src from lo end of srchi
				const __m128i src = _mm_unpacklo_epi8( srchi,zero );

				// broadcast coefficients 1,0 to top and bottom 4 words
				// (first duplicate WORD coeffients in low DWORDS, then shuffle by DWORDS)
				const __m128i co = _mm_shuffle_epi32( _mm_shufflehi_epi16(
					coef,_MM_SHUFFLE( 1,1,0,0 ) ),_MM_SHUFFLE( 3,3,2,2 ) );

				// multiply pixel components by coefficients
				const __m128i prod = _mm_mullo_epi16( co,src );

				// predivide by 16 and accumulate
				const __m128i prediv = _mm_srli_epi16( prod,4 );
				sum = _mm_add_epi16( sum,prediv );
			}
			// process high pixels of srchi
			{
				// unpack two pixel byte->word components into src from lo end of srchi
				const __m128i src = _mm_unpackhi_epi8( srchi,zero );

				// broadcast coefficients 1,0 to top and bottom 4 words
				// (first duplicate WORD coeffients in low DWORDS, then shuffle by DWORDS)
				const __m128i co = _mm_shuffle_epi32( _mm_shufflehi_epi16(
					coef,_MM_SHUFFLE( 3,3,2,2 ) ),_MM_SHUFFLE( 3,3,2,2 ) );

				// multiply pixel components by coefficients
				const __m128i prod = _mm_mullo_epi16( co,src );

				// predivide by 16 and accumulate
				const __m128i prediv = _mm_srli_epi16( prod,4 );
				sum = _mm_add_epi16( sum,prediv );
			}
			return sum;
		};
		// the lo must be pre-rotated 32-bit (to allow chaining)
		auto Shift256 = [=]( __m128i& lo,__m128i& hi )
		{
			// rotate second lowest pixel in hi to lowest position (lowest goes to top)
			hi = _mm_shuffle_epi32( hi,_MM_SHUFFLE( 0,3,2,1 ) );
			// clear high pixel of convolution window lo
			lo = _mm_and_si128( lo,maskLo );
			// copy high pixel from hi to high pixel location in lo
			lo = _mm_or_si128( lo,_mm_and_si128( hi,maskHi ) );
		};

		// indexing constants
		const size_t centerKernel = GetKernelCenter();
		const size_t width = hBuffer.GetWidth();
		const size_t height = hBuffer.GetHeight();

		// load coefficent bytes and unpack to words
		const __m128i coef = _mm_load_si128( (__m128i*)kernel );
		const __m128i coefLo = _mm_unpacklo_epi8( coef,zero );
		const __m128i coefHi = _mm_unpackhi_epi8( coef,zero );

		for( size_t y = 0u; y < height; y++ )
		{
			// setup pointers
			const __m128i* pIn = reinterpret_cast<const __m128i*>(
				&hBuffer.GetBufferConst()[y * hBuffer.GetPitch()] );
			const __m128i* const pEnd = reinterpret_cast<const __m128i*>(
				&hBuffer.GetBufferConst()[( y + 1 ) * hBuffer.GetPitch()] );
			Color* pOut = &vBuffer.GetBuffer()[y * vBuffer.GetPitch() + GetKernelCenter()];

			// preload input pixels for convolution window
			__m128i src0 = _mm_load_si128( pIn );
			pIn++;
			__m128i src1 = _mm_load_si128( pIn );
			pIn++;
			__m128i src2 = _mm_load_si128( pIn );
			pIn++;
			__m128i src3 = _mm_load_si128( pIn );
			pIn++;

			for( ; pIn < pEnd; pIn++ )
			{
				// on-deck pixels for shifting into convolution window
				__m128i deck = _mm_load_si128( pIn );

				for( size_t i = 0u; i < 4u; i++,pOut++ )
				{
					// process convolution window and accumulate
					__m128i sum16 = Process8Pixels( src0,src1,coefLo );
					sum16 = _mm_add_epi16( sum16,Process8Pixels( src2,src3,coefHi ) );

					// add low and high accumulators
					sum16 = _mm_add_epi16( sum16,_mm_srli_si128( sum16,8 ) );

					// divide by 64 (16 x 64 = 1024 in total / 2x overdrive factor)
					sum16 = _mm_srli_epi16( sum16,6 );

					// pack result and output to buffer
					*pOut = _mm_cvtsi128_si32( _mm_packus_epi16( sum16,sum16 ) );

					// shift pixels from deck through convolution window
					// pre-rotate src0 to begin chaining
					src0 = _mm_shuffle_epi32( src0,_MM_SHUFFLE( 0,3,2,1 ) );
					// 640-bit chained shift
					Shift256( src0,src1 );
					Shift256( src1,src2 );
					Shift256( src2,src3 );
					Shift256( src3,deck );
				}
			}
			// final pixel end of row
			{
				// process convolution window and accumulate
				__m128i sum16 = Process8Pixels( src0,src1,coefLo );
				sum16 = _mm_add_epi16( sum16,Process8Pixels( src2,src3,coefHi ) );

				// add low and high accumulators
				sum16 = _mm_add_epi16( sum16,_mm_srli_si128( sum16,8 ) );

				// divide by 64 (16 x 64 = 1024 in total / 2x overdrive factor)
				sum16 = _mm_srli_epi16( sum16,6 );

				// pack result and output to buffer
				*pOut = _mm_cvtsi128_si32( _mm_packus_epi16( sum16,sum16 ) );
			}
		}
	}
	void _HorizontalPassX86()
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
					unsigned char( min( r / divisorKernel,255u ) ),
					unsigned char( min( g / divisorKernel,255u ) ),
					unsigned char( min( b / divisorKernel,255u ) )
				};
			}
		}
	}
private:
	static const unsigned int diameter = 16u;
	__declspec( align( 16 ) ) unsigned char kernel[diameter];
	unsigned int divisorKernel = 1024u;
	Surface& input;
	Surface hBuffer;
	Surface vBuffer;
	// function pointers
	std::function<void( BloomProcessor* )> DownsizePassFunc;
	std::function<void( BloomProcessor* )> HorizontalPassFunc;
	// benchmarking
	FrameTimer timer;
	std::wofstream log;
};