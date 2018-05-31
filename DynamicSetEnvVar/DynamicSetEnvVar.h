/********************************************************************
	Created :	30-12-2008
	Filename: 	DynamicSetEnvVar.h
	Author  :	Sarath C
	Purpose :	Application class Header file
*********************************************************************/

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CDynamicSetEnvVarApp:
// See DynamicSetEnvVar.cpp for the implementation of this class
//

class CDynamicSetEnvVarApp : public CWinApp
{
public:
	CDynamicSetEnvVarApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CDynamicSetEnvVarApp theApp;