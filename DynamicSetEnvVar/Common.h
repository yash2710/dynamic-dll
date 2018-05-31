/********************************************************************
	Created :	30-12-2008
	Filename: 	Common.h
	Author  :	Sarath C
	Purpose :	Common header file shared with DLL and main application.
				Defines shared data items
*********************************************************************/
#pragma once

// Structure to share data
#pragma pack( push )
#pragma pack( 4 )
struct SharedData
{
	BOOL bAddToExisting;
	WCHAR strVariable[1024];
	WCHAR strValue[1024];

	SharedData()
	{
		ZeroMemory( strVariable, sizeof( strVariable ));
		ZeroMemory( strValue, sizeof( strValue ));
		bAddToExisting = TRUE;
	}
};

TCHAR SHAREMEM_NAME[]= _T("EnvCopy");

#pragma pack( pop )