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

#include "CCanvasDialog.h"
#include "resource.h"

class CAssignment1Dlg : public CDialogEx
{
public:
	CAssignment1Dlg(CWnd* pParent = nullptr);
	virtual ~CAssignment1Dlg();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedExitBtn();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);	
	afx_msg void OnBnClickedInit();
	afx_msg void OnBnClickedRandomMove();
	afx_msg void OnCbnSelchangeClickedCircleRadiusCombo();
	afx_msg void OnCbnSelchangeCircleThicknessCombo();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_APP_DIALOG };
#endif

protected:
	HICON m_hIcon;
	virtual void DoDataExchange(CDataExchange* pDX);	
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	
private:
	CMFCButton exitBtn;
	CCanvasDialog* canvasDialog;
	CComboBox clickedCircleRadiusCombo;
	CComboBox circleThCombo;
	
	void AttatchCanvas(CRect& parentRect);
	void InitCircleComboBox();	
};
