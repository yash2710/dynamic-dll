#include "StdAfx.h"
#include "DLLInjector.h"
#include <TlHelp32.h>
#include <Shlwapi.h>
#include "Common.h"

CDLLInjector::CDLLInjector() : m_hMapFile( NULL )
{
}

CDLLInjector::~CDLLInjector()
{
}

BOOL CDLLInjector::SetEnvironmentVariable( const DWORD dwProcessID, 
										   const CString& strVarName,
										   const CString& strVarVal,
										   BOOL bAddToExisting )
{
#ifdef UNICODE
	CreateAndCopyToShareMem( strVarName, strVarVal, bAddToExisting );
#else
	int nLenVar, nLenVal;
	nLenVar = strVarName.GetLength();

	if ( !nLenVar)
		return FALSE;

	nLenVal = strVarVal.GetLength();
	if ( !nLenVal )
		return FALSE;

	char* pTempVarBuff = new char[nLenVar+1];
	char* pTempValBuff = new char[nLenVal+1];
	mbstowcs( pTempVarBuff, strVarName, nLenVar );
	mbstowcs( pTempValBuff, strVarVal, nLenVal );
	CreateAndCopyToShareMem( pTempVarBuff, pTempValBuff );
	delete []pTempVarBuff;
	delete []pTempValBuff;
#endif

	InjectLibW( dwProcessID );
	FreeShareMem();
	EjectLibW( dwProcessID );
	return true;
}

// Update the user entered data to sharememory
BOOL CDLLInjector::CreateAndCopyToShareMem( LPCWSTR lpVarName, LPCWSTR lpVarVal, BOOL bAddToExisting )
{
	SharedData stData;

	int nLenVar = wcslen( lpVarName );
	if ( 0 == nLenVar || nLenVar >= _countof( stData.strVariable ))
	{
		AfxMessageBox( _T("Variable length is too high. Currently supports only 1024 chars" ));
		return FALSE;
	}

	LPWSTR pBuf;

	// prepare data for copying 
	wcscpy_s( stData.strVariable, _countof( stData.strVariable), lpVarName );
	wcscpy_s( stData.strValue, _countof( stData.strValue), lpVarVal );
	stData.bAddToExisting = bAddToExisting;

	m_hMapFile = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, 
									PAGE_READWRITE, 0, sizeof(stData),
									SHAREMEM_NAME );

	if ( m_hMapFile == NULL) 
	{ 
		MessageBox(0, _T("Could not create file mapping object"), 
			_T("Error"), MB_OK | MB_ICONERROR );
		return FALSE;
	}

	pBuf = (LPWSTR) MapViewOfFile( m_hMapFile, FILE_MAP_ALL_ACCESS, 
									0, 0, sizeof( stData ));

	if ( pBuf == NULL) 
	{ 
		MessageBox(0, _T("Could not map view of file"), 
			_T( "Error" ), MB_OK | MB_ICONERROR ); 

		CloseHandle(m_hMapFile);
		m_hMapFile = 0;
		return FALSE;
	}

	// Copy the data
	CopyMemory((PVOID)pBuf, &stData, sizeof( stData ));

	UnmapViewOfFile(pBuf);
	return TRUE;
}

// Function to inject the library
void CDLLInjector::InjectLibW( DWORD dwProcessId )
{
	HANDLE hProcess= NULL; // Process handle
	PWSTR pszLibFileRemote = NULL;
	HANDLE hRemoteThread = NULL; 
	__try
	{
		hProcess = OpenProcess(
			PROCESS_QUERY_INFORMATION |   
			PROCESS_CREATE_THREAD     | 
			PROCESS_VM_OPERATION |
			PROCESS_VM_WRITE,  // For CreateRemoteThread
			FALSE, dwProcessId);

		if( !hProcess )
		{
			AfxMessageBox( _T("Failed to update selected process") );
			__leave;
		}

		WCHAR szFilePath[MAX_PATH];
		GetModuleFileNameW( NULL, szFilePath, MAX_PATH );

		// Remove file name of the string
		PathRemoveFileSpecW( szFilePath );

		// Append the DLL file which is there in the same directory of exe
		LPCWSTR pszLib = L"\\SetEnvLib.dll";

		// Append the string
		wcscat_s( szFilePath, MAX_PATH, pszLib ); 

		int cch = 1 + lstrlenW(szFilePath);
		int cb  = cch * sizeof(WCHAR);

		// Allocate space in the remote process for the pathname
		pszLibFileRemote = (PWSTR) VirtualAllocEx( hProcess, 
			NULL, cb, 
			MEM_COMMIT, PAGE_READWRITE);
		if ( pszLibFileRemote == NULL) 
		{
			AfxMessageBox( _T("Unable to allocate memory") );
			return;
		}

		// Copy the DLL's pathname to the remote process' address space
		if (!WriteProcessMemory(hProcess, pszLibFileRemote,
			(PVOID) szFilePath, cb, NULL)) 
		{
			AfxMessageBox( _T( "Failed to write" ));
			return;
		};

		// Create remote thread and inject the library
		hRemoteThread = CreateRemoteThread( hProcess, NULL, 0, 
			(LPTHREAD_START_ROUTINE)LoadLibraryW, 
			pszLibFileRemote, NULL,NULL );

		if( !hRemoteThread )
		{
			AfxMessageBox( _T("Failed to update selected process") );
			__leave;
		}

		WaitForSingleObject( hRemoteThread, INFINITE );

		AfxMessageBox( _T("Successfully Set values"));
	}
	__finally // Do the cleanup
	{
		// Free the remote memory that contained the DLL's pathname
		if (pszLibFileRemote != NULL) 
			VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);

		if ( hRemoteThread != NULL) 
			CloseHandle(hRemoteThread);

		if ( hProcess != NULL) 
			CloseHandle(hProcess);
	}
}

// Eject the library loaded to remote process
BOOL CDLLInjector::EjectLibW( DWORD dwProcessID )
{
	BOOL bOk		   = FALSE; // Assume that the function fails
	HANDLE hthSnapshot = NULL;
	HANDLE hProcess = NULL, hThread = NULL;

	__try
	{
		// Grab a new snapshot of the process
		hthSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessID );
		if (hthSnapshot == INVALID_HANDLE_VALUE) __leave;

		// Get the HMODULE of the desired library
		MODULEENTRY32W me = { sizeof(me) };
		BOOL bFound = FALSE;
		BOOL bMoreMods = Module32FirstW(hthSnapshot, &me);

		// Iterate through all the loaded modules
		for (; bMoreMods; bMoreMods = Module32NextW(hthSnapshot, &me))
		{
			bFound = (_wcsicmp(me.szModule,  L"SetEnvLib.dll" ) == 0) || 
				(_wcsicmp(me.szExePath, L"SetEnvLib.dll" ) == 0);
			if (bFound) break;
		}
		if (!bFound) __leave;

		// Get a handle for the target process.
		hProcess = OpenProcess(
			PROCESS_QUERY_INFORMATION |   
			PROCESS_CREATE_THREAD     | 
			PROCESS_VM_OPERATION,  // For CreateRemoteThread
			FALSE, dwProcessID);
		if (hProcess == NULL) __leave;

		// Get the address of FreeLibrary in Kernel32.dll
		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
			GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "FreeLibrary");
		if (pfnThreadRtn == NULL) __leave;

		// Create a remote thread that calls FreeLibrary()
		hThread = CreateRemoteThread(hProcess, NULL, 0, 
			pfnThreadRtn, me.modBaseAddr, 0, NULL);
		if (hThread == NULL) __leave;

		// Wait for the remote thread to terminate
		WaitForSingleObject(hThread, INFINITE);

		bOk = TRUE; // Everything executed successfully
	}
	__finally 
	{
		if (hthSnapshot != NULL) 
			CloseHandle(hthSnapshot);

		if (hThread     != NULL) 
			CloseHandle(hThread);

		if (hProcess    != NULL) 
			CloseHandle(hProcess);
	}

	return bOk;
}