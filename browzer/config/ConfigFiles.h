#if !defined(AFX_CONFIGFILES_H__449E928C_3B06_11D6_8695_002078049F22__INCLUDED_)
#define AFX_CONFIGFILES_H__449E928C_3B06_11D6_8695_002078049F22__INCLUDED_
#include "Resource.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigFiles.h : header file
//
#include "PlayerDlg.h"
class ThreadParams;

/////////////////////////////////////////////////////////////////////////////
// CConfigFiles dialog

class CConfigFiles : public CPropertyPage
{
	DECLARE_DYNCREATE(CConfigFiles)

// Construction
public:
	CConfigFiles(CPlayerDlg * p = NULL);
	~CConfigFiles();

// Dialog Data
	//{{AFX_DATA(CConfigFiles)
	enum { IDD = IDD_CONFIG_FILES };
	CButton	m_RunAtStartup;
	CListBox	m_Mp3Extensions;
	CEdit	m_Mp3Extension;
	CStatic	m_MdbLocation;
	CListBox	m_MP3DirList;
	CButton m_UseGenre;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CConfigFiles)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CConfigFiles)
	afx_msg void OnDiradd();
	afx_msg void OnSelchangeDirlist();
	afx_msg void OnDirremove();
	afx_msg void OnDirscan();
	afx_msg void OnLocationButton();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnMp3Add();
	afx_msg void OnMp3Remove();
	virtual void OnCancel();
	afx_msg void OnDirscanNew();
	afx_msg void OnAlbumsortDate();
	afx_msg void OnAlbumsortAlpha();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    CPlayerDlg * m_PlayerDlg;
    void EnableDisableButtons();
    CString m_path;

    CStringList m_origMp3Extensions;
    CString m_origMdbLocation;
    CStringList m_origMP3DirList;
    unsigned long m_RunAtStartupUL;
	unsigned long m_UseGenreUL;
	BOOL m_AlbumSortAlpha;
	BOOL m_AlbumSortDate;

    int m_ScanThread_continue;

    void ScanThreadStart(ThreadParams &);
    void ScanThreadStop();
    void StoreReg();
    void ReadReg();
    void init();
    void setRunAtStartup();
	void dirScan();
public:
	BOOL m_scanNew;
    void setDefaults();
	BOOL UseGenre();
	BOOL AlbumSortAlpha();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGFILES_H__449E928C_3B06_11D6_8695_002078049F22__INCLUDED_)
