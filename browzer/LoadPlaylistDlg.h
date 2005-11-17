#if !defined(AFX_LOADPLAYLISTDLG_H__7340F5BB_C8A5_11D5_8680_002078049F22__INCLUDED_)
#define AFX_LOADPLAYLISTDLG_H__7340F5BB_C8A5_11D5_8680_002078049F22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoadPlaylistDlg.h : header file
//
#include "PlayerDlg.h"
#include "MusicDb.h"
#include "ExtendedListBox.h"
#include "ResizableGrip.h"

/////////////////////////////////////////////////////////////////////////////
// LoadPlaylistDlg dialog
class VirtualControl;

class LoadPlaylistDlg : public CDialog, public CResizableGrip
{
// Construction
public:
	LoadPlaylistDlg(CPlayerDlg *p, MusicLib *m, 
		CWnd* pParent = NULL, BOOL editor=FALSE);   // standard constructor

// Dialog Data
	//{{AFX_DATA(LoadPlaylistDlg)
	enum { IDD = IDD_LOADPLAYLISTDLG };
	CButton	m_Delete;
	CButton	m_Load;
	CStatic	m_GAAS;
	CButton m_Exit;
	CButton m_Up;
	CButton m_Down;
	CButton m_Save;
	CButton m_Rename;
	CColorStatic m_NamesLabel;
	CColorStatic m_SongsLabel;
	//}}AFX_DATA
	CExtendedListBox	m_PlaylistSongs;
	CExtendedListBox	m_PlaylistNames;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LoadPlaylistDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
public:
    VirtualControl * m_Control;
    void OnControlSelChange();
    void OnControlClick();
    void OnDelete();
    void OnPageUp();
    void OnPageDown();
    void OnMenuButton();
    void OnExit();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(LoadPlaylistDlg)
	afx_msg void OnDblclkPlaylistNames();
	afx_msg void OnSelchangePlaylistNames();
	afx_msg void OnSetfocusPlaylistNames();
	virtual BOOL OnInitDialog();
	afx_msg void OnSetfocusPlaylistSongs();
	afx_msg void OnCancel();
	afx_msg void OnPlaylistOk();
	afx_msg void OnPlaylistUp();
	afx_msg void OnPlaylistDown();
	afx_msg void OnPlaylistSave();
	afx_msg void OnDeletePlaylist();
	afx_msg void OnRenamePlaylist();

	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusPlaylistSongs();
	afx_msg void OnSelchangePlaylistSongs();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg UINT OnNcHitTest( CPoint point );
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	afx_msg void OnNcMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnNcLButtonDown(UINT, CPoint);
	afx_msg void OnNcLButtonUp(UINT, CPoint);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnMovePlaylistUp(UINT wParam, LONG lParam=0);
	afx_msg LRESULT OnMovePlaylistDn(UINT wParam, LONG lParam=0);
	void AdjustLibraryWidths(CPoint & point);
	CWnd* GetResizableWnd();

private:
    MusicLib *m_mdb;
    CPlayerDlg *m_pd;
	BOOL m_Resizing;
	void resetControls();
    void init();
	CBrush m_brush,m_HatchBrush;
	CRect m_AdjustRect,m_AdjustInt;
	BOOL m_AdjustLibrary, m_LibraryDragging;
	BOOL m_IsEditor;
	int m_MinNamesWidth,m_MaxNamesWidth;
	CStringArray m_csaPlaylistDesc,m_csaPlaylist;
	CString m_LastName;
	BOOL m_Modified;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOADPLAYLISTDLG_H__7340F5BB_C8A5_11D5_8680_002078049F22__INCLUDED_)
