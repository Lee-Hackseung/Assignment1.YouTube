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

// �ȼ��� �����ϴ� ���� �Լ� (��ǥ ��ȿ�� �˻� ����)
inline void __put_pxel(UINT32* data, int width, int height, int x, int y, UINT32 color)
{
    if ((0 <= x) && (width > x) && (0 <= y) && (height > y))
    {
        *(data + (y * width) + x) = color;
    }
}

// ���� 8��Ī ���� Bresenham �� �˰��� ������� ����
// PIXEL_PUT ��ũ�η� ��ǥ ��ȿ�� �˻� �� �ȼ� ����
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

// �� �׸��� �Լ� (�׵θ� �β� ���)
// �߽�(xc, yc), ������ r, �β� thickness�� �� �׵θ��� �׸�
// GDI+ Bitmap�� LockBits�� ��� �� ���� �ȼ� �迭�� ����
// �β��� 1 ������ ���: Bresenham �˰������� �� ���� ���
// �β��� ���: �ܺ�-���� ������ ������ �Ÿ� �������� �ȼ� ����
void misc_util_draw_circle(Gdiplus::Bitmap* bitmap, int xc, int yc, int r, int thickness, UINT32 color)
{
    if (!bitmap || (r <= 0))
    {
        return;
    }

    Gdiplus::BitmapData bmpData;
    Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());

    // GDI+ Bitmap�� ��� (���� ����)
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
            // Bresenham ������� �׵θ� ���� ���� �׸�
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
            // ������ r �������� �β��� ���� ���� ���� ������ ����� �ȼ� ä��
            // �β��� �ִ� �� �׵θ� �׸��� (�簢�� ���� �� ���� �� �ѷ��� ������� �˻�)
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

        bitmap->UnlockBits(&bmpData);  // ��� ����
    }
}

// ���ΰ� ä���� �� �׸��� �Լ�
// �߽�(xc, yc), ������ r�� �ش��ϴ� ���θ� ��� ä�� �׸��� �Լ�
// dx�� + dy�� <= r�� ������ �����ϴ� �ȼ��� ä��
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
