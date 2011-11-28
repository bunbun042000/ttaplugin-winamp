// in_tta.h : main header file for the in_tta DLL
//
// $LastChangedDate$

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#define TTA_VERSION "3.0"

// Cin_ttaApp
// See in_tta.cpp for the implementation of this class
//

class Cin_ttaApp : public CWinApp
{
public:
	Cin_ttaApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

