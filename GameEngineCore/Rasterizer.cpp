#ifndef BLOCK_SCAN_RASTERIZER_H
#define BLOCK_SCAN_RASTERIZER_H

#include "CoreLib/Basic.h"
#include "CoreLib/VectorMath.h"
#include "Rasterizer.h"

using namespace VectorMath;

namespace GameEngine
{
    inline int GetOutCode(__m128i e0, __m128i e1, __m128i e2)
    {
        int sign0 = _mm_movemask_epi8(e0);
        int sign1 = _mm_movemask_epi8(e1);
        int sign2 = _mm_movemask_epi8(e2);
        int code = (sign0 & 0x8888) | ((sign1 & 0x8888) >> 1) | ((sign2 & 0x8888) >> 2);
        return code;
    }

    struct TriangleSIMD
    {
        __m128i a0, a1, a2, b0, b1, b2, x0, y0, x1, y1, x2, y2, c0, c1, c2;
        int isOwnerEdge[3];

        inline void LoadForCoordinates(ProjectedTriangle & tri)
        {
            a0 = _mm_set1_epi32(tri.A0);
            a1 = _mm_set1_epi32(tri.A1);
            a2 = _mm_set1_epi32(tri.A2);
            b0 = _mm_set1_epi32(tri.B0);
            b1 = _mm_set1_epi32(tri.B1);
            b2 = _mm_set1_epi32(tri.B2);
            x0 = _mm_set1_epi32(tri.X0);
            y0 = _mm_set1_epi32(tri.Y0);
            x1 = _mm_set1_epi32(tri.X1);
            y1 = _mm_set1_epi32(tri.Y1);
            x2 = _mm_set1_epi32(tri.X2);
            y2 = _mm_set1_epi32(tri.Y2);
            c0 = _mm_set1_epi32(tri.C0);
            c1 = _mm_set1_epi32(tri.C1);
            c2 = _mm_set1_epi32(tri.C2);
        }

        inline bool TouchesBlock(__m128i x, __m128i y)
        {
            auto e0 = a0 * (x - x0) + b0 * (y - y0);
            auto e1 = a1 * (x - x1) + b1 * (y - y1);
            auto e2 = a2 * (x - x2) + b2 * (y - y2);
            int code = GetOutCode(e0, e1, e2);
            code = code & (code >> 8);
            code = code & (code >> 4);
            return code == 0;
        }

        inline void Load(ProjectedTriangle & tri)
        {
            a0 = _mm_set1_epi32(tri.A0);
            a1 = _mm_set1_epi32(tri.A1);
            a2 = _mm_set1_epi32(tri.A2);
            b0 = _mm_set1_epi32(tri.B0);
            b1 = _mm_set1_epi32(tri.B1);
            b2 = _mm_set1_epi32(tri.B2);
            x0 = _mm_set1_epi32(tri.X0);
            y0 = _mm_set1_epi32(tri.Y0);
            x1 = _mm_set1_epi32(tri.X1);
            y1 = _mm_set1_epi32(tri.Y1);
            x2 = _mm_set1_epi32(tri.X2);
            y2 = _mm_set1_epi32(tri.Y2);
            c0 = _mm_set1_epi32(tri.C0);
            c1 = _mm_set1_epi32(tri.C1);
            c2 = _mm_set1_epi32(tri.C2);

            isOwnerEdge[0] = tri.Y0 < tri.Y1 || (tri.Y0 == tri.Y1 && tri.Y2 >= tri.Y0);
            isOwnerEdge[1] = tri.Y1 < tri.Y2 || (tri.Y1 == tri.Y2 && tri.Y0 >= tri.Y1);
            isOwnerEdge[2] = tri.Y2 < tri.Y0 || (tri.Y0 == tri.Y2 && tri.Y1 >= tri.Y0);
        }
        /*
        template< typename TShader, int32 Dilate >
void RasterizeTriangle( TShader& Shader, const FVector2D Points[3], int32 ScissorWidth, int32 ScissorHeight )
{
	const FVector2D HalfPixel( 0.5f, 0.5f );
	FVector2D p0 = Points[0] - HalfPixel;
	FVector2D p1 = Points[1] - HalfPixel;
	FVector2D p2 = Points[2] - HalfPixel;

	// Correct winding
	float Facing = ( p0.X - p1.X ) * ( p2.Y - p0.Y ) - ( p0.Y - p1.Y ) * ( p2.X - p0.X );
	if( Facing < 0.0f )
	{
		Swap( p0, p2 );
	}

	// 28.4 fixed point
	const int32 X0 = (int32)( 16.0f * p0.X + 0.5f );
	const int32 X1 = (int32)( 16.0f * p1.X + 0.5f );
	const int32 X2 = (int32)( 16.0f * p2.X + 0.5f );
	
	const int32 Y0 = (int32)( 16.0f * p0.Y + 0.5f );
	const int32 Y1 = (int32)( 16.0f * p1.Y + 0.5f );
	const int32 Y2 = (int32)( 16.0f * p2.Y + 0.5f );

	// Bounding rect
	int32 MinX = ( FMath::Min3( X0, X1, X2 ) - Dilate + 15 ) / 16;
	int32 MaxX = ( FMath::Max3( X0, X1, X2 ) + Dilate + 15 ) / 16;
	int32 MinY = ( FMath::Min3( Y0, Y1, Y2 ) - Dilate + 15 ) / 16;
	int32 MaxY = ( FMath::Max3( Y0, Y1, Y2 ) + Dilate + 15 ) / 16;

	// Clip to image
	MinX = FMath::Clamp( MinX, 0, ScissorWidth );
	MaxX = FMath::Clamp( MaxX, 0, ScissorWidth );
	MinY = FMath::Clamp( MinY, 0, ScissorHeight );
	MaxY = FMath::Clamp( MaxY, 0, ScissorHeight );

	// Deltas
	const int32 DX01 = X0 - X1;
	const int32 DX12 = X1 - X2;
	const int32 DX20 = X2 - X0;

	const int32 DY01 = Y0 - Y1;
	const int32 DY12 = Y1 - Y2;
	const int32 DY20 = Y2 - Y0;

	// Half-edge constants
	int32 C0 = DY01 * X0 - DX01 * Y0;
	int32 C1 = DY12 * X1 - DX12 * Y1;
	int32 C2 = DY20 * X2 - DX20 * Y2;

	// Correct for fill convention
	C0 += ( DY01 < 0 || ( DY01 == 0 && DX01 > 0 ) ) ? 0 : -1;
	C1 += ( DY12 < 0 || ( DY12 == 0 && DX12 > 0 ) ) ? 0 : -1;
	C2 += ( DY20 < 0 || ( DY20 == 0 && DX20 > 0 ) ) ? 0 : -1;

	// Dilate edges
	C0 += ( abs(DX01) + abs(DY01) ) * Dilate;
	C1 += ( abs(DX12) + abs(DY12) ) * Dilate;
	C2 += ( abs(DX20) + abs(DY20) ) * Dilate;

	for( int32 y = MinY; y < MaxY; y++ )
	{
		for( int32 x = MinX; x < MaxX; x++ )
		{
			// same as Edge1 >= 0 && Edge2 >= 0 && Edge3 >= 0
			int32 IsInside;
			IsInside  = C0 + (DX01 * y - DY01 * x) * 16;
			IsInside |= C1 + (DX12 * y - DY12 * x) * 16;
			IsInside |= C2 + (DX20 * y - DY20 * x) * 16;

			if( IsInside >= 0 )
			{
				Shader.Process( x, y );
			}
		}
	}
}
        */
        inline int TestQuadFragment(__m128i x, __m128i y)
        {
            int sign0, sign1, sign2;
            if (isOwnerEdge[0])
            {
                auto e0 = a0 * (x - x0) + b0 * (y - y0) + c0;
                sign0 = ~_mm_movemask_epi8(e0);
            }
            else
            {
                auto e0 = a0 * (x0 - x) + b0 * (y0 - y) - c0;
                sign0 = _mm_movemask_epi8(e0);
            }
            if (isOwnerEdge[1])
            {
                auto e1 = a1 * (x - x1) + b1 * (y - y1) + c1;
                sign1 = ~_mm_movemask_epi8(e1);
            }
            else
            {
                auto e1 = a1 * (x1 - x) + b1 * (y1 - y) - c1;
                sign1 = _mm_movemask_epi8(e1);
            }
            if (isOwnerEdge[2])
            {
                auto e2 = a2 * (x - x2) + b2 * (y - y2) + c2;
                sign2 = ~_mm_movemask_epi8(e2);
            }
            else
            {
                auto e2 = a2 * (x2 - x) + b2 * (y2 - y) - c2;
                sign2 = _mm_movemask_epi8(e2);
            }
            return sign0 & sign1 & sign2 & 0x8888;
        }
    };


    template<typename ProcessPixelFunc>
    inline void BlockScanRasterize(int tileX0, int tileY0, int tileW, int tileH, const ProjectedTriangle & tri, TriangleSIMD & triSIMD, ProcessPixelFunc f)
    {
        // NVIDIA SPECIFIC: coarse rasterization of 8x8 tiles
        int tileX1 = tileX0 + tileW;
        int tileY1 = tileY0 + tileH;
        // identify top vertex
        int startX = tri.X0;
        int startY = tri.Y0;
        if (tri.Y1 < startY || (tri.Y1 == startY && tri.X1 < startX))
        {
            startX = tri.X1;
            startY = tri.Y1;
        }
        if (tri.Y2 < startY || (tri.Y2 == startY && tri.X2 < startX))
        {
            startX = tri.X2;
            startY = tri.Y2;
        }
        // scan 8x8 blocks from top vertex
        bool search = false;
        int startTileX = startX & (~127);
        int startTileY = startY & (~127);
        if (startTileX >= tileX1)
        {
            startTileX = (tileX1 - 1)&(~127);
            search = true;
        }
        if (startTileX < tileX0)
        {
            startTileX = tileX0;
            search = true;
        }
        if (startTileY >= tileY1)
        {
            startTileY = (tileY1 - 1)&(~127);
            search = true;
        }
        if (startTileY < tileY0)
        {
            startTileY = tileY0;
            search = true;
        }
        // if starting point is out of current tile, search for the first triangle interior
        int tx = startTileX;
        int ty = startTileY;
        __m128i step = _mm_set_epi32(32, 64, 96, 128);
        __m128i a0Step = triSIMD.a0*step;
        __m128i a1Step = triSIMD.a1*step;
        __m128i a2Step = triSIMD.a2*step;
        __m128i b0Step = triSIMD.b0*step;
        __m128i b1Step = triSIMD.b1*step;
        __m128i b2Step = triSIMD.b2*step;
        __m128i b0lineStep = triSIMD.b0*_mm_set1_epi32(32);
        __m128i b1lineStep = triSIMD.b1*_mm_set1_epi32(32);
        __m128i b2lineStep = triSIMD.b2*_mm_set1_epi32(32);

        int returnX = tx;
        int downX = tx;
        int movingDir = 0; // 0=right; 1=left
        bool shouldReturn = false;
        bool shouldMoveDown = false;
        bool initial = true;

        while (ty < tileY0 + tileH)
        {
            __m128i x = _mm_set_epi32(tx, tx, tx + 128, tx + 128);
            __m128i y = _mm_set_epi32(ty, ty + 128, ty + 128, ty);

            auto e0 = triSIMD.a0*(x - triSIMD.x0) + triSIMD.b0*(y - triSIMD.y0);
            auto e1 = triSIMD.a1*(x - triSIMD.x1) + triSIMD.b1*(y - triSIMD.y1);
            auto e2 = triSIMD.a2*(x - triSIMD.x2) + triSIMD.b2*(y - triSIMD.y2);

            int code = GetOutCode(e0, e1, e2);

            int mergeResult = ((code >> 4)&(code));
            int edgeCode0 = (mergeResult >> 9) & 7;
            int edgeCode1 = (mergeResult >> 5) & 7;
            int edgeCode2 = (mergeResult >> 1) & 7;

            if (edgeCode1 == 0)
            {
                downX = tx;
                shouldMoveDown = true;
            }
            if (search)
                search = (edgeCode0&edgeCode1&edgeCode2) != 0;

            if (!search)
            {
                // fine rasterize this block
                int codes[5][5]; //[y][x]
                /*
                The SIMD fine rasterizer compute OutCodes at each corner vertex of the 4x4 grid (of quad pixels).
                It is equivalent to the following code:
                    for (int i = 0; i<5; i++)
                        for (int j = 0; j<5; j++)
                        {
                            int x = tx+j*32;
                            int y = ty+i*32;
                            int e0 = tri.A0*(tri.X0-x) + tri.B0*(tri.Y0-y)>=0?1:0;
                            int e1 = tri.A1*(tri.X1-x) + tri.B1*(tri.Y1-y)>=0?2:0;
                            int e2 = tri.A2*(tri.X2-x) + tri.B2*(tri.Y2-y)>=0?4:0;
                            codes[i][j] = e0|e1|e2;
                        }
                This implementation follows the "stepping" idea, covered by Abrush in his larabee rasterizer article.

                */
                auto e0_corner = _mm_set1_epi32(_mm_extract_epi32(e0, 3));
                auto e1_corner = _mm_set1_epi32(_mm_extract_epi32(e1, 3));
                auto e2_corner = _mm_set1_epi32(_mm_extract_epi32(e2, 3));
                auto e0_line0 = e0_corner + a0Step;
                auto e0_line1 = e0_line0 + b0lineStep;
                auto e0_line2 = e0_line1 + b0lineStep;
                auto e0_line3 = e0_line2 + b0lineStep;
                auto e0_line4 = e0_line3 + b0lineStep;
                auto e0_lastLine = e0_corner + b0Step;

                auto e1_line0 = e1_corner + a1Step;
                auto e1_line1 = e1_line0 + b1lineStep;
                auto e1_line2 = e1_line1 + b1lineStep;
                auto e1_line3 = e1_line2 + b1lineStep;
                auto e1_line4 = e1_line3 + b1lineStep;
                auto e1_lastLine = e1_corner + b1Step;

                auto e2_line0 = e2_corner + a2Step;
                auto e2_line1 = e2_line0 + b2lineStep;
                auto e2_line2 = e2_line1 + b2lineStep;
                auto e2_line3 = e2_line2 + b2lineStep;
                auto e2_line4 = e2_line3 + b2lineStep;
                auto e2_lastLine = e2_corner + b2Step;

                int code_line[6];
                code_line[0] = GetOutCode(e0_line0, e1_line0, e2_line0);
                code_line[1] = GetOutCode(e0_line1, e1_line1, e2_line1);
                code_line[2] = GetOutCode(e0_line2, e1_line2, e2_line2);
                code_line[3] = GetOutCode(e0_line3, e1_line3, e2_line3);
                code_line[4] = GetOutCode(e0_line4, e1_line4, e2_line4);
                code_line[5] = GetOutCode(e0_lastLine, e1_lastLine, e2_lastLine);

                for (int i = 0; i < 5; i++)
                {
                    codes[i][1] = (code_line[i] >> 12) & 14;
                    codes[i][2] = (code_line[i] >> 8) & 14;
                    codes[i][3] = (code_line[i] >> 4) & 14;
                    codes[i][4] = (code_line[i]) & 14;
                }
                codes[0][0] = (code >> 12) & 14;
                codes[1][0] = (code_line[5] >> 12) & 14;
                codes[2][0] = (code_line[5] >> 8) & 14;
                codes[3][0] = (code_line[5] >> 4) & 14;
                codes[4][0] = (code_line[5]) & 14;

                // populate each quad pixel and invoke external processing function
                for (int i = 0; i < 4; i++)
                    for (int j = 0; j < 4; j++)
                    {
                        bool trivialAccept = (codes[i][j] | codes[i][j + 1] | codes[i + 1][j] | codes[i + 1][j + 1]) == 0;
                        if (trivialAccept)
                        {
                            f(tx + j * 32, ty + i * 32, true);
                        }
                        else
                        {
                            int testResult = codes[i][j] & codes[i][j + 1] & codes[i + 1][j] & codes[i + 1][j + 1];
                            if (testResult == 0)
                            {
                                f(tx + j * 32, ty + i * 32, false);
                            }
                        }
                    }
            }

            if (initial)
            {
                initial = false;
                shouldReturn = (movingDir == 0 ? (search || edgeCode0 == 0) && tx > tileX0 : (search || edgeCode2 == 0) && tx < tileX1 - 128);
                returnX = tx;
            }

            bool nextRow = false;
            if (movingDir == 0)
            {
                if (!edgeCode2 && tx + 128 < tileX1)
                    tx += 128;
                else
                {
                    if (search && tx + 128 < tileX1)
                        tx += 128;
                    else
                    {
                        if (shouldReturn)
                        {
                            tx = returnX - 128;
                            movingDir = 1;
                            shouldReturn = false;
                        }
                        else
                        {
                            nextRow = true;
                        }
                    }
                }
            }
            else
            {
                if (!edgeCode0 && tx > tileX0)
                    tx -= 128;
                else
                {
                    if (search && tx > tileX0)
                        tx -= 128;
                    else
                    {
                        if (shouldReturn)
                        {
                            tx = returnX + 128;
                            movingDir = 0;
                            shouldReturn = false;
                        }
                        else
                            nextRow = true;
                    }
                }
            }
            if (nextRow)
            {
                if (shouldMoveDown || search)
                {
                    tx = downX;
                    ty += 128;
                    initial = true;
                    shouldMoveDown = false;
                }
                else
                    break;
            }
        }
    }

    // sets up the triangle based on three post-projection clip space coordinates.
    bool Rasterizer::SetupTriangle(ProjectedTriangle & tri, Vec2 s0, Vec2 s1, Vec2 s2, int width, int height, int dilate)
    {
        tri.X0 = (int)((s0.x)*width * 16);
        tri.Y0 = (int)((s0.y)*height * 16);
        tri.X1 = (int)((s1.x)*width * 16);
        tri.Y1 = (int)((s1.y)*height * 16);
        tri.X2 = (int)((s2.x)*width * 16);
        tri.Y2 = (int)((s2.y)*height * 16);
        tri.A0 = tri.Y0 - tri.Y1;
        tri.B0 = tri.X1 - tri.X0;
        tri.A1 = tri.Y1 - tri.Y2;
        tri.B1 = tri.X2 - tri.X1;
        tri.A2 = tri.Y2 - tri.Y0;
        tri.B2 = tri.X0 - tri.X2;
        tri.C0 = (abs(tri.A0) + abs(tri.B0)) * dilate;
        tri.C1 = (abs(tri.A1) + abs(tri.B1)) * dilate;
        tri.C2 = (abs(tri.A2) + abs(tri.B2)) * dilate;

        int divisor = tri.B2*tri.A0 - tri.A2*tri.B0;
        if (divisor > 0)
        {
            tri.A0 = -tri.A0;
            tri.B0 = -tri.B0;
            tri.A1 = -tri.A1;
            tri.B1 = -tri.B1;
            tri.A2 = -tri.A2;
            tri.B2 = -tri.B2;
        }
        return true;
    }

    template<typename SetPixelFunc>
    void RasterizeImpl(ProjectedTriangle & ptri, int w, int h, const SetPixelFunc& setPixel)
    {
        TriangleSIMD triSIMD;
        triSIMD.Load(ptri);
        BlockScanRasterize(0, 0, w << 4, h << 4, ptri, triSIMD, [&](int tx, int ty, bool trivialAccept)
        {
            __m128i coordX_center, coordY_center;

            coordX_center = _mm_set1_epi32(tx);
            coordY_center = _mm_set1_epi32(ty);
            int ix = (tx >> 4);
            int iy = (ty >> 4);
            auto coordX = coordX_center;
            auto coordY = coordY_center;
            int mask = trivialAccept ? 0xFFFF : triSIMD.TestQuadFragment(coordX, coordY);
            if (mask & 0x0008)
            {
                setPixel(ix, iy);
            }
            if (mask & 0x0080)
            {
                setPixel(ix + 1, iy);
            }
            if (mask & 0x0800)
            {
                setPixel(ix, iy + 1);
            }
            if (mask & 0x8000)
            {
                setPixel(ix + 1, iy + 1);
            }
        });
    }
    int Rasterizer::CountOverlap(Canvas & canvas, ProjectedTriangle & ptri)
    {
        int count = 0;
        RasterizeImpl(ptri, canvas.width, canvas.height, [&](int x, int y)
        {
            if (x < canvas.width && y < canvas.height && canvas.bitmap.Contains(y * canvas.width + x))
                count++; 
        });
        return count;
    }
    void Rasterizer::Rasterize(Canvas & canvas, ProjectedTriangle & ptri)
    {
        RasterizeImpl(ptri, canvas.width, canvas.height, [&](int x, int y) 
        {
            if (x < canvas.width && y < canvas.height)
                canvas.bitmap.Add(y * canvas.width + x); 
        });
    }
}

#endif