// TechnicalTestDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "TechnicalTest.h"
#include "TechnicalTestDlg.h"
#include "afxdialogex.h"

#include "DataConversion.h"
#include "Logger.h"
#include "FileOperations.h"
#include "FilenameHelpers.h"
#include "ConfigurationManager.h"

// CAboutDlg dialog used for App About

struct sCleanupInfo
{
	HWND hWndParent;
	Logger* pLogger;
	CTechnicalTestApp* pApp;
};

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTechnicalTestDlg dialog

UINT CleanUpThread(void* pointer)
{
	CString strMess;
	sCleanupInfo* pInfo = (sCleanupInfo*)pointer;
	CWaitCursor wait;

	COleDateTime dtNow = COleDateTime::GetCurrentTime();

	HRESULT res = ::CoInitializeEx(0, COINIT_APARTMENTTHREADED);

	pInfo->pLogger->AddToLogByString(_T("Cleanup thread started"));

	pInfo->pLogger->AddToLogByString(_T("Moving example files"));

	for (int i = 0; i < ConfigManager::arrMoveConfig.GetCount(); i++) {
		auto& cfg = ConfigManager::arrMoveConfig[i];

		CStringArray aryFilesToMove;
		FileOperations::ListAllFiles(pInfo->pLogger, cfg.strFolderPath + L'*.' + cfg.strFileExtensions, &aryFilesToMove, cfg.bIncludeDirs && !cfg.bIncludeFiles, false, cfg.bIncludeDirs && cfg.bIncludeFiles);

		for (int nFile = 0; nFile < aryFilesToMove.GetSize(); nFile++) {
			CString strFile = aryFilesToMove.GetAt(nFile);
			CString strMoveTo = cfg.strDestination + FilenameHelpers::GetJustFilename(strFile);
			FileOperations::MoveFileEx_WithRetry(strFile, strMoveTo, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED, pInfo->pLogger, 30);
		}
	}

	pInfo->pLogger->AddToLogByString(_T("Cleaning"));

	for (int i = 0; i < ConfigManager::arrDeleteConfig.GetCount(); i++) {
		auto& cfg = ConfigManager::arrDeleteConfig[i];

		pInfo->pLogger->AddToLogByString(_T("Cleaning folder - ") + cfg.strFolderPath);

		CStringArray aryFiles;
		FileOperations::ListAllFiles(pInfo->pLogger, cfg.strFolderPath, &aryFiles, cfg.bIncludeDirs && !cfg.bIncludeFiles, !cfg.bRecurse, cfg.bIncludeDirs && cfg.bIncludeFiles);

		for (int i = 0; i < aryFiles.GetSize(); i++) {
			CString strFile = aryFiles.GetAt(i);

			int nDot = strFile.ReverseFind('.');

			CString strSuffix = strFile.Mid(nDot);
			strSuffix.MakeUpper();

			bool bShouldExclude = false;

			for (int ext = 0; ext < cfg.arrExcludedExtensions.GetSize(); ext++) {
				CString strExclExtention = cfg.arrExcludedExtensions.GetAt(ext).MakeUpper();

				if (strExclExtention == strSuffix || _T(".") + strExclExtention == strSuffix) {
					bShouldExclude = true;
					break;
				}
			}

			if (bShouldExclude) {
				continue;
			}

			CFileStatus stat;
			if (CFile::GetStatus(strFile, stat))
			{
				SYSTEMTIME st;
				if (stat.m_mtime.GetAsSystemTime(st))
				{
					COleDateTime dtFile(st);

					COleDateTimeSpan span = COleDateTime::GetCurrentTime() - dtFile;

					DWORD dwDaysOld = (DWORD)span.GetTotalDays();

					if (dwDaysOld > cfg.nRetentionDays)
					{
						::SetFileAttributes(strFile, GetFileAttributes(strFile) & !FILE_ATTRIBUTE_READONLY);
						pInfo->pLogger->AddToLogByString(_T("Delete file: ") + strFile);
						if (!FileOperations::DeleteFileSecure(pInfo->pLogger, strFile, FILE_DELETE_ALGORITHM_WINDOWS))
						{
							pInfo->pLogger->AddToLogByString(_T("ERROR: Cannot delete. ") + DataConversion::ConvertDWORDToCString(::GetLastError()));
							AfxMessageBox(_T("Cannot Delete - ") + strFile);
						}
					}
				}
			}

		}
	}

	strMess = _T("Clean up complete");
	pInfo->pLogger->AddToLogByString(strMess);
	::PostMessage(pInfo->hWndParent, WM_APP, (WPARAM)strMess.GetBuffer(0), true);
	delete pInfo;

	::CoUninitialize();
	return 0;
}

CTechnicalTestDlg::CTechnicalTestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TECHNICALTEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nActiveThreads = 0;
}

void CTechnicalTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTechnicalTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CTechnicalTestDlg message handlers

BOOL CTechnicalTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
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

	// TODO: Add extra initialization here
	sCleanupInfo* pInfo = new sCleanupInfo;
	pInfo->hWndParent = m_hWnd;
	pInfo->pLogger = &m_Logger;
	pInfo->pApp = (CTechnicalTestApp*)AfxGetApp();

	m_nActiveThreads++;
	m_Logger.SetLogFile(ConfigManager::strLogFilePath);

	AfxBeginThread(CleanUpThread, pInfo, THREAD_PRIORITY_NORMAL, 0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTechnicalTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTechnicalTestDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTechnicalTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CTechnicalTestDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_APP:
		if (!lParam)
			break;

		m_nActiveThreads--;
		if (m_nActiveThreads < 1)
		{
			m_Logger.AddToLogByString(_T("All threads have completed"));
			AfxMessageBox(_T("Clean up complete"));
			break;
		}

		m_Logger.AddToLogByString(DataConversion::ConvertIntToString(m_nActiveThreads) + _T(" Threads are still running..."));
		break;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}