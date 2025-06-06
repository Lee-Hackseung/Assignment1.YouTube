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
#include "framework.h"
#include "과제1.유튜브.h"
#include "과제1.유튜브Dlg.h"
#include "afxdialogex.h"

#define CLICKED_CIRCLE_RADIUS_DEFAULT 3
#define CIRCLE_THUCKNESS_DEFAULT      1
#define COMBO_WIDTH                   80  // 콤보박스 너비 상수화
#define COMBO_HEIGHT                  300 // 콤보박스 높이 상수화

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


C과제1유튜브Dlg::C과제1유튜브Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_APP_DIALOG, pParent)
{
	canvasDialog = nullptr;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

C과제1유튜브Dlg::~C과제1유튜브Dlg()
{
	if (canvasDialog)
	{
		delete canvasDialog;
		canvasDialog = nullptr;
	}
}

void C과제1유튜브Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXIT_BTN, exitBtn);
	DDX_Control(pDX, IDC_CLICKED_CIRCLE_COMBO, clickedCircleRadiusCombo);
	DDX_Control(pDX, IDC_CIRCLE_TH_COMBO, circleThCombo);
}

BEGIN_MESSAGE_MAP(C과제1유튜브Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()	
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_EXIT_BTN, &C과제1유튜브Dlg::OnBnClickedExitBtn)
	ON_BN_CLICKED(IDC_INIT_MFCBUTTON, &C과제1유튜브Dlg::OnBnClickedInit)
	ON_BN_CLICKED(IDC_RANDOM_MFCBUTTON, &C과제1유튜브Dlg::OnBnClickedRandomMove)
	ON_CBN_SELCHANGE(IDC_CLICKED_CIRCLE_COMBO, &C과제1유튜브Dlg::OnCbnSelchangeClickedCircleRadiusCombo)
	ON_CBN_SELCHANGE(IDC_CIRCLE_TH_COMBO, &C과제1유튜브Dlg::OnCbnSelchangeCircleThicknessCombo)
END_MESSAGE_MAP()


BOOL C과제1유튜브Dlg::OnInitDialog()
{
	int screenWidth;
	int screenHeight;	
	CRect parentRect;	

	CDialogEx::OnInitDialog();

	// 전체 화면 크기로 다이얼로그 설정
	screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

	::MoveWindow(this->GetSafeHwnd(), 0, 0, screenWidth, screenHeight, TRUE);

	GetWindowRect(parentRect);
	AttatchCanvas(parentRect); // 캔버스 창 생성 및 부착
	InitCircleComboBox();      // 콤보 박스 초기화

	// 기본 반지름 및 두께 설정
	canvasDialog->SetClickedCircleRadius(CLICKED_CIRCLE_RADIUS_DEFAULT);
	canvasDialog->SetCircletTickness(CIRCLE_THUCKNESS_DEFAULT);

	return TRUE;
}

// 종료 버튼 클릭 시 종료
void C과제1유튜브Dlg::OnBnClickedExitBtn()
{
	OnOK();
}

// 캔버스 다이얼로그 생성 및 위치 지정
void C과제1유튜브Dlg::AttatchCanvas(CRect& parentRect)
{
	CRect canvaseRect;
	int margin;	

	GetDlgItem(IDC_CANVAS_STATIC)->GetWindowRect(&canvaseRect);
	margin = canvaseRect.left;

	canvasDialog = new CCanvasDialog(this);

	canvaseRect.bottom = parentRect.bottom - margin;
	canvaseRect.right = parentRect.right - margin;

	canvasDialog->Create(IDD_CANVAS_DIALOG, this);
	canvasDialog->MoveWindow(&canvaseRect);
	canvasDialog->ShowWindow(SW_SHOW);
}

// 배경색 회색으로 채움
BOOL C과제1유튜브Dlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;

	GetClientRect(rect);
	pDC->FillSolidRect(rect, RGB(0x7f, 0x7f, 0x7f));

	return TRUE;
}

// 콤보박스 항목 초기화 및 위치 조정
void C과제1유튜브Dlg::InitCircleComboBox()
{
	CRect rect;

	for (int i = 0; i < 15; ++i)
	{
		CString wstr;

		wstr.Format(_T(" %d"), CLICKED_CIRCLE_RADIUS_DEFAULT + i);
		clickedCircleRadiusCombo.AddString(wstr);

		wstr.Format(_T(" %d"), CIRCLE_THUCKNESS_DEFAULT + i);
		circleThCombo.AddString(wstr);
	}
	
	clickedCircleRadiusCombo.GetWindowRect(&rect);
	ScreenToClient(&rect);
	clickedCircleRadiusCombo.MoveWindow(rect.left, rect.top, COMBO_WIDTH, COMBO_HEIGHT);
	clickedCircleRadiusCombo.SetCurSel(0);
	
	circleThCombo.GetWindowRect(&rect);
	ScreenToClient(&rect);
	circleThCombo.MoveWindow(rect.left, rect.top, COMBO_WIDTH, COMBO_HEIGHT);
	circleThCombo.SetCurSel(0);
}

// ESC/Enter 키 무시 처리
BOOL C과제1유튜브Dlg::PreTranslateMessage(MSG* pMsg)
{
	if (WM_KEYDOWN == pMsg->message)
	{
		switch (pMsg->wParam)
		{
		case VK_ESCAPE:
		case VK_RETURN:
			return TRUE; // 키 입력 무시
		default:
			break;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

// 초기화 버튼 처리
void C과제1유튜브Dlg::OnBnClickedInit()
{
	if (canvasDialog != nullptr)
	{
		canvasDialog->InitCanvas();
	}
}

// 랜덤 이동 버튼 처리
void C과제1유튜브Dlg::OnBnClickedRandomMove()
{
	if (canvasDialog != nullptr)
	{
		// RunRandomDraw()는 이미 랜덤 드로잉 쓰레드가 실행 중인 경우 false를 반환
		if (!canvasDialog->RunRandomDraw())
		{
			AfxMessageBox(_T("랜덤 이동을 진행 중 입니다"));
		}
	}
}

// 석택점 반지름 콤보 선택 변경 시 처리
void C과제1유튜브Dlg::OnCbnSelchangeClickedCircleRadiusCombo()
{
	CString wstr;
	int radius;

	clickedCircleRadiusCombo.GetLBText(clickedCircleRadiusCombo.GetCurSel(), wstr);

	radius = _wtoi(wstr);

	canvasDialog->SetClickedCircleRadius(_wtoi(wstr));
}

// 정원 테두리 두께 콤보 선택 변경 시
void C과제1유튜브Dlg::OnCbnSelchangeCircleThicknessCombo()
{
	CString wstr;
	int thickness;

	circleThCombo.GetLBText(circleThCombo.GetCurSel(), wstr);

	thickness = _wtoi(wstr);

	canvasDialog->SetCircletTickness(thickness);
}
