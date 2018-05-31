/********************************************************************
	Created :	30-12-2008
	Filename: 	DLLInjector.h
	Author  :	Sarath C
	Purpose :	Class to inject/eject DLL and IPC
*********************************************************************/

#pragma once

class CDLLInjector
{
public:
	CDLLInjector();
	~CDLLInjector();
	BOOL SetEnvironmentVariable( const DWORD dwProcessID,
								 const CString& strVarName,
								 const CString& strVarVal,
								 BOOL bAddToExisting );

private:

	BOOL CreateAndCopyToShareMem( LPCWSTR lpVarName, LPCWSTR lpVarVal, BOOL bAddToExisting );

	void FreeShareMem()
	{
		if( m_hMapFile )
		{
			CloseHandle(m_hMapFile);
			m_hMapFile = NULL;
		}
	}

	void InjectLibW( DWORD dwProcessID );
	BOOL EjectLibW( DWORD dwProcessID );

private:
	HANDLE m_hMapFile;
};
