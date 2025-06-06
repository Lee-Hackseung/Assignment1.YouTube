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

#pragma once
#include "afxdialogex.h"
#include <vector>
#include <thread>

class CCanvasDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CCanvasDialog)

public:
	CCanvasDialog(CWnd* pParent = nullptr);	
	virtual ~CCanvasDialog();		
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMessageRandomDraw(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	void InitCanvas();
	void SetClickedCircleRadius(int radius);
	void SetCircletTickness(int thickness);
	bool RunRandomDraw();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CANVAS_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

private:
	int clickedPointCount;
	int clickedCircleRadius;
	int circleThickness;
	std::vector<CPoint> dots;
	Gdiplus::Graphics* bgBufferGraphics;
	Gdiplus::Bitmap* bgBufferBitmap;
	Gdiplus::Font* bgFont;
	std::thread randomDrawThread;
	bool stopRandomDraw;
	int draggingCircleIndex;

	void PublishBitmapToForeground(CPaintDC* pDC, Gdiplus::Bitmap* pBitmap, int x, int y);
	bool CalCircleFrom3Points(
		double x1, double y1,
		double x2, double y2,
		double x3, double y3,
		double* cx, double* cy, double* r);
	void RunRandomDrawThread();
	void MakeRandomPositions(CRect& rect);
	int GetCickedCircleIndex(CPoint& point);
	void DrawCircles(Gdiplus::Graphics* graphics, Gdiplus::Font* font);	
};
