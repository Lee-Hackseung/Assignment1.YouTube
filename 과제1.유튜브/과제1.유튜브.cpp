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
#include "MISC_GUI_UTIL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(C과제1유튜브App, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

C과제1유튜브App theApp;

C과제1유튜브App::C과제1유튜브App()
{
}

BOOL C과제1유튜브App::InitInstance()
{
	CWinApp::InitInstance();


	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));    

	if (misc_gui_util_init())
	{
		C과제1유튜브Dlg* dlg;

		dlg = new C과제1유튜브Dlg;

		if (!dlg)
		{
			AfxMessageBox(_T("C과제1유튜브App 다이얼로그 생성 실패"));

			return FALSE;
		}

		m_pMainWnd = dlg;
		dlg->DoModal();

		delete dlg;

		misc_gui_util_term();
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	return FALSE;
}

