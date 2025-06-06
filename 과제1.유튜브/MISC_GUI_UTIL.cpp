/*
 * Copyright (C) 2025 Lee Hackseung
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (Copyleft License)
 * as published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pch.h"
#include <gdiplus.h>
#include <algorithm>
#include "misc_gui_util.h"

#pragma comment(lib, "gdiplus")

static ULONG_PTR __gdiplusToken;

bool misc_gui_util_init()
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;

    return Gdiplus::Ok == GdiplusStartup(&__gdiplusToken, &gdiplusStartupInput, nullptr);
}

void misc_gui_util_term()
{
    Gdiplus::GdiplusShutdown(__gdiplusToken);
}

// 픽셀을 설정하는 헬퍼 함수 (좌표 유효성 검사 포함)
inline void __put_pxel(UINT32* data, int width, int height, int x, int y, UINT32 color)
{
    if ((0 <= x) && (width > x) && (0 <= y) && (height > y))
    {
        *(data + (y * width) + x) = color;
    }
}

// 원의 8대칭 점을 Bresenham 원 알고리즘 기반으로 찍음
// PIXEL_PUT 매크로로 좌표 유효성 검사 후 픽셀 설정
inline void __draw_circle_points(UINT32* data, int width, int height, int xc, int yc, int x, int y, UINT32 color)
{
#define PIXEL_PUT(x, y) \
    if ((unsigned)(x) < (unsigned)(width) && (unsigned)(y) < (unsigned)(height)) \
        __put_pxel(data, width, height, x, y, color);

    PIXEL_PUT(xc + x, yc + y)
    PIXEL_PUT(xc - x, yc + y)
    PIXEL_PUT(xc + x, yc - y)
    PIXEL_PUT(xc - x, yc - y)
    PIXEL_PUT(xc + y, yc + x)
    PIXEL_PUT(xc - y, yc + x)
    PIXEL_PUT(xc + y, yc - x)
    PIXEL_PUT(xc - y, yc - x)
}

// 원 그리기 함수 (테두리 두께 고려)
// 중심(xc, yc), 반지름 r, 두께 thickness로 원 테두리를 그림
// GDI+ Bitmap을 LockBits로 잠근 뒤 직접 픽셀 배열을 접근
// 두께가 1 이하일 경우: Bresenham 알고리즘으로 원 점들 계산
// 두꺼운 경우: 외부-내부 반지름 사이의 거리 조건으로 픽셀 설정
void misc_util_draw_circle(Gdiplus::Bitmap* bitmap, int xc, int yc, int r, int thickness, UINT32 color)
{
    if (!bitmap || (r <= 0))
    {
        return;
    }

    Gdiplus::BitmapData bmpData;
    Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());

    // GDI+ Bitmap을 잠금 (쓰기 가능)
    if (Gdiplus::Ok == bitmap->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bmpData))
    {
        UINT32* pixels;
        int width;
        int height;

        pixels = static_cast<UINT32*>(bmpData.Scan0);
        width = bmpData.Stride / 4;
        height = bmpData.Height;

        if (thickness <= 1)
        {
            // Bresenham 방식으로 테두리 얇은 원을 그림
            int x;
            int y;
            int d;

            x = 0;
            y = r;
            d = 3 - (2 * r);

            while (x <= y)
            {
                __draw_circle_points(pixels, width, height, xc, yc, x, y, color);

                if (d < 0)
                {
                    d += (x << 2) + 6;
                }
                else
                {
                    d += ((x - y) << 2) + 10;
                    --y;
                }

                ++x;
            }
        }
        else 
        {
            // 반지름 r 기준으로 두께를 갖는 도넛 형태 영역을 계산해 픽셀 채움
            // 두께가 있는 원 테두리 그리기 (사각형 루프 내 점이 원 둘레에 가까운지 검사)
            int in;
            int rout;
            int limit;
            int thickness_half;            

            thickness_half = thickness >> 1;
            in = (r - thickness_half) * (r - thickness_half);
            rout = (r + thickness_half) * (r + thickness_half);
            limit = r + thickness;

            for (int dy = -limit; dy <= limit; ++dy) {
                int y;
                UINT32* base_y;
                int dy2;

                y = yc + dy;

                if (y < 0 || y >= height)
                {
                    continue;
                }

                base_y = pixels + (y * width);
                dy2 = dy * dy;

                for (int dx = -limit; dx <= limit; ++dx)
                {
                    int dist;
                    int x;

                    x = xc + dx;

                    if ((x < 0) || (x >= width))
                    {
                        continue;
                    }

                    dist = dx * dx + dy2;

                    if ((dist >= in) && (dist <= rout)) {
                        *(base_y + x) = color;
                    }
                }
            }
        }

        bitmap->UnlockBits(&bmpData);  // 잠금 해제
    }
}

// 내부가 채워진 원 그리기 함수
// 중심(xc, yc), 반지름 r에 해당하는 내부를 모두 채워 그리는 함수
// dx² + dy² <= r² 조건을 만족하는 픽셀만 채움
void misc_util_draw_fill_circle(Gdiplus::Bitmap* bitmap, int xc, int yc, int r, UINT32 color)
{
    if (!bitmap || (0 >= r))
    {
        return;
    }

    Gdiplus::BitmapData bmpData;
    Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());

    if (Gdiplus::Ok == bitmap->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bmpData)) {
        UINT32* data;
        int width;
        int height;
        int r2;

        data = static_cast<UINT32*>(bmpData.Scan0);
        width = bmpData.Stride / 4;
        height = bmpData.Height;
        r2 = r * r;

        for (int dy = -r; dy <= r; ++dy)
        {
            UINT32* base_y;
            int y = yc + dy;

            if (y < 0 || (y >= height))
            {
                continue;
            }

            base_y = data + (y * width);

            for (int dx = -r; dx <= r; ++dx)
            {
                int x;

                x = xc + dx;

                if (x < 0 || (x >= width))
                {
                    continue;
                }

                if (dx * dx + dy * dy <= r2)
                {
                    *(base_y + x) = color;
                }
            }
        }

        bitmap->UnlockBits(&bmpData);
    }
}
