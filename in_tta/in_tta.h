// in_tta.h : main header file for the in_tta DLL
//
// $LastChangedDate$

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "TtaTag.h"
#include "ttadec.h"

#define MAX_PATHLEN		512
#define GENRES	148

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

