// mfcplayerDlg.h : header file
//

#if !defined(AFX_PLAYERDLG_H__FCF45C46_7793_11D2_9317_0020AFF7E192__INCLUDED_)
#define AFX_PLAYERDLG_H__FCF45C46_7793_11D2_9317_0020AFF7E192__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// Includes

#include "PlayerApp.h"
#include "InitDlg.h"
#include "MusicDb.h"
#include "irman.h"
//#include "VirtualControl.h"
#include "ExtendedListBox.h"
//#include "MenuDialog.h"
#include "MBConfig.h"
#include "ColorStatic.h"
#include "MusicPlayer.h"
#include "PictureStatic.h"
#include "MyString.h"
#include "Controls.h"
#include "ButtonSt.h"
#include "SkinSlider.h"
#include "PlayerCallbacks.h"

#define CDialogClassImpl CDialogSK

#include "CDialogSK.h"

class VirtualControl;
class VirtualDialog;
class CMenuDialog;
class CTransparentDialogDlg;


class CPlayerDlg;

#ifdef THE_PLAYER
CPlayerDlg * thePlayer = 0;
#else
extern CPlayerDlg * thePlayer;
#endif


// class CPictureDlg;

//#include "LoadPlaylistDlg.h"
//class LoadPlaylistDlg;
/////////////////////////////////////////////////////////////////////////////
// CMfcplayerDlg dialog

//class CConfigSettings;

class CPlayerDlg : public CDialogClassImpl
{
// Construction
public:
    MusicPlayer *m_Player;

//	CPlayerDlg(CPlayerApp *, CTransparentDialogDlg *, CWnd* pParent = NULL);	// standard constructor
	CPlayerDlg(CPlayerApp *, CWnd* pParent = NULL);	// standard constructor
    ~CPlayerDlg();
    CPlayerApp * m_PlayerApp;
	MusicLib m_mlib;
	CString _selectedGenre, _selectedArtist, _selectedAlbum,
        _selectedSong, _selectedPlaylistSong;

	void updatePlaylist(const BOOL save=TRUE);
	virtual BOOL PreTranslateMessage(MSG*);
	Irman & irman() { return m_irman; }
    void PlayLoop();
    void initDb();
    void init();
    void SavePlaylist(CString);
    void LoadPlaylist(CString);
    void OnExit();
    void SetSavePlaylistFlag(BOOL flag) { m_SavePlaylistFlag = flag; }
//	void RunTestHarness();
	void setDuration(int duration);
	void addDuration(int duration);
	void calcDuration();
	afx_msg void OnTimer(UINT nIDEvent);

    VirtualControl * m_Control;
    VirtualDialog * m_Dialog;

    void OnControlSelChange();
    void OnControlClick();
    void OnDelete();
    void OnPageUp();
    void OnPageDown();
    void OnNextSong();
    void OnPreviousSong();
    void setFont();
	void setColors();
    MBConfig & config() { return m_Config; }
    void PlayerStatusSet(CString &);
	void PlayerStatusSet(LPCTSTR);
	void CurrentTitleSet(CString &);
	void CurrentTitleSet(LPCTSTR);
    void PlayerStatusTempSet(CString &);
	void PlayerStatusTempSet(LPCTSTR);
    void setPlaylistCurrent(int sel) {
        m_PlaylistCurrent = sel; }
    void movePlaylistUp(int);
    void movePlaylistDown(int);
	void StartSeekTimer();
	void StopSeekTimer();
	void StartStatusTimer();
	void StopStatusTimer();
	void StopTimers();
	int HandleIRMessage(int key);
    void redraw();
	void Need2Erase(BOOL flag) {
		m_Need2Erase = flag;
	}
private:
	PlayerCallbacks m_callbacks;	
	Controls m_Controls;
	int m_PlaylistDuration;
	CTransparentDialogDlg *_initdialog;
	Irman m_irman;
	CString m_AlbumArt;
public:
    MBConfig m_Config;
private:
	COLORREF m_TransMain;
	COLORREF m_TransPanel;
	CStringList m_Skins;
    CString m_PlayerStatusSave;
    CTime m_PlayerStatusTime;
    CMenuDialog * m_MenuDlg;
    BOOL m_StopFlag;
    int m_PlaylistCurrent;
	CString m_LastThingQueuedUp;
	BOOL m_Resizing;

    CString m_StatusArray[10];

    int mWindowFlag;
	int m_trialCounter;

    void resetControls();
    void SaveWindowPos();
    void ReadWindowPos(int &, CRect &);

//	void updateVolumeLabel();
	void adjustVolume(int level);
	void adjustVolume();
	void OnVolUp();
	void OnVolDown();
	void OnFastForward();
	void OnReverse();
	void updatePositionLabel();
	void adjustPosition();
	void adjustPosition(int pct);
	void resetPosition();
    void PlayerStatusRevert();

	void InputClose();
	void Pause();
	BOOL Play();
	void Stop();
	void saveConfig();
	void readConfig();
	void ShowBetaWarning();
	BOOL HelpInfo();
	void recordTLEN();
 	void displayAlbumArt(const CString & file);
 	void killAlbumArt();

    BOOL m_LButtonDown;
    CPoint m_deltaPoint;
    CBrush m_brush;

    CMapStringToString m_GenreArtist;
    CMapStringToString m_ArtistAlbum;
    CMapStringToString m_AlbumSong;
    BOOL m_SavePlaylistFlag;
	REFERENCE_TIME m_timerid;
	REFERENCE_TIME m_StatusTimerId;
	REFERENCE_TIME m_VolumeTimerId;

	CString m_HelpMsg;
	BOOL m_InitDone;
	enum ArtOwnedType {
		AOTPlaylist, AOTLibrary} m_ArtOwnedBy;
	BOOL m_Maximized;
// Dialog Data
	//{{AFX_DATA(CPlayerDlg)
	enum { IDD = IDD_PLAYER_DIALOG };
	CButtonST	m_ButtonResize;
	CButtonST	m_ButtonShuffle;
	CButtonST	m_ButtonSave;
	CButtonST	m_ButtonReverse;
	CButtonST	m_ButtonRandom;
	CButtonST	m_ButtonLoad;
	CButtonST	m_ButtonFastForward;
	CButtonST	m_ButtonClear;
	CColorStatic	m_GenresLabel;
	CColorStatic	m_ArtistsLabel;
	CColorStatic	m_AlbumsLabel;
	CColorStatic	m_SongsLabel;	
	CColorStatic	m_PlaylistLabel;
	CColorStatic	m_PositionLabel;
	CColorStatic	m_PlayerStatus;
	CColorStatic	m_CurrentTitle;
	CExtendedListBox    m_Genres;
	CExtendedListBox	m_Artists;
	CExtendedListBox	m_Albums;
	CExtendedListBox	m_Songs;	
	CExtendedListBox	m_Playlist;    
	CButtonST		m_OptionsButton;
	CButtonST		m_MusicButton;
	CButtonST		m_PicturesButton;
	CButtonST		m_VideoButton;
	CButtonST		m_ButtonMinimize;
	CButtonST		m_ButtonMaximize;
	CButtonST		m_ButtonExit;
	CButtonST		m_ButtonRestore;
	CButtonST		m_ButtonStop;
	CButtonST		m_ButtonPlay;
	CButtonST		m_ButtonPause;
 	PictureStatic	m_Picture;
	CMySliderCtrl	m_VolumeSlider;
	CMySliderCtrl	m_PositionSlider;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayerDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

public:
	HICON m_hIcon;
//protected:
	// Generated message map functions
	//{{AFX_MSG(CPlayerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnPlayButton();
	afx_msg void OnOpenFileButton();
	afx_msg void OnStopButton();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnAbout();
	afx_msg void OnSelchangeArtists();
	afx_msg void OnSelchangeAlbums();
	afx_msg void OnDblclkSongs();
	afx_msg void OnDblclkAlbums();
	afx_msg void OnDblclkArtists();
	afx_msg void OnDblclkGenres();
	afx_msg void OnMenuButton();
	afx_msg void OnMenuOptions();
	afx_msg void OnMenuClearplaylist();
	afx_msg void OnMenuExit();
	afx_msg void OnMenuLoadplaylist();
	afx_msg void OnMenuSaveplaylist();
    afx_msg void OnMenuExportLibrary();
	afx_msg void OnMenuShuffleplaylist();
	afx_msg void OnMenuPause();
	afx_msg void OnMenuPlay();
	afx_msg void OnMenuStop();
	afx_msg void OnMenuHelp();
	afx_msg void OnSelchangeGenres();
	afx_msg void OnDblclkPlaylist();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUserEditSong();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSelchangeSongs();
	afx_msg void OnSelchangePlaylist();
	afx_msg void OnMenuRandomizePlaylist();
	afx_msg void OnButtonMenu();
	afx_msg void OnMusicButton();
	afx_msg void OnPicturesButton();
	afx_msg void OnVideoButton();
	afx_msg void OnButtonMinimize();
	afx_msg void OnButtonMaximize();
	afx_msg void OnCancel();
	afx_msg void OnButtonResize();
	//}}AFX_MSG
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg LRESULT OnGraphNotify(UINT wParam, LONG lParam);
	afx_msg LRESULT OnPlayloop(UINT wParam, LONG lParam);
	afx_msg LRESULT OnVolume(UINT wParam, LONG lParam);
	afx_msg LRESULT OnProgress(UINT wParam, LONG lParam);
    afx_msg LRESULT OnPostMyIdle(UINT wParam, LONG lParam);
	afx_msg LRESULT OnSkinPic(UINT wParam, LONG lParam);
	afx_msg LRESULT OnMusicAdd(UINT wParam, LONG lParam);
	afx_msg LRESULT OnMusicScan(UINT wParam, LONG lParam);
	afx_msg LRESULT OnMusicScanNew(UINT wParam, LONG lParam);
	afx_msg void OnGenresFocus();
	afx_msg void OnArtistsFocus();
	afx_msg void OnAlbumsFocus();
	afx_msg void OnSongsFocus();
	afx_msg void OnPlaylistFocus();
    afx_msg UINT OnNcHitTest( CPoint point );
    afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
    afx_msg void OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct );
    afx_msg void OnTestMenu();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg LRESULT OnSerialMsg (WPARAM wParam, LPARAM lParam);
	afx_msg void OnNcPaint( );

	DECLARE_MESSAGE_MAP()

//	AFX_THREADPROC  IRReader( LPVOID pParam );
};



#endif // !defined(AFX_PLAYERDLG_H__FCF45C46_7793_11D2_9317_0020AFF7E192__INCLUDED_)


