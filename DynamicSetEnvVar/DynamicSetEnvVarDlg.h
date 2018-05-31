/********************************************************************
	Created :	30-12-2008
	Filename: 	DynamicSetEnvVarDlg.h
	Author  :	Sarath C
	Purpose :	Main Dlg Header file
*********************************************************************/

#pragma once
#include "afxcmn.h"
#include <TlHelp32.h>
#include <map>
#include "DLLInjector.h"
#include "afxwin.h"

// CDynamicSetEnvVarDlg dialog
class CDynamicSetEnvVarDlg : public CDialog
{
// Construction
public:
	CDynamicSetEnvVarDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DYNAMICSETENVVAR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public: // Message Handlers
	afx_msg void OnChangedListProcess(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonSearchId();
	afx_msg void OnBnClickedButtonSetvar();
	afx_msg void OnBnClickedButtonRefresh();

public: // Functions
	void RefreshProcessList();
	void ResetControlContents();
	void DisplayProcessInfo( DWORD dwProcessID );
	void AddProcessInfoToList( int nIndex, const PROCESSENTRY32& pe32 );

private:
	CListCtrl m_ProcessListCtrl;
	CButton m_CheckAddExisting;

	typedef std::map<DWORD, PROCESSENTRY32> ProcessInfoMap;
	ProcessInfoMap m_ProcessInfoMap;
	CDLLInjector m_Inject;
};
