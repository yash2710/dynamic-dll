// DynamicSetEnvVarDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DynamicSetEnvVar.h"
#include "DynamicSetEnvVarDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CDynamicSetEnvVarDlg dialog




CDynamicSetEnvVarDlg::CDynamicSetEnvVarDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDynamicSetEnvVarDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDynamicSetEnvVarDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PROCESS, m_ProcessListCtrl);
	DDX_Control(pDX, IDC_CHECK_ADDEXISTING, m_CheckAddExisting);
}

BEGIN_MESSAGE_MAP(CDynamicSetEnvVarDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PROCESS, &CDynamicSetEnvVarDlg::OnChangedListProcess)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_ID, &CDynamicSetEnvVarDlg::OnBnClickedButtonSearchId)
	ON_BN_CLICKED(IDC_BUTTON_SETVAR, &CDynamicSetEnvVarDlg::OnBnClickedButtonSetvar)
	ON_BN_CLICKED(IDC_BUTTON2, &CDynamicSetEnvVarDlg::OnBnClickedButtonRefresh)
END_MESSAGE_MAP()


// CDynamicSetEnvVarDlg message handlers

BOOL CDynamicSetEnvVarDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CRect rect;
	m_ProcessListCtrl.GetClientRect( rect );
	m_ProcessListCtrl.InsertColumn( 0, _T("Process Name" ), 0, 200 );
	m_ProcessListCtrl.InsertColumn( 1, _T("PID" ), 0, rect.Width()-220 );

	m_ProcessListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_ONECLICKACTIVATE);

	m_CheckAddExisting.SetCheck( BST_CHECKED );
	RefreshProcessList();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDynamicSetEnvVarDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDynamicSetEnvVarDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDynamicSetEnvVarDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDynamicSetEnvVarDlg::OnChangedListProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if( LVIF_STATE == pNMLV->uChanged )
	{
		if( LVIS_SELECTED == m_ProcessListCtrl.GetItemState( pNMLV->iItem, LVIS_SELECTED ))
		{
			CString strMsg = m_ProcessListCtrl.GetItemText( pNMLV->iItem, 1 );
			LONG lPiD = _tstol( strMsg );
			DisplayProcessInfo( lPiD );
		}
	}
	
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CDynamicSetEnvVarDlg::DisplayProcessInfo( DWORD dwProcessID )
{
	CString strFormat;

	ProcessInfoMap::const_iterator iter = m_ProcessInfoMap.find( dwProcessID );
	if ( m_ProcessInfoMap.end() != iter )
	{
		const PROCESSENTRY32& pe32 = (*iter).second;//m_ProcessInfoMap[dwProcessID];

		ProcessInfoMap::const_iterator iterParent = m_ProcessInfoMap.find( pe32.th32ParentProcessID );

		TCHAR buff[MAX_PATH] = { 0 };
		if ( iterParent == m_ProcessInfoMap.end())
  		{
			_tcscpy_s( buff, MAX_PATH, _T("Unknown"));
  		}
		else
		{
			_tcscpy_s( buff, MAX_PATH, iterParent->second.szExeFile );
		}

		strFormat.Format( _T("Name: \t\t\t%s\n\n" )
			_T( "Process ID: \t\t%d \n\n" )
			_T( "Parent Process ID: \t%d \n\n" )
			_T( "Parent Process Name: \t%s \n\n" )
			_T( "Threads Count: \t\t%d\n" ),
			pe32.szExeFile, pe32.th32ProcessID,
			pe32.th32ParentProcessID, buff,
			pe32.cntThreads,
			pe32.th32DefaultHeapID, pe32.th32ModuleID );

		SetDlgItemInt( IDC_EDIT_PROCID, pe32.th32ProcessID );
	}
// 	else
// 	{
// 		strFormat = _T( "Invalid Process" );
// 	}
	SetDlgItemText( IDC_STATIC_PINFO, strFormat );
}
void CDynamicSetEnvVarDlg::AddProcessInfoToList( int nIndex, const PROCESSENTRY32& pe32 )
{
	// Insert into the map
	m_ProcessInfoMap[pe32.th32ProcessID] = pe32; // Copy it to the map

	CString strFormat;
	m_ProcessListCtrl.InsertItem( nIndex, pe32.szExeFile );
	strFormat.Format( _T( "%d" ), pe32.th32ProcessID );
	m_ProcessListCtrl.SetItemText( nIndex, 1, strFormat );
}
void CDynamicSetEnvVarDlg::RefreshProcessList()
{
	m_ProcessListCtrl.DeleteAllItems();
	m_ProcessInfoMap.clear();
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		MessageBox( _T( "Failed to query processes" ));
		return;
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process, and exit if unsuccessful
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		CloseHandle( hProcessSnap );    // Must clean up the
		//   snapshot object!
		return;
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	CString csProcessInfo;
	int i = 0;
	do
	{
		AddProcessInfoToList( i++, pe32 );

	} while( Process32Next( hProcessSnap, &pe32 ) );

	CloseHandle( hProcessSnap );
}
void CDynamicSetEnvVarDlg::ResetControlContents()
{
	
}

void CDynamicSetEnvVarDlg::OnBnClickedButtonSearchId()
{
	LVFINDINFO stFindInfo;
	CString strPID;
	GetDlgItemText( IDC_EDIT_SEARCH_NAME,strPID );
	stFindInfo.psz = strPID; 
	stFindInfo.flags = LVFI_STRING | LVFI_PARTIAL;
	stFindInfo.vkDirection = VK_END;
	int nIdx = m_ProcessListCtrl.FindItem( &stFindInfo );
	if (nIdx != -1 )
	{
		m_ProcessListCtrl.SetItemState( nIdx, LVIS_SELECTED, LVIS_SELECTED );
		GetDlgItem( IDC_EDIT_VARNAME )->SetFocus();
	}
	else
	{
		MessageBox( _T( "Process ID not found" ));
	}
}

void CDynamicSetEnvVarDlg::OnBnClickedButtonSetvar()
{
	CString strProcID;
	CString strMsg;
	GetDlgItemText(IDC_EDIT_PROCID, strProcID );
	if( !strProcID.GetLength())
	{
		MessageBox( _T("Select a valid process"), _T("Error"), MB_OK | MB_ICONERROR );
		return;
	}
	
	CString strVar, strVal;
	UINT uProcessID = GetDlgItemInt( IDC_EDIT_PROCID );
	GetDlgItemText( IDC_EDIT_VARNAME, strVar );

	if ( !strVar.GetLength())
	{
		MessageBox( _T("Enter a variable name and value"), _T("Error"), MB_OK | MB_ICONERROR );
		return;
	}

	GetDlgItemText( IDC_EDIT_VARVAL, strVal );

	BOOL bChecked =  ( BST_CHECKED == m_CheckAddExisting.GetCheck()) ? TRUE : FALSE;

	int nLen = strVal.GetLength();
	if(  nLen == 0 )
	{
		if( !bChecked )
		{
			strMsg.LoadString( IDS_STRING_DELETE_WARN );
			if( IDNO == MessageBox( strMsg,_T("Confirm"), MB_YESNO| MB_ICONQUESTION ))
				return;
		}
		else
		{
			strMsg.LoadString( IDS_STRING_EMPTYVAL_WARN );
			MessageBox( strMsg ,_T("Information"), MB_OK| MB_ICONINFORMATION );			
			return;
		}
	}

	strMsg.LoadString( IDS_STRING_FINAL_WARN );
	int nRet = MessageBox( strMsg ,_T("Confirm"), MB_YESNO| MB_ICONQUESTION );

	if ( nRet == IDYES )
	{
		m_Inject.SetEnvironmentVariable( uProcessID, strVar, strVal, bChecked );
	}
}

void CDynamicSetEnvVarDlg::OnBnClickedButtonRefresh()
{
	m_ProcessListCtrl.SetRedraw( FALSE );
	RefreshProcessList();
	m_ProcessListCtrl.SetRedraw( TRUE );
	m_ProcessListCtrl.RedrawItems(0,m_ProcessListCtrl.GetItemCount()-1);
	SetDlgItemText( IDC_STATIC_PINFO, _T("Select Any process" ));
	SetDlgItemText( IDC_EDIT_PROCID, _T(""));
}
