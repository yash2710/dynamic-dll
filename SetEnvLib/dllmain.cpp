/********************************************************************
	Created :	30-12-2008
	Filename: 	dllmain.cpp
	Author  :	Sarath C
	Purpose :	DLL Enry point and other main functions
*********************************************************************/

#include "stdafx.h"
#include <tchar.h>
#include <string>
#include "../DynamicSetEnvVar/Common.h"

// Function which reads and updates shared memory
BOOL UpdateEnvVar()
{
	HANDLE hMapFile = 0;
	SharedData* pShared = 0;
	hMapFile = OpenFileMapping( FILE_MAP_READ, FALSE, SHAREMEM_NAME );               // name of mapping object 

	if (hMapFile == NULL) 
	{ 
		OutputDebugString(TEXT("Could not open file mapping object"));
		return FALSE;
	} 

	pShared = (SharedData*) MapViewOfFile(hMapFile, // handle to map object
		FILE_MAP_READ, 0, 0, sizeof( SharedData ));                   

	if (pShared == NULL) 
	{ 
		OutputDebugString(TEXT("Could not map view of file")); 
		return FALSE;
	}

	if( !pShared->bAddToExisting )
	{
		if( wcslen( pShared->strValue ))
		{
			SetEnvironmentVariableW( pShared->strVariable, pShared->strValue);
		}
		else
		{
			// Delete variable
			SetEnvironmentVariableW( pShared->strVariable, NULL );
		}
	}
	else
	{
		// Get the required size
		const DWORD dwReturn = GetEnvironmentVariable( pShared->strVariable, 0, 0 );
		const DWORD dwErr = GetLastError();

		if( 0 ==  dwReturn && ERROR_ENVVAR_NOT_FOUND == dwErr ) // Variable not found
		{
			// Set the new one
			SetEnvironmentVariableW( pShared->strVariable, pShared->strValue);
		}
		else if( dwReturn > 0 )
		{
			WCHAR* pstrExisting = new WCHAR[1024];
			if( 0 == GetEnvironmentVariable( pShared->strVariable, 
				pstrExisting, dwReturn ) &&
				GetLastError() == 0 )
			{
				std::wstring strNew( pstrExisting );
				strNew += L";";
				strNew += pShared->strValue;
				SetEnvironmentVariableW( pShared->strVariable, strNew.c_str());
			}
		}
	}
	

	if( pShared )
		UnmapViewOfFile(pShared);
	if( hMapFile )
		CloseHandle(hMapFile);
	return TRUE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			TCHAR buff[MAX_PATH] = { 0 };
			_stprintf_s( buff, _T( "Attached Process: %d" ), GetCurrentProcessId());
			OutputDebugString( buff );
			UpdateEnvVar();
		}

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

