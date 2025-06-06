// CCanvasDialog.cpp: 구현 파일
//

#include "pch.h"
#include <mmsystem.h>
#include <random>
#include "과제1.유튜브.h"
#include "afxdialogex.h"
#include "CCanvasDialog.h"
#include "MISC_GUI_UTIL.h"

#pragma comment(lib, "winmm.lib")

#define CLICKED_3_POINTS                     3
#define RAMDOM_MOVE_COUNT	                 10
#define RAMDOM_MOVE_INTERVAL_TIME_MS	     500
#define RAMDOM_MOVE_CHECK_INTERVAL_TIME_MS   16
#define RES_1_MS	                         1
#define CHECK_RANDOM_DRAW_THREAD_TIMER_ID    0xcafe77
#define WM_CHECK_RANDOM_DRAW_THREAD_MESSAGE  (WM_USER + 1)
#define RANDOM_DRAW_START                    0
#define RANDOM_DRAW_STOP                     1
#define INVALID_CIRCLE_INDEX                -1

IMPLEMENT_DYNAMIC(CCanvasDialog, CDialogEx)

CCanvasDialog::CCanvasDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CANVAS_DIALOG, pParent)
{
	draggingCircleIndex = INVALID_CIRCLE_INDEX;
	clickedPointCount = 0;
	clickedCircleRadius = 0;
	circleThickness = 0;
	stopRandomDraw = true;
	bgBufferGraphics = nullptr;
	bgBufferBitmap = nullptr;
	bgFont = nullptr;

	for (int i = 0; i < CLICKED_3_POINTS; i++)
	{
		dots.push_back(CPoint());
	}
}

// - randomDrawThread: 실행 중이면 join() 호출로 안전하게 쓰레드 종료
// - bgBufferBitmap: 백 버퍼용 비트맵 객체
// - bgBufferGraphics: 비트맵에 그리기 위한 GDI+ 그래픽스 객체
// - bgFont: 텍스트 출력을 위한 폰트 객체
// - dots: 원을 구성하는 클릭된 점들 (vector)
CCanvasDialog::~CCanvasDialog()
{
	stopRandomDraw = true; // 쓰레드 루프 종료 신호

	if (randomDrawThread.joinable()) // 실행 중이면 차단 없이 종료 대기
	{
		randomDrawThread.join();
	}

	if (bgBufferBitmap)
	{
		delete bgBufferBitmap;
	}

	if (bgBufferGraphics)
	{
		delete bgBufferGraphics;
	}

	if (bgFont)
	{
		delete bgFont;
	}

	dots.clear();
}

void CCanvasDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCanvasDialog, CDialogEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_CHECK_RANDOM_DRAW_THREAD_MESSAGE, &CCanvasDialog::OnMessageRandomDraw)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// ESC/Enter 키 무시 처리
BOOL CCanvasDialog::PreTranslateMessage(MSG* pMsg)
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

BOOL CCanvasDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();		

	bgFont = new Gdiplus::Font (_T("Lucida Sans Typewriter"), 20, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

	return TRUE;
}

// 캔버스를 초기화하는 함수.
// 랜덤 드로잉 쓰레드가 이미 실행 중이면 중복 실행을 방지하기 위해 리턴
void CCanvasDialog::InitCanvas()
{
	if (randomDrawThread.joinable())
	{
		return;
	}

	clickedPointCount = 0;
	Invalidate();
}

// 세 점으로부터 하나의 원을 계산하는 함수.
// 입력: (x1, y1), (x2, y2), (x3, y3) - 평면상의 세 점 좌표
// 출력: cx, cy - 계산된 원의 중심 좌표, r - 반지름
// 수학적 원리는 세 점으로부터 외접원을 구성하는 기하학 공식 이용
// 삼각형 세 변의 수직 이등분선의 교점을 구하는 방식으로 중심 계산
// 반환값: 유효한 원이 생성되면 true, 세 점이 일직선일 경우 false
bool CCanvasDialog::CalCircleFrom3Points(
	double x1, double y1,
	double x2, double y2,
	double x3, double y3,
	double* cx, double* cy, double* r)
{
	double x1_sq;
	double x2_sq;
	double x3_sq;
	double A;
	double D;
	double ux;
	double uy;

	// 외접원 행렬식 계산 (면적 0이면 일직선)
	A = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2;

	if (fabs(A) < 1e-10)
	{ // 세 점이 일직선
		return false;
	}

	// 각 점의 제곱합 (x^2 + y^2)
	A = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2;
	x1_sq = x1 * x1 + y1 * y1;
	x2_sq = x2 * x2 + y2 * y2;
	x3_sq = x3 * x3 + y3 * y3;
	D = 2 * A;

	// 중심 좌표 계산 공식
	ux = (x1_sq * (y2 - y3) + x2_sq * (y3 - y1) + x3_sq * (y1 - y2)) / D;
	uy = (x1_sq * (x3 - x2) + x2_sq * (x1 - x3) + x3_sq * (x2 - x1)) / D;

	*cx = ux;
	*cy = uy;

	// 반지름 계산: 중심과 한 점 간의 거리
	*r = sqrt((ux - x1) * (ux - x1) + (uy - y1) * (uy - y1));

	return true;
}

// 백 버퍼에 그려진 비트맵을 실제 윈도우 DC에 출력.
// GetHBITMAP: GDI+ 비트맵을 GDI 호환 비트맵 핸들로 변환
// CreateCompatibleDC: 메모리 DC 생성
// SelectObject: 비트맵을 메모리 DC에 선택
// BitBlt: 메모리 DC의 비트맵을 실제 DC에 복사하여 출력
void CCanvasDialog::PublishBitmapToForeground(CPaintDC* pDC, Gdiplus::Bitmap* pBitmap, int x, int y)
{
	HDC hdcDest;
	HBITMAP hBitmap;
	HDC hMemDC;
	HGDIOBJ hOldBitmap;

	if (!pDC || !pBitmap)
	{
		return;
	}

	hdcDest = pDC->GetSafeHdc();
	pBitmap->GetHBITMAP(Gdiplus::Color::White, &hBitmap); // GDI+ -> GDI 비트맵 변환

	hMemDC = CreateCompatibleDC(hdcDest);                 // 메모리 DC 생성
	hOldBitmap = SelectObject(hMemDC, hBitmap);

	BitBlt(hdcDest, x, y, pBitmap->GetWidth(), pBitmap->GetHeight(), hMemDC, 0, 0, SRCCOPY);  // 실제 화면에 복사 출력

	SelectObject(hMemDC, hOldBitmap);                    // 원래 오브젝트 복원
	DeleteDC(hMemDC);                                    // 메모리 DC 해제
	DeleteObject(hBitmap);                               // 비트맵 객체 해제
}

// 다이얼로그에 그림을 그리는 주요.
// 먼저 백 버퍼용 GDI+ Bitmap과 Graphics 객체가 초기화되어 있는지 확인하고,
// 없으면 클라이언트 영역 크기에 맞게 새로 생성.
// 이후 도형을 백 버퍼에 그린 후, 최종적으로 화면 DC에 비트맵을 출력.
void CCanvasDialog::OnPaint()
{
	CPaintDC dc(this);

	if (!bgBufferBitmap)
	{
		CRect rectDraw;

		GetClientRect(rectDraw);

		bgBufferBitmap = new Gdiplus::Bitmap(rectDraw.Width(), rectDraw.Height());
		bgBufferGraphics = new Gdiplus::Graphics(bgBufferBitmap);

		bgBufferGraphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
		bgBufferGraphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	}

	DrawCircles(bgBufferGraphics, bgFont);
	PublishBitmapToForeground(&dc, bgBufferBitmap, 0, 0);
}

void CCanvasDialog::DrawCircles(Gdiplus::Graphics* graphics, Gdiplus::Font* font)
{
	if (!graphics || !bgBufferBitmap)
	{
		return;
	}

	graphics->Clear(Gdiplus::Color(0xff, 0xff, 0xff, 0xff));

	if (CLICKED_3_POINTS == clickedPointCount)
	{
		double x1 = static_cast<double>(dots[0].x);
		double y1 = static_cast<double>(dots[0].y);
		double x2 = static_cast<double>(dots[1].x);
		double y2 = static_cast<double>(dots[1].y);
		double x3 = static_cast<double>(dots[2].x);;
		double y3 = static_cast<double>(dots[2].y);
		double radius = 0.0f;
		double cx = 0.0f;
		double cy = 0.0f;
		CString wstr;

		if (!CalCircleFrom3Points(x1, y1, x2, y2, x3, y3, &cx, &cy, &radius))
		{
			AfxMessageBox(_T("세 점은 직선 입니다."));

			return;
		}

		misc_util_draw_circle(bgBufferBitmap, static_cast<int>(cx), static_cast<int>(cy), static_cast<int>(radius), circleThickness, 0xff000000);

		Gdiplus::SolidBrush brush(Gdiplus::Color(0xff, 0x00, 0x00, 0xff));

		wstr.Format(_T("정원 중심 좌표(픽셀): %.02f, %.02f"), cx, cy);
		graphics->DrawString(wstr, -1, font, Gdiplus::PointF(100.0f, 100.0f), &brush);

		wstr.Format(_T("정원 반지름(픽셀): %.02f"), radius);
		graphics->DrawString(wstr, -1, font, Gdiplus::PointF(100.0f, 150.0f), &brush);
	}

	for (int i = 0; i < clickedPointCount; ++i)
	{
		int cx;
		int cy;
		int radius;
		CString wstr;
		UINT32 cicrcleColor;

		cx = dots[i].x;
		cy = dots[i].y;
		radius = clickedCircleRadius;

		if (draggingCircleIndex == i)
		{
			cicrcleColor = 0xff0000ff;
		}
		else
		{
			cicrcleColor = 0xffff0000;
		}

		misc_util_draw_fill_circle(bgBufferBitmap, cx, cy, radius, cicrcleColor);

		Gdiplus::PointF ptf(static_cast<Gdiplus::REAL>(cx), static_cast<Gdiplus::REAL>(cy));
		Gdiplus::SolidBrush brush(Gdiplus::Color(0xff, 0xff, 0x00, 0x00));
		
		wstr.Format(_T("  %d,%d"), cx + radius, cy + radius);		
		graphics->DrawString(wstr, -1, font, ptf, &brush);
	}
}

BOOL CCanvasDialog::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CCanvasDialog::SetClickedCircleRadius(int radius)
{
	clickedCircleRadius = radius;
	Invalidate();
}

void CCanvasDialog::SetCircletTickness(int thickness)
{
	circleThickness = thickness;
	Invalidate();
}

bool CCanvasDialog::RunRandomDraw()
{
	if (randomDrawThread.joinable())
	{
		return false;
	}

	randomDrawThread = std::thread(&CCanvasDialog::RunRandomDrawThread, this);

	return true;
}

void CCanvasDialog::MakeRandomPositions(CRect& rect)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distX(rect.left + 1, rect.right - 1);
	std::uniform_int_distribution<int> distY(rect.top + 1, rect.bottom - 1);

	for (int i = 0; i < CLICKED_3_POINTS; ++i)
	{
		dots[i].x = distX(gen);
		dots[i].y = distY(gen);
	}

	clickedPointCount = CLICKED_3_POINTS;
}

void CCanvasDialog::RunRandomDrawThread()
{	
	int count;	

	stopRandomDraw = false;
	count = 0;

	timeBeginPeriod(RES_1_MS);

	do
	{
		ULONG64 startTick64;

		startTick64 = GetTickCount64();

		PostMessage(WM_CHECK_RANDOM_DRAW_THREAD_MESSAGE, RANDOM_DRAW_START, 0);

		do
		{
			Sleep(RES_1_MS);

			if (stopRandomDraw)
			{
				return;
			}
		} while (RAMDOM_MOVE_INTERVAL_TIME_MS > (GetTickCount64() - startTick64));
		
	} while (RAMDOM_MOVE_COUNT > ++count);

	timeEndPeriod(RES_1_MS);

	PostMessage(WM_CHECK_RANDOM_DRAW_THREAD_MESSAGE, RANDOM_DRAW_STOP, 0);
}


LRESULT CCanvasDialog::OnMessageRandomDraw(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case RANDOM_DRAW_START:
	{
		CRect rect;

		GetClientRect(rect);

		MakeRandomPositions(rect);
		Invalidate();
	}		
		break;
	case RANDOM_DRAW_STOP:
		if (randomDrawThread.joinable())
		{
			randomDrawThread.join();
		}

		break;
	}	

	return 0;
}

void CCanvasDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (randomDrawThread.joinable())
	{
		return;
	}

	if (CLICKED_3_POINTS > clickedPointCount)
	{
		dots[clickedPointCount].x = point.x;
		dots[clickedPointCount].y = point.y;

		clickedPointCount++;

		Invalidate();
	}

	draggingCircleIndex = GetCickedCircleIndex(point);

	if (INVALID_CIRCLE_INDEX == draggingCircleIndex)
	{
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	}
	else
	{
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
	}
}

void CCanvasDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	draggingCircleIndex = INVALID_CIRCLE_INDEX;
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	ReleaseCapture();
	Invalidate();
}

void CCanvasDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	int circleIndex;
	static UINT64 _draw_skip = 0;

	circleIndex = draggingCircleIndex;

	if (INVALID_CIRCLE_INDEX == circleIndex)
	{
		return;
	}

	dots[circleIndex].x = point.x;
	dots[circleIndex].y = point.y;

	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
	SetCapture();
	Invalidate();
}

int CCanvasDialog::GetCickedCircleIndex(CPoint& point)
{
	for (int i = 0; i < clickedPointCount; ++i)
	{
		double distance;
		int dx;
		int dy;
		int cx = dots[i].x;
		int cy = dots[i].y;
		

		dx = abs(point.x - cx);
		dy = abs(point.y - cy);
		distance = std::sqrt(dx * dx + dy * dy);

		if (clickedCircleRadius >= distance)
		{
			return i;
		}
	}

	return INVALID_CIRCLE_INDEX;
}
