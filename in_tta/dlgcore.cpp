// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


//#include "stdafx.h"

#undef	_WIN32_WINNT
#undef	WINVER

#include "C:\Program Files\Microsoft Visual Studio 8\VC\atlmfc\src\mfc\stdafx.h"
#include "C:\Program Files\Microsoft Visual Studio 8\VC\atlmfc\src\mfc\occimpl.h"

#define new DEBUG_NEW



BOOL AFXAPI _AfxCompareClassName(HWND hWnd, LPCTSTR lpszClassName)
{
	ASSERT(::IsWindow(hWnd));
	TCHAR szTemp[32];
	::GetClassName(hWnd, szTemp, _countof(szTemp));
	return ::AfxInvariantStrICmp(szTemp, lpszClassName) == 0;
}




// for backward compatibility
BOOL CDialog::CreateIndirect(HGLOBAL hDialogTemplate, CWnd* pParentWnd)
{
	return CreateIndirect(hDialogTemplate, pParentWnd, NULL);
}

BOOL CDialog::CreateIndirect(HGLOBAL hDialogTemplate, CWnd* pParentWnd,
	HINSTANCE hInst)
{
	ASSERT(hDialogTemplate != NULL);

	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);
	BOOL bResult = CreateIndirect(lpDialogTemplate, pParentWnd, NULL, hInst);
	UnlockResource(hDialogTemplate);

	return bResult;
}

// for backward compatibility
BOOL CDialog::CreateIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd* pParentWnd,
	void* lpDialogInit)
{
	return CreateIndirect(lpDialogTemplate, pParentWnd, lpDialogInit, NULL);
}

BOOL CDialog::CreateIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd* pParentWnd,
	void* lpDialogInit, HINSTANCE hInst)
{
	ASSERT(lpDialogTemplate != NULL);

	if (pParentWnd == NULL)
		pParentWnd = AfxGetMainWnd();
	m_lpDialogInit = lpDialogInit;

	return CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);
}

BOOL CWnd::CreateDlg(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	// load resource
	LPCDLGTEMPLATE lpDialogTemplate = NULL;
	HGLOBAL hDialogTemplate = NULL;
	HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName, RT_DIALOG);
	HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
	hDialogTemplate = LoadResource(hInst, hResource);
	if (hDialogTemplate != NULL)
		lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);
	ASSERT(lpDialogTemplate != NULL);

	// create a modeless dialog
	BOOL bSuccess = CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);

	// free resource
	UnlockResource(hDialogTemplate);
	FreeResource(hDialogTemplate);

	return bSuccess;
}

BOOL CWnd::CreateDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate,
	CWnd* pParentWnd, HINSTANCE hInst)
{
	ASSERT(lpDialogTemplate != NULL);
	if (pParentWnd != NULL)
		ASSERT_VALID(pParentWnd);

	if(!hInst)
		hInst = AfxGetResourceHandle();

#ifndef _AFX_NO_OCC_SUPPORT
	_AFX_OCC_DIALOG_INFO occDialogInfo;
	COccManager* pOccManager = afxOccManager;
#endif

	HGLOBAL hTemplate = NULL;

	HWND hWnd = NULL;
#ifdef _DEBUG
	DWORD dwError = 0;
#endif

	TRY
	{
//		VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTLS_REG)); // for dll
//		AfxDeferRegisterClass(AFX_WNDCOMMCTLSNEW_REG);		// for dll

#ifndef _AFX_NO_OCC_SUPPORT
		// separately create OLE controls in the dialog template
		if (pOccManager != NULL)
		{
			if (!SetOccDialogInfo(&occDialogInfo))
				return FALSE;

			lpDialogTemplate = pOccManager->PreCreateDialog(&occDialogInfo,
				lpDialogTemplate);
		}

		if (lpDialogTemplate == NULL)
			return FALSE;
#endif //!_AFX_NO_OCC_SUPPORT

		// If no font specified, set the system font.
		CString strFace;
		WORD wSize = 0;
		BOOL bSetSysFont = !CDialogTemplate::GetFont(lpDialogTemplate, strFace,
			wSize);

		if (afxData.bWin95 && !bSetSysFont && GetSystemMetrics(SM_DBCSENABLED))
		{
			bSetSysFont = (strFace == _T("MS Shell Dlg"));
			if (bSetSysFont && (wSize == 8))
				wSize = 0;
		}

		if (bSetSysFont)
		{
			CDialogTemplate dlgTemp(lpDialogTemplate);
			dlgTemp.SetSystemFont(wSize);
			hTemplate = dlgTemp.Detach();
		}

		if (hTemplate != NULL)
			lpDialogTemplate = (DLGTEMPLATE*)GlobalLock(hTemplate);

		// setup for modal loop and creation
		m_nModalResult = -1;
		m_nFlags |= WF_CONTINUEMODAL;

		// create modeless dialog
		AfxHookWindowCreate(this);
		hWnd = ::CreateDialogIndirect(hInst, lpDialogTemplate,
			pParentWnd->GetSafeHwnd(), AfxDlgProc);
#ifdef _DEBUG
		dwError = ::GetLastError();
#endif
	}
	CATCH_ALL(e)
	{
		DELETE_EXCEPTION(e);
		m_nModalResult = -1;
	}
	END_CATCH_ALL

#ifndef _AFX_NO_OCC_SUPPORT
	if (pOccManager != NULL)
	{
		pOccManager->PostCreateDialog(&occDialogInfo);
		if (hWnd != NULL)
			SetOccDialogInfo(NULL);
	}
#endif //!_AFX_NO_OCC_SUPPORT

	if (!AfxUnhookWindowCreate())
		PostNcDestroy();        // cleanup if Create fails too soon

	// handle EndDialog calls during OnInitDialog
#ifdef _DEBUG
#ifndef _AFX_NO_OCC_SUPPORT
	DWORD dwOldFlags = m_nFlags;
#endif
#endif
	if (hWnd != NULL && !(m_nFlags & WF_CONTINUEMODAL))
	{
		::DestroyWindow(hWnd);
		hWnd = NULL;
	}

	if (hTemplate != NULL)
	{
		GlobalUnlock(hTemplate);
		GlobalFree(hTemplate);
	}

	// help with error diagnosis (only if WM_INITDIALOG didn't EndDialog())
	if (hWnd == NULL)
	{
#ifdef _DEBUG
#ifndef _AFX_NO_OCC_SUPPORT
		if (dwOldFlags & WF_CONTINUEMODAL)
		{
			if (afxOccManager == NULL)
			{
				TRACE(traceAppMsg, 0, ">>> If this dialog has OLE controls:\n");
				TRACE(traceAppMsg, 0, ">>> AfxEnableControlContainer has not been called yet.\n");
				TRACE(traceAppMsg, 0, ">>> You should call it in your app's InitInstance function.\n");
			}
			else if (dwError != 0)
			{
				TRACE(traceAppMsg, 0, "Warning: Dialog creation failed!  GetLastError returns 0x%8.8X\n", dwError);
			}
		}
#endif //!_AFX_NO_OCC_SUPPORT
#endif //_DEBUG
		return FALSE;
	}

	ASSERT(hWnd == m_hWnd);
	return TRUE;
}


INT_PTR CDialog::DoModal()
{
	// can be constructed with a resource template or InitModalIndirect
	ASSERT(m_lpszTemplateName != NULL || m_hDialogTemplate != NULL ||
		m_lpDialogTemplate != NULL);

	// load resource as necessary
	LPCDLGTEMPLATE lpDialogTemplate = m_lpDialogTemplate;
	HGLOBAL hDialogTemplate = m_hDialogTemplate;
	HINSTANCE hInst = AfxGetResourceHandle();
	if (m_lpszTemplateName != NULL)
	{
		hInst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
		HRSRC hResource = ::FindResource(hInst, m_lpszTemplateName, RT_DIALOG);
		hDialogTemplate = LoadResource(hInst, hResource);
	}
	if (hDialogTemplate != NULL)
		lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);

	// return -1 in case of failure to load the dialog template resource
	if (lpDialogTemplate == NULL)
		return -1;

	// disable parent (before creating dialog)
	HWND hWndParent = PreModal();
	AfxUnhookWindowCreate();
	BOOL bEnableParent = FALSE;
#ifndef _AFX_NO_OLE_SUPPORT
	CWnd* pMainWnd = NULL;
	BOOL bEnableMainWnd = FALSE;
#endif
	if (hWndParent && hWndParent != ::GetDesktopWindow() && ::IsWindowEnabled(hWndParent))
	{
		::EnableWindow(hWndParent, FALSE);
		bEnableParent = TRUE;
#ifndef _AFX_NO_OLE_SUPPORT
		pMainWnd = AfxGetMainWnd();
		if (pMainWnd && pMainWnd->IsFrameWnd() && pMainWnd->IsWindowEnabled())
		{
			//
			// We are hosted by non-MFC container
			// 
			pMainWnd->EnableWindow(FALSE);
			bEnableMainWnd = TRUE;
		}
#endif
	}

	TRY
	{
		// create modeless dialog
		AfxHookWindowCreate(this);
		if (CreateDlgIndirect(lpDialogTemplate,
					CWnd::FromHandle(hWndParent), hInst))
		{
			if (m_nFlags & WF_CONTINUEMODAL)
			{
				// enter modal loop
				DWORD dwFlags = MLF_SHOWONIDLE;
				if (GetStyle() & DS_NOIDLEMSG)
					dwFlags |= MLF_NOIDLEMSG;
				VERIFY(RunModalLoop(dwFlags) == m_nModalResult);
			}

			// hide the window before enabling the parent, etc.
			if (m_hWnd != NULL)
				SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|
					SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
		}
	}
	CATCH_ALL(e)
	{
		DELETE_EXCEPTION(e);
		m_nModalResult = -1;
	}
	END_CATCH_ALL

#ifndef _AFX_NO_OLE_SUPPORT
	if (bEnableMainWnd)
		pMainWnd->EnableWindow(TRUE);
#endif
	if (bEnableParent)
		::EnableWindow(hWndParent, TRUE);
	if (hWndParent != NULL && ::GetActiveWindow() == m_hWnd)
		::SetActiveWindow(hWndParent);

	// destroy modal window
	DestroyWindow();
	PostModal();

	// unlock/free resources as necessary
	if (m_lpszTemplateName != NULL || m_hDialogTemplate != NULL)
		UnlockResource(hDialogTemplate);
	if (m_lpszTemplateName != NULL)
		FreeResource(hDialogTemplate);

	return m_nModalResult;
}

