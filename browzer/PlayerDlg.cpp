


// PlayerDlg.cpp : implementation file

#include "stdafx.h"
#define THE_PLAYER
#include "PlayerDlg.h"

#define LOGOWNER
#include "MyLog.h"

#include "About.h"
#include "ConfigFiles.h"
#include "ConfigIrman.h"
#include "DialogData.h"
#include "FontCombo.h"
#include "FontTipWnd.h"
#include "HtmlHelp.h"
#include "id3/misc_support.h"
#include "IRCodes.h"
#include "Registry.h"
#include "LoadPlaylistDlg.h"
#include "MBConfig.h"
#include "MBGlobals.h"
#include "MBMessageBox.h"
#include "ModifyIDThree.h"
#include "MusicDb.h"
#include "MyID3LibMiscSupport.h"
#include "WmaTagger.h"
#include "PlayerApp.h"
#include "SavePlaylistDlg.h"
#include "SerialMFC.h"
#include "TestHarness/TestHarness.h"
#include "VirtualControl.h"
#include "muzikbrowzerVersion.h"
#include "TransparentDialogDlg.h"
#include "util/Misc.h"
#include "FileUtils.h"
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "SkinDefs.h"
#include "DIBSectionLite.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void Redraw() {
	thePlayer->redraw();

}
CString MBDir() {
	return thePlayer->m_Config.mbdir();
}
void initDb() {
	thePlayer->initDb();
}
CString GetLibraryCounts() {
	return thePlayer->m_mlib.getLibraryCounts();
}
void PlayerStatusSet(CString text) {
	thePlayer->PlayerStatusSet(text);
}
void PlayerUpdateWindow() {
	thePlayer->UpdateWindow();
}
void SetDbLocation(CString path) {
	thePlayer->m_mlib.setDbLocation(path);
}
CString ScanDirectories(const CStringList & directories,
						InitDlg * initDlg, BOOL scanNew) {
	return thePlayer->m_mlib.scanDirectories(directories, initDlg, scanNew);
}

/////////////////////////////////////////////////////////////////////////////
// CPlayerDlg dialog

CPlayerDlg::CPlayerDlg(CPlayerApp * theApp, CTransparentDialogDlg *ip, 
					   CWnd* pParent /*=NULL*/)
	: CDialogClassImpl(CPlayerDlg::IDD, pParent), m_PlayerApp(theApp), 
	_initdialog(ip),
	m_mlib(NULL), m_PlaylistCurrent(-1),
    m_MenuDlg(0), m_StopFlag(FALSE), m_LButtonDown(FALSE),
    m_Config("Configuration"), m_SavePlaylistFlag(TRUE),
	m_PlaylistDuration(0), m_timerid(0), m_StatusTimerId(0),
	m_Control(new VirtualControl), m_Dialog(new VirtualDialog),
	m_trialCounter(0), //m_AppLabel(FALSE),
	m_ArtOwnedBy(AOTLibrary),
	m_InitDone(FALSE),
	m_Genres(TRUE,"genres"),
	m_Artists(TRUE,"artists"),
	m_Albums(TRUE,"albums"),
	m_Songs(TRUE,"songs"),
	m_Playlist(TRUE, "playlist")

{
	//{{AFX_DATA_INIT(CPlayerDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
    m_Playlist.m_reorder = TRUE;
	m_hIcon = AfxGetApp()->LoadIcon(IDI_MBLOGO);
//    m_hIcon = AfxGetApp()->LoadIcon(IDI_MBLOGO3232);
	thePlayer = this;
	
	m_callbacks.redraw = &::Redraw;
	m_callbacks.mbdir = &::MBDir;
	m_callbacks.initDb = &::initDb;
	m_callbacks.getLibraryCounts = &::GetLibraryCounts;
	m_callbacks.statusset = &::PlayerStatusSet;
	m_callbacks.UpdateWindow = &::PlayerUpdateWindow;
	m_callbacks.setDbLocation = &::SetDbLocation;
	m_callbacks.scanDirectories = &::ScanDirectories;

	irman().init(RegKeyIrman, IR_MESSAGE_NUMBER_OF, this);
//	m_brush.CreateSolidBrush(RGB( 255, 0, 0));
	m_Resizing = FALSE;
}
CPlayerDlg::~CPlayerDlg() {
    //delete m_Config;
    //m_brush.DeleteObject();
	delete m_Control;
	delete m_Dialog;
	delete m_Player;
}

void CPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogClassImpl::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CPlayerDlg)
	DDX_Control(pDX, IDC_BUTTON_RESIZE, m_ButtonResize);
	DDX_Control(pDX, IDC_BUTTON_SHUFFLE, m_ButtonShuffle);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_ButtonSave);
	DDX_Control(pDX, IDC_BUTTON_REVERSE, m_ButtonReverse);
	DDX_Control(pDX, IDC_BUTTON_RANDOM, m_ButtonRandom);
	DDX_Control(pDX, IDC_BUTTON_LOAD, m_ButtonLoad);
	DDX_Control(pDX, IDC_BUTTON_FASTFORWARD, m_ButtonFastForward);
	DDX_Control(pDX, IDC_BUTTON_CLEAR, m_ButtonClear);
	DDX_Control(pDX, IDC_GENRESLABEL,               m_GenresLabel);
	DDX_Control(pDX, IDC_ARTISTSLABEL,              m_ArtistsLabel);
	DDX_Control(pDX, IDC_ALBUMSLABEL,               m_AlbumsLabel);
	DDX_Control(pDX, IDC_SONGSLABEL,                m_SongsLabel);
	DDX_Control(pDX, IDC_PLAYLISTLABEL,             m_PlaylistLabel);
	DDX_Control(pDX, IDC_POSITION_LABEL,            m_PositionLabel);
	DDX_Control(pDX, IDC_PLAYER_STATUS,             m_PlayerStatus);
	DDX_Control(pDX, IDC_CURRENT_TITLE,             m_CurrentTitle);
	DDX_Control(pDX, IDC_GENRES,                    m_Genres);
	DDX_Control(pDX, IDC_ARTISTS,                   m_Artists);
	DDX_Control(pDX, IDC_ALBUMS,                    m_Albums);
	DDX_Control(pDX, IDC_SONGS,                     m_Songs);
	DDX_Control(pDX, IDC_PLAYLIST,                  m_Playlist);
	DDX_Control(pDX, IDC_OPTIONS_BUTTON,            m_OptionsButton);
	DDX_Control(pDX, IDC_BUTTON_MINIMIZE,           m_ButtonMinimize);
	DDX_Control(pDX, IDC_BUTTON_MAXIMIZE,           m_ButtonMaximize);
	DDX_Control(pDX, IDC_BUTTON_EXIT,               m_ButtonExit);
	DDX_Control(pDX, IDC_BUTTON_STOP,               m_ButtonStop);
	DDX_Control(pDX, IDC_BUTTON_PLAY,               m_ButtonPlay);
	DDX_Control(pDX, IDC_BUTTON_PAUSE,              m_ButtonPause);
	DDX_Control(pDX, IDC_PICTURE_CTRL,              m_Picture);
	DDX_Control(pDX, IDC_VOLUME_SLIDER,             m_VolumeSlider);
	DDX_Control(pDX, IDC_POSITION_SLIDER,			m_PositionSlider);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPlayerDlg, CDialogClassImpl)
	//{{AFX_MSG_MAP(CPlayerDlg)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR,OnMenuClearplaylist)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, OnCancel)
	ON_BN_CLICKED(IDC_BUTTON_FASTFORWARD, OnNextSong)
	ON_BN_CLICKED(IDC_BUTTON_LOAD,OnMenuLoadplaylist)
	ON_BN_CLICKED(IDC_BUTTON_MAXIMIZE, OnButtonMaximize)
	ON_BN_CLICKED(IDC_BUTTON_MINIMIZE, OnButtonMinimize)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, OnMenuPause)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnMenuPlay)
	ON_BN_CLICKED(IDC_BUTTON_RANDOM,OnMenuRandomizePlaylist)
	ON_BN_CLICKED(IDC_BUTTON_RESIZE, OnButtonResize)
	ON_BN_CLICKED(IDC_BUTTON_REVERSE, OnPreviousSong)
	ON_BN_CLICKED(IDC_BUTTON_SAVE,OnMenuSaveplaylist)
	ON_BN_CLICKED(IDC_BUTTON_SHUFFLE,OnMenuShuffleplaylist)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnMenuStop)
	ON_BN_CLICKED(IDC_MENU_BUTTON, OnMenuButton)
	ON_BN_CLICKED(IDC_OPEN_FILE_BUTTON, OnOpenFileButton)
	ON_BN_CLICKED(IDC_OPTIONS_BUTTON, OnButtonMenu)
	ON_BN_CLICKED(IDC_PLAY_BUTTON, OnPlayButton)
	ON_BN_CLICKED(IDC_STOP_BUTTON, OnStopButton)
	ON_BN_CLICKED(IDD_ABOUT, OnAbout)
	ON_COMMAND(ID_EDIT_ID3TAG, OnUserEditSong)
	ON_COMMAND(ID_MENU_CLEARPLAYLIST, OnMenuClearplaylist)
	ON_COMMAND(ID_MENU_EXIT, OnMenuExit)
	ON_COMMAND(ID_MENU_EXPORT, OnMenuExportLibrary)
	ON_COMMAND(ID_MENU_HELP, HelpInfo)
	ON_COMMAND(ID_MENU_LOADPLAYLIST, OnMenuLoadplaylist)
	ON_COMMAND(ID_MENU_OPTIONS, OnMenuOptions)
	ON_COMMAND(ID_MENU_PAUSE, OnMenuPause)
	ON_COMMAND(ID_MENU_PLAY, OnMenuPlay)
	ON_COMMAND(ID_MENU_RANDOMIZE_PLAYLIST, OnMenuRandomizePlaylist)
	ON_COMMAND(ID_MENU_SAVEPLAYLIST, OnMenuSaveplaylist)
	ON_COMMAND(ID_MENU_SHUFFLEPLAYLIST, OnMenuShuffleplaylist)
	ON_COMMAND(ID_MENU_STOP, OnMenuStop)
	ON_COMMAND(ID_PMENU_CLEAR, OnMenuClearplaylist)
	ON_COMMAND(ID_PMENU_EXIT, OnMenuExit)
	ON_COMMAND(ID_PMENU_HELP, OnMenuHelp)
	ON_COMMAND(ID_PMENU_LOADPLAYLIST, OnMenuLoadplaylist)
	ON_COMMAND(ID_PMENU_PAUSE, OnMenuPause)
	ON_COMMAND(ID_PMENU_PLAY, OnMenuPlay)
	ON_COMMAND(ID_PMENU_SHUFFLE, OnMenuShuffleplaylist)
	ON_COMMAND(ID_PMENU_STOP, OnMenuStop)
	ON_LBN_DBLCLK(IDC_ALBUMS, OnDblclkAlbums)
	ON_LBN_DBLCLK(IDC_ARTISTS, OnDblclkArtists)
	ON_LBN_DBLCLK(IDC_GENRES, OnDblclkGenres)
	ON_LBN_DBLCLK(IDC_PLAYLIST, OnDblclkPlaylist)
	ON_LBN_DBLCLK(IDC_SONGS, OnDblclkSongs)
	ON_LBN_SELCHANGE(IDC_ALBUMS, OnSelchangeAlbums)
	ON_LBN_SELCHANGE(IDC_ARTISTS, OnSelchangeArtists)
	ON_LBN_SELCHANGE(IDC_GENRES, OnSelchangeGenres)
	ON_LBN_SELCHANGE(IDC_PLAYLIST, OnSelchangePlaylist)
	ON_LBN_SELCHANGE(IDC_SONGS, OnSelchangeSongs)
	ON_WM_COMPAREITEM()
	ON_WM_CONTEXTMENU()
	ON_WM_HSCROLL()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_SYSCOMMAND()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP

	ON_MESSAGE(WM_GRAPHNOTIFY, OnGraphNotify)
	ON_MESSAGE(WM_PLAYLOOP, OnPlayloop)
	ON_MESSAGE(MB_VOLUME_MSG, OnVolume)
	ON_MESSAGE(MB_PROGRESS_MSG, OnProgress)
    ON_MESSAGE(MB_POST_MYIDLE_MESSAGE, OnPostMyIdle)
	ON_LBN_SETFOCUS(IDC_GENRES, OnGenresFocus)
	ON_LBN_SETFOCUS(IDC_ARTISTS, OnArtistsFocus)
	ON_LBN_SETFOCUS(IDC_ALBUMS, OnAlbumsFocus)
	ON_LBN_SETFOCUS(IDC_SONGS, OnSongsFocus)
	ON_LBN_SETFOCUS(IDC_PLAYLIST, OnPlaylistFocus)
    ON_WM_NCHITTEST()
	ON_WM_CAPTURECHANGED()
	ON_WM_NCPAINT()
    ON_WM_CTLCOLOR()
    ON_COMMAND(ID_MYTEST, OnTestMenu)
    ON_WM_HELPINFO()
	ON_WM_TIMER()
	ON_MESSAGE(MB_SERIAL_MESSAGE, OnSerialMsg)


END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CPlayerDlg message handlers
void initFont(CWnd *);
void changeFont(CWnd *, CFont &);
BOOL CPlayerDlg::OnInitDialog()
{
	CDialogClassImpl::OnInitDialog();
	
	m_Genres.m_id = "genres";
	m_Artists.m_id = "artists";
	m_Albums.m_id = "albums";
	m_Songs.m_id = "songs";
	m_Playlist.m_id = "playlist";

    *m_Dialog = this;
	m_mlib.readDbLocation();
    CString lpath = m_mlib.getDbLocation();
    logger.open(lpath);
	logger.log(CS("muzikbrowzer version: ") + CS(MUZIKBROWZER_VERSION));
	m_Config.createit(this, &m_callbacks);

	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	CString msg = "GetVersionEx=";
	msg += osvi.dwPlatformId;
	logger.log(msg);

    if (m_mlib.init()) {
		MBMessageBox("Error", "Database corrupted. Rebuild it by performing a\r\nScan in Options/Configuration");
		PlayerStatusSet(CString(
			"Database corrupted. Do a scan in Configuration."));
	}

    // set some of the control's properties
	// progress slider must be 0 to 100 in order for percentage
	// stuff to work - see updatePosition
    m_PositionSlider.SetRange(0,100);
	m_VolumeSlider.SetRange(0,50);

    // instanciate a Player
	m_Player = new MusicPlayer(this, lpath);
	m_Player->init();

    // sets title in taskbar
    SetWindowText(MUZIKBROWZER); // needed if you used resource editor to remove title bar	
    ModifyStyle(WS_CAPTION,WS_SYSMENU,SWP_FRAMECHANGED);

    m_HeaderFont.CreateFont(
        /* height */ 				20,
        /* width */ 				0,
        /* escapement */ 			0,
        /* orientation */ 			0,
        /* weight */ 				FW_BOLD,
        /* italic */ 				FALSE,
        /* underline */ 			FALSE,
        /* strikeout */ 			0,
        /* charset */ 				DEFAULT_CHARSET,
        /* outprecision */ 			OUT_CHARACTER_PRECIS,
        /* clipprecision */ 		CLIP_CHARACTER_PRECIS,
        /* quality */ 				DEFAULT_QUALITY,
        /* pitch and family */ 		DEFAULT_PITCH,
        /* facename */ 				"Arial"
		);

	m_OptionsButtonFont.CreateFont(
        /* height */ 				12,
        /* width */ 				0,
        /* escapement */ 			0,
        /* orientation */ 			0,
        /* weight */ 				FW_BOLD,
        /* italic */ 				FALSE,
        /* underline */ 			FALSE,
        /* strikeout */ 			0,
        /* charset */ 				DEFAULT_CHARSET,
        /* outprecision */ 			OUT_CHARACTER_PRECIS,
        /* clipprecision */ 		CLIP_CHARACTER_PRECIS,
        /* quality */ 				DEFAULT_QUALITY,
        /* pitch and family */ 		DEFAULT_PITCH,
        /* facename */ 				"Arial"
		);

//	m_OptionsButton.SetWindowText("&Menu");

	if (m_Config.UseGenre()) {
		m_GenresLabel.setText("Genres");
	}
	m_ArtistsLabel.setText("Artists");
	m_AlbumsLabel.setText("Albums");
	m_SongsLabel.setText("Songs");
	m_PlaylistLabel.setText("Playlist");

    CRect rect;
    int max;
    ReadWindowPos(max, rect);
    if (max) {
        ShowWindow(SW_SHOWMAXIMIZED);
    } else if (rect.TopLeft().x != 0) {
        MoveWindow(rect, TRUE );
    }

// For CDialogSK
	EnableEasyMove();  // enable moving of the dialog 
    SetTransparentColor(RGB(255, 0, 0)); // set green as the 

    resetControls();

    if (m_Config.UseGenre()) {
		m_Genres.SetFocus();
	    OnGenresFocus();
	} else {
		m_Artists.SetFocus();
		OnArtistsFocus();
	}
	initDb();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small ic

	irman().Open();

	//ShowBetaWarning();
	StartStatusTimer();
	if (m_Config.trialMode() == TRUE) {
		PlayerStatusTempSet("Trial Mode. Set Password in Configuration.");
	}
	CString menuFunc,menuDesc;
	irman().getDescs(IR_MESSAGE_MENU, menuFunc, menuDesc);
	if (menuDesc == "") menuDesc = "Menu";
	m_HelpMsg = "Press "; m_HelpMsg += menuDesc; m_HelpMsg += " for options.";
	if (m_Config.trialMode() == 1) {
		m_HelpMsg += " Trial Mode.";
	} else if (m_Config.trialMode() == 2) {
		m_HelpMsg = " Trial Expired.";
	}
	PlayerStatusSet(m_HelpMsg);

	UpdateWindow();
	if (m_Config.UseGenre()) {m_Genres.RedrawWindow(FALSE);};
	m_Artists.RedrawWindow(FALSE);

//	create themes dir, playlists dir here
	CString tmp = m_Config.mbdir();
//	tmp += "\\themes";
//	FileUtil::mkdirp(tmp);
	tmp = m_Config.mbdir();
	tmp += "\\playlists";
	FileUtil::mkdirp(tmp);

	readConfig();

	m_AlbumArt = m_Config.getSkin(MB_SKIN_ALBUMART);

	if (_initdialog) {
		time_t now = CTime::GetCurrentTime().GetTime();
		if (now - _initdialog->m_start.GetTime() < 2) {
			Sleep(1000);
		}
		_initdialog->DestroyWindow();
	}

	return FALSE;  // return TRUE  unless you set the focus to a control
}
void CPlayerDlg::ShowBetaWarning() {
	CTime t = CTime::GetCurrentTime();
	CString mdy = t.Format("%d%m%y");
	RegistryKey reg(HKEY_LOCAL_MACHINE, RegKey);
	AutoBuf buf(500);
	reg.Read("lastrunmdy", buf.p, 49, "");
	CString lastrunmdy = buf.p;
	if (lastrunmdy != mdy) {
		reg.Write("lastrunmdy", (LPCTSTR) mdy);
		CString msg;
	msg << 
"Warning: This is a beta version of muzikbrowzer. It\r\n" <<
"may contain bugs which could potentially damage\r\n" <<
"damage data on your computer. While every effort has\r\n" <<
"been made to ensure against this there are no\r\n" <<
"guarantees. Pecan Ventures assumes no liability what\r\n" <<
"so ever for any damage caused by muzikbrowzer\r\n";
		MBMessageBox("Beta software", msg);
	}
}

void
CPlayerDlg::saveConfig() {
	int v = m_VolumeSlider.GetPos();
	m_Config.setRegistry("Volume", v);
	SaveWindowPos();
}
void
CPlayerDlg::readConfig() {
	int v=50;
	m_Config.getRegistry("Volume", v, 50);
	adjustVolume(v);
}

void 
initFont(CWnd * wnd) {

// Create a new font so we can change it later
     CFont * f = wnd->GetFont();
     CFont newfont;
     LOGFONT lf;
     if(f != NULL)
         { /* Set up duplicate font */
          f->GetObject(sizeof(LOGFONT), &lf);
          newfont.CreateFontIndirect(&lf);
         } /* Set up duplicate font */
     else
         { /* Use default font spec */
          newfont.CreateStockObject(ANSI_VAR_FONT);
         } /* Use default font spec */
     wnd->SetFont(&newfont);
     newfont.Detach();
}
void
changeFont(CWnd * wnd, CFont & f) {
     CFont * oldfont = wnd->GetFont();
     if(oldfont != NULL)
        oldfont->DeleteObject(); 

    wnd->SetFont(&f);
    f.Detach();
}
void CPlayerDlg::setFont() {
    static int first = 1;
    if (first) {
        first = 0;
        if (m_Config.UseGenre()) m_Genres.initFont();
        m_Artists.initFont();
        m_Albums.initFont();
        m_Songs.initFont();
        m_Playlist.initFont();
		if (m_Config.UseGenre()) m_GenresLabel.initFont();
		m_ArtistsLabel.initFont();
		m_AlbumsLabel.initFont();
		m_SongsLabel.initFont();
		m_PlaylistLabel.initFont();
		m_PlayerStatus.initFont();
		m_PositionLabel.initFont();
//		m_VolumeLabel.initFont();
		m_CurrentTitle.initFont();
		m_CurrentTitle.m_HCenter = 0;
		m_PlayerStatus.m_HCenter = 1;
    }
    LPLOGFONT lplfTitles = m_Config.getTitlesFont();
    if (m_Config.UseGenre()) m_Genres.changeFont(lplfTitles);
    m_Artists.changeFont(lplfTitles);
    m_Albums.changeFont(lplfTitles);
    m_Songs.changeFont(lplfTitles);
    m_Playlist.changeFont(lplfTitles);
	LPLOGFONT lplfPanel = m_Config.getPanelFont();
	if (m_Config.UseGenre()) m_GenresLabel.changeFont(lplfPanel);
	m_ArtistsLabel.changeFont(lplfPanel);
	m_AlbumsLabel.changeFont(lplfPanel);
	m_SongsLabel.changeFont(lplfPanel);
	m_PlaylistLabel.changeFont(lplfPanel);

	m_PlayerStatus.changeFont(lplfPanel);
	m_PositionLabel.changeFont(lplfPanel);
//	m_VolumeLabel.changeFont(lplfPanel);
	m_CurrentTitle.changeFont(lplfTitles);

//	m_OptionsButton.SetFont(&m_OptionsButtonFont);


}
void
CPlayerDlg::setColors() {
	COLORREF transclr = RGB(255,0,0);
	m_GenresLabel.SetTextColor(m_Config.getColorTxColHdr());
	m_ArtistsLabel.SetTextColor(m_Config.getColorTxColHdr());
	m_AlbumsLabel.SetTextColor(m_Config.getColorTxColHdr());
	m_SongsLabel.SetTextColor(m_Config.getColorTxColHdr());
	m_PlaylistLabel.SetTextColor(m_Config.getColorTxColHdr());
//	m_VolumeLabel.SetTextColor(m_Config.getColorTxPanel());
	m_PositionLabel.SetTextColor(m_Config.getColorTxPanel());
	m_PlayerStatus.SetTextColor(m_Config.getColorTxPanel());
	m_CurrentTitle.SetTextColor(m_Config.getColorTxPanel());

	m_GenresLabel.SetBkColor(m_Config.getColorBkColHdr());
	m_ArtistsLabel.SetBkColor(m_Config.getColorBkColHdr());
	m_AlbumsLabel.SetBkColor(m_Config.getColorBkColHdr());
	m_SongsLabel.SetBkColor(m_Config.getColorBkColHdr());
	m_PlaylistLabel.SetBkColor(m_Config.getColorBkColHdr());
//	m_VolumeLabel.SetBkColor(m_Config.getColorBkPanel());
	m_PositionLabel.SetBkColor(m_Config.getColorBkPanel());
	m_PlayerStatus.SetBkColor(m_Config.getColorBkPanel());
	m_CurrentTitle.SetBkColor(m_Config.getColorBkPanel());

	m_Genres.SetColors(m_Config.getColorBkNormal(),
		m_Config.getColorBkHigh(),
		m_Config.getColorBkSel(),
		m_Config.getColorTxNormal(),
		m_Config.getColorTxHigh(),
		m_Config.getColorTxSel());
	m_Artists.SetColors(m_Config.getColorBkNormal(),
		m_Config.getColorBkHigh(),
		m_Config.getColorBkSel(),
		m_Config.getColorTxNormal(),
		m_Config.getColorTxHigh(),
		m_Config.getColorTxSel());
	m_Albums.SetColors(m_Config.getColorBkNormal(),
		m_Config.getColorBkHigh(),
		m_Config.getColorBkSel(),
		m_Config.getColorTxNormal(),
		m_Config.getColorTxHigh(),
		m_Config.getColorTxSel());
	m_Songs.SetColors(m_Config.getColorBkNormal(),
		m_Config.getColorBkHigh(),
		m_Config.getColorBkSel(),
		m_Config.getColorTxNormal(),
		m_Config.getColorTxHigh(),
		m_Config.getColorTxSel());
	m_Playlist.SetColors(m_Config.getColorBkNormal(),
		m_Config.getColorBkHigh(),
		m_Config.getColorBkSel(),
		m_Config.getColorTxNormal(),
		m_Config.getColorTxHigh(),
		m_Config.getColorTxSel());

	m_OptionsButton.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONOPTIONSIN),	transclr,
		m_Config.getSkin(MB_SKIN_BUTTONOPTIONSHOVER),	transclr,
		m_Config.getSkin(MB_SKIN_BUTTONOPTIONSOUT),	transclr
		);
	m_ButtonResize.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONRESIZEIN),	transclr,	
		m_Config.getSkin(MB_SKIN_BUTTONRESIZEHOVER),	transclr,	
		m_Config.getSkin(MB_SKIN_BUTTONRESIZEOUT),		transclr);

	m_ButtonMinimize.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONMINIMIZEIN),	transclr,	
		m_Config.getSkin(MB_SKIN_BUTTONMINIMIZEHOVER),	transclr,	
		m_Config.getSkin(MB_SKIN_BUTTONMINIMIZEOUT),		transclr);

	m_ButtonMaximize.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONMAXIMIZEIN),	transclr,	
		m_Config.getSkin(MB_SKIN_BUTTONMAXIMIZEHOVER),	transclr,	
		m_Config.getSkin(MB_SKIN_BUTTONMAXIMIZEOUT),		transclr);

	m_ButtonExit.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONEXITIN),	transclr,	
		m_Config.getSkin(MB_SKIN_BUTTONEXITHOVER),	transclr,	
		m_Config.getSkin(MB_SKIN_BUTTONEXITOUT),		transclr);

	m_ButtonStop.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONSTOPIN),	transclr,	
		m_Config.getSkin(MB_SKIN_BUTTONSTOPHOVER),	transclr,	
		m_Config.getSkin(MB_SKIN_BUTTONSTOPOUT),		transclr);

	m_ButtonPlay.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONPLAYIN),	transclr,	
		m_Config.getSkin(MB_SKIN_BUTTONPLAYHOVER),	transclr,	
		m_Config.getSkin(MB_SKIN_BUTTONPLAYOUT),		transclr);

	m_ButtonPause.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONPAUSEIN),	transclr,	
		m_Config.getSkin(MB_SKIN_BUTTONPAUSEHOVER),	transclr,	
		m_Config.getSkin(MB_SKIN_BUTTONPAUSEOUT),		transclr);

	m_ButtonShuffle.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONSHUFFLEIN), transclr, 
		m_Config.getSkin(MB_SKIN_BUTTONSHUFFLEHOVER), transclr, 
		m_Config.getSkin(MB_SKIN_BUTTONSHUFFLEOUT), transclr);

	m_ButtonSave.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONSAVEIN), transclr, 
		m_Config.getSkin(MB_SKIN_BUTTONSAVEHOVER), transclr, 
		m_Config.getSkin(MB_SKIN_BUTTONSAVEOUT), transclr);

	m_ButtonReverse.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONREVERSEIN), transclr, 
		m_Config.getSkin(MB_SKIN_BUTTONREVERSEHOVER), transclr, 
		m_Config.getSkin(MB_SKIN_BUTTONREVERSEOUT), transclr);

	m_ButtonRandom.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONRANDOMIN), transclr, 
		m_Config.getSkin(MB_SKIN_BUTTONRANDOMHOVER), transclr, 
		m_Config.getSkin(MB_SKIN_BUTTONRANDOMOUT), transclr);

	m_ButtonLoad.logit = TRUE;

	m_ButtonLoad.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONLOADIN), transclr, 
		m_Config.getSkin(MB_SKIN_BUTTONLOADHOVER), transclr, 
		m_Config.getSkin(MB_SKIN_BUTTONLOADOUT), transclr);

	m_ButtonFastForward.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONFASTFORWARDIN), transclr, 
		m_Config.getSkin(MB_SKIN_BUTTONFASTFORWARDHOVER), transclr, 
		m_Config.getSkin(MB_SKIN_BUTTONFASTFORWARDOUT), transclr);

	m_ButtonClear.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONCLEARIN), transclr, 
		m_Config.getSkin(MB_SKIN_BUTTONCLEARHOVER), transclr, 
		m_Config.getSkin(MB_SKIN_BUTTONCLEAROUT), transclr);

//	m_OptionsButton.SetColor(CButtonST::BTNST_COLOR_FG_IN, m_Config.getColorTxPanel(), TRUE);
//	m_OptionsButton.SetColor(CButtonST::BTNST_COLOR_FG_OUT, m_Config.getColorTxPanel(), TRUE);
//	m_OptionsButton.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS, m_Config.getColorTxPanel(), TRUE);
	
//	m_OptionsButton.SetAlign(CButtonST::ST_ALIGN_OVERLAP,TRUE);
	m_ButtonMinimize.SetAlign(CButtonST::ST_ALIGN_OVERLAP,TRUE);
	m_ButtonResize.SetAlign(CButtonST::ST_ALIGN_OVERLAP,TRUE);
	m_ButtonMaximize.SetAlign(CButtonST::ST_ALIGN_OVERLAP,TRUE);
	m_ButtonExit.SetAlign(CButtonST::ST_ALIGN_OVERLAP,TRUE);
//	m_ButtonRestore.SetAlign(CButtonST::ST_ALIGN_OVERLAP,TRUE);
	m_ButtonStop.SetAlign(CButtonST::ST_ALIGN_OVERLAP,TRUE);
	m_ButtonPlay.SetAlign(CButtonST::ST_ALIGN_OVERLAP,TRUE);
	m_ButtonPause.SetAlign(CButtonST::ST_ALIGN_OVERLAP,TRUE);

	m_OptionsButton.DrawTransparent(TRUE);
	m_ButtonMinimize.DrawTransparent(TRUE);
	m_ButtonResize.DrawTransparent(TRUE);
	m_ButtonMaximize.DrawTransparent(TRUE);
	m_ButtonExit.DrawTransparent(TRUE);
//	m_ButtonRestore.DrawTransparent(TRUE);
	m_ButtonStop.DrawTransparent(TRUE);
	m_ButtonPlay.DrawTransparent(TRUE);
	m_ButtonPause.DrawTransparent(TRUE);

	m_ButtonShuffle.DrawTransparent(TRUE);
	m_ButtonSave.DrawTransparent(TRUE);
	m_ButtonReverse.DrawTransparent(TRUE);
	m_ButtonRandom.DrawTransparent(TRUE);
	m_ButtonLoad.DrawTransparent(TRUE);
	m_ButtonFastForward.DrawTransparent(TRUE);
	m_ButtonClear.DrawTransparent(TRUE);

	m_VolumeSlider.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONVOLUMEBACKGROUND),
		transclr,
		m_Config.getSkin(MB_SKIN_BUTTONVOLUMEKNOB),
		transclr
		);
	m_PositionSlider.SetBitmaps(
		m_Config.getSkin(MB_SKIN_BUTTONPROGRESSBACKGROUND),
		transclr,
		m_Config.getSkin(MB_SKIN_BUTTONPROGRESSKNOB),
		transclr
		);
	m_PositionSlider.SetOrientation(CMySliderCtrl::HORIZONTAL);

}
void
CPlayerDlg::resetControls() {
	static BOOL firsttime = TRUE;

	CString skindef = m_Config.getSkin(MB_SKIN_DEF);
	RegistryKey regSD(skindef);
	regSD.ReadFile();
//#ifdef _DEBUG	
	if (firsttime) {
		int w = regSD.Read("StartWidth",0);
		int h = regSD.Read("StartHeight",0);
		if (w && h) {
			CRect rect;
			int max;
			ReadWindowPos(max, rect);
			rect.right = rect.left + w;
			rect.bottom = rect.top + h;
			MoveWindow(rect, TRUE );
		}
	}
//#endif
	
	m_Controls.init(this);

	setFont();
	setColors();

	GetClientRect(m_Controls.dialogrect);
//	CString msg = "dlg w,h=" + numToString(m_Controls.dialogrect.Width())
//		+ " " + numToString(m_Controls.dialogrect.Height()) + "\r\n";
//	OutputDebugString(msg);

	int ControlBoxWidth = regSD.Read("ControlBoxWidth",0);
	int ControlBoxHeight = regSD.Read("ControlBoxHeight",0);

	int border = m_Config.getDlgBorderWidth();
	int panelborder = m_Config.getPanelWidth();
	int borderhorz = m_Config.getDlgBorderHorz();
	int bordervert = m_Config.getDlgBorderVert();


	m_Controls.FreeEm();

	m_Controls.add(0,0, "options button", IDC_OPTIONS_BUTTON);
	m_Controls.add(0,1, "app label",		IDC_APP_LABEL);
	m_Controls.add(0,2, "button min",		IDC_BUTTON_MINIMIZE);
	m_Controls.add(0,3, "button max",		IDC_BUTTON_MAXIMIZE);
	m_Controls.add(0,4, "button exit",	IDC_BUTTON_EXIT);
	m_Controls.add(0,5, "button resize",IDC_BUTTON_RESIZE);
	m_Controls.add(1,0, "button reverse",	IDC_BUTTON_REVERSE);
	m_Controls.add(1,1, "button stop",	IDC_BUTTON_STOP);
	m_Controls.add(1,2, "button play",	IDC_BUTTON_PLAY);
	m_Controls.add(1,3, "button pause",	IDC_BUTTON_PAUSE);
	m_Controls.add(1,4,	"button fford", IDC_BUTTON_FASTFORWARD);
	m_Controls.add(1,5, "playlist",		IDC_PLAYLIST, IDC_PLAYLISTLABEL);
	m_Controls.add(1,6, "picture",		IDC_PICTURE_CTRL);
	m_Controls.add(2,0, "random",	IDC_BUTTON_RANDOM);
	m_Controls.add(2,1, "vol slider",		IDC_VOLUME_SLIDER);
	m_Controls.add(3,0, "shuffle",		IDC_BUTTON_SHUFFLE);
	m_Controls.add(4,0, "clear",		IDC_BUTTON_CLEAR);
	m_Controls.add(5,0, "load",			IDC_BUTTON_LOAD);
	m_Controls.add(6,0, "save",			IDC_BUTTON_SAVE);
	m_Controls.add(7,0, "pos label",		IDC_POSITION_LABEL);

	m_Controls.add(7,1, "title",			IDC_CURRENT_TITLE);
	m_Controls.add(8,0, "genre",			IDC_GENRES, IDC_GENRESLABEL);
	m_Controls.add(8,1, "artist",			IDC_ARTISTS, IDC_ARTISTSLABEL);
	m_Controls.add(8,2, "album",			IDC_ALBUMS, IDC_ALBUMSLABEL);
	m_Controls.add(8,3, "songs",			IDC_SONGS, IDC_SONGSLABEL);
	m_Controls.add(9,0, "player status",	IDC_PLAYER_STATUS);

	m_Controls.add(2,0, "pos slider",		IDC_POSITION_SLIDER);

	int labelheight=0;
	int textheight=0;

//	double PlaylistHeightPct = m_Config.getPlaylistHeightPct();
	double GenreWidthPct = m_Config.getGenreWidthPct();
	double LibraryWidthPct = (1 - GenreWidthPct) / 3;
	int fixedy = bordervert * 3;
	int y = border;
	int x = border;
	int rowMaxY = 0;

	logger.log("resizeControls 0");

	labelheight = m_ArtistsLabel.GetItemHeight();
	textheight = m_Artists.GetItemHeight(0);

	labelheight += 2;

	Control * p = m_Controls.getObj(IDC_OPTIONS_BUTTON);
	rowMaxY = __max(rowMaxY, y + p->height);
	m_Controls.move(p, x, y, p->row, p->col);

	CDIBSectionLite bmpaplabel;
	bmpaplabel.Load(m_Config.getSkin(MB_SKIN_BUTTONAPPLABELOUT));
	rowMaxY = __max(rowMaxY, y + bmpaplabel.GetHeight());
	
	p = m_Controls.getObj(IDC_BUTTON_EXIT);
	x = m_Controls.dialogrect.Width()
		- (border + p->width);
	rowMaxY = __max(rowMaxY, y + p->height);
	m_Controls.move(p, x, y, p->row, p->col);

	p = m_Controls.getObj(IDC_BUTTON_MAXIMIZE);
	x -= (borderhorz + p->width);
	rowMaxY = __max(rowMaxY, y + p->height);
	m_Controls.move(p, x, y, p->row, p->col);

	p = m_Controls.getObj(IDC_BUTTON_MINIMIZE);
	x -= (borderhorz + p->width);
	rowMaxY = __max(rowMaxY, y + p->height);
	m_Controls.move(p, x, y, p->row, p->col);	
	int playpaneltop = rowMaxY;

	p = m_Controls.getObj(IDC_BUTTON_RESIZE);
	x -= (borderhorz + p->width);
	rowMaxY = __max(rowMaxY, y + p->height);
	m_Controls.move(p, x, y, p->row, p->col);	
	playpaneltop = rowMaxY;

// control box

	y = panelborder + rowMaxY + bordervert;
	x = border + panelborder;
	int ControlBoxLeft = x;
	int ControlBoxTop  = y;
	int ControlBoxRight = x + ControlBoxWidth;
	int ControlBoxBottom = y + ControlBoxHeight;

	x = ControlBoxLeft + regSD.Read("StopX",0);
	y = ControlBoxTop + regSD.Read("StopY", 0);
	p = m_Controls.getObj(IDC_BUTTON_STOP);
	int maxX = x + p->width;
	rowMaxY = y + p->height;
	m_Controls.move(p, x, y, p->row, p->col);	

	int stopy = y;

	x = ControlBoxLeft + regSD.Read("PlayX",0);
	y = ControlBoxTop + regSD.Read("PlayY", 0);
	p = m_Controls.getObj(IDC_BUTTON_PLAY);
	maxX = __max(maxX, x + p->width);
	rowMaxY = __max(rowMaxY, y + p->height);
	m_Controls.move(p, x, y, p->row, p->col);	

	x = ControlBoxLeft + regSD.Read("PauseX",0);
	y = ControlBoxTop + regSD.Read("PauseY", 0);
	p = m_Controls.getObj(IDC_BUTTON_PAUSE);
	maxX = __max(maxX, x + p->width);
	rowMaxY = __max(rowMaxY, y + p->height);
	m_Controls.move(p, x, y, p->row, p->col);	
	int pauseright = x + p->width;
	
	y = rowMaxY;

	p = m_Controls.getObj(IDC_BUTTON_REVERSE);
	x = ControlBoxLeft + regSD.Read("ReverseX",0);
	y = ControlBoxTop + regSD.Read("ReverseY", 0);
	rowMaxY = __max(rowMaxY, y + p->height);
	m_Controls.move(p, x, y, p->row, p->col);	

	x += p->width;
	p = m_Controls.getObj(IDC_BUTTON_FASTFORWARD);
	x = ControlBoxLeft + regSD.Read("ForwardX",0);
	y = ControlBoxTop + regSD.Read("ForwardY", 0);
	maxX = __max(maxX, x + p->width);
	rowMaxY = __max(rowMaxY, y + p->height);
	m_Controls.move(p, x, y, p->row, p->col);	
	int ffright = x + p->width;

	y = rowMaxY;
	x = border + panelborder;
	p = m_Controls.getObj(IDC_BUTTON_RANDOM);
	int randomy = y;
	x = ControlBoxLeft + regSD.Read("RandomX",0);
	y = ControlBoxTop + regSD.Read("RandomY", 0);
	rowMaxY = __max(rowMaxY, y + p->height);
	maxX = __max(maxX, x + p->width);
	m_Controls.move(p, x, y, p->row, p->col);

	y += p->height;
	p = m_Controls.getObj(IDC_BUTTON_SHUFFLE);
	x = ControlBoxLeft + regSD.Read("ShuffleX",0);
	y = ControlBoxTop + regSD.Read("ShuffleY", 0);
	maxX = __max(maxX, x + p->width);
	m_Controls.move(p, x, y, p->row, p->col);
	
	y += p->height;
	p = m_Controls.getObj(IDC_BUTTON_CLEAR);
	x = ControlBoxLeft + regSD.Read("ClearX",0);
	y = ControlBoxTop + regSD.Read("ClearY", 0);
	maxX = __max(maxX, x + p->width);
	m_Controls.move(p, x, y, p->row, p->col);
	
	y += p->height;
	p = m_Controls.getObj(IDC_BUTTON_LOAD);
	x = ControlBoxLeft + regSD.Read("LoadX",0);
	y = ControlBoxTop + regSD.Read("LoadY", 0);
	maxX = __max(maxX, x + p->width);
	m_Controls.move(p, x, y, p->row, p->col);
	
	y += p->height;
	p = m_Controls.getObj(IDC_BUTTON_SAVE);
	x = ControlBoxLeft + regSD.Read("SaveX",0);
	y = ControlBoxTop + regSD.Read("SaveY", 0);
	maxX = __max(maxX, x + p->width);
	m_Controls.move(p, x, y, p->row, p->col);
	int savebottom = y + p->height;

	p = m_Controls.getObj(IDC_VOLUME_SLIDER);
	x = maxX ;//+ borderhorz;
	y = stopy;
	x = ControlBoxLeft + regSD.Read("VolumeX",0);
	y = ControlBoxTop + regSD.Read("VolumeY", 0);
	m_Controls.move(p, x, y, p->row, p->col);	
	int volright = x + p->width;
	int volbottom = y + p->height;

	p = m_Controls.getObj(IDC_POSITION_SLIDER);
	x = ControlBoxLeft + regSD.Read("ProgressX",0);
	y = ControlBoxTop + regSD.Read("ProgressY", 0);
	m_Controls.move(p, x, y, p->row, p->col);	
	
	y = ControlBoxTop;
	p = m_Controls.getObj(IDC_PICTURE_CTRL);

	p->height = ControlBoxHeight;
	p->width = p->height;
	int pheight = p->height;
	x = m_Controls.dialogrect.Width() - (border + p->width + panelborder);
	m_Controls.move(p,x,y,p->row,p->col);
	int picbottom = y + p->height;
	int picleft = x;

	int plmaxy = __max(ControlBoxBottom,volbottom);
	plmaxy = __max(plmaxy,picbottom);
	int plmaxx = __max(ffright,volright);
	plmaxx = __max(plmaxx,ControlBoxRight) + borderhorz;

	p = m_Controls.getObj(IDC_PLAYLIST);
	p->height = plmaxy - (labelheight + textheight + bordervert
		+ stopy);
	p->height = ControlBoxHeight - (labelheight + textheight + bordervert);
	p->labelheight = labelheight;
	x = plmaxx;
	p->width = (picleft - (((2 * panelborder) + borderhorz))) - x;
	m_Controls.move(p,x,y,p->row,p->col);
	int playlistright = x + p->width;
	int playlistleft = x;
	int playlistbottom = y + p->height + labelheight;
	
	y = playlistbottom + bordervert;
	int pwidth = (x + p->width) - (border + panelborder);

	int posy = __max(ControlBoxBottom,volbottom);
	posy = __max(posy, playlistbottom) + bordervert;
					 
	p = m_Controls.getObj(IDC_POSITION_LABEL);
	CSize s = m_PositionLabel.GetSize("000000000");
	p->width = s.cx;
	p->height = textheight;
	x = border + panelborder; 
	x = playlistleft;
//	y = posy;
	int poslabely = y;
	m_Controls.move(p, x, y, p->row, p->col);	

	x = x + p->width + borderhorz;
	p = m_Controls.getObj(IDC_CURRENT_TITLE);
	p->height = textheight;
	p->width = playlistright - x;
	m_Controls.move(p,x,y,p->row,p->col);
	int titlebottom = y + p->height;
	
	y += p->height + bordervert;
	int genrey = y + (panelborder * 2);

	p = m_Controls.getObj(IDC_PLAYER_STATUS);
	p->height = labelheight;
	x = border + panelborder;
	p->width = m_Controls.dialogrect.Width()
		- (2 * (border + panelborder));
//	p->setRects();
	y = m_Controls.dialogrect.Height() 
		- (labelheight + border + panelborder);
	m_Controls.move(p,x,y,p->row,p->col);
	int statusbottom = y + p->height;

	p = m_Controls.getObj(IDC_GENRES);		
	p->width = (m_Controls.dialogrect.Width()
		- (2 * (panelborder + border))) * GenreWidthPct;
	p->labelheight = labelheight;
	p->height = (y - bordervert) - (genrey + labelheight);
	y = genrey;
	x = border + panelborder;
//	p->setRects();
	m_Controls.move(p,x,y,p->row,p->col);
	
	pheight = p->height;
	x += p->width + borderhorz;

	p = m_Controls.getObj(IDC_ARTISTS);
	p->width = (m_Controls.dialogrect.Width()
		- (2 * (panelborder + border))) * LibraryWidthPct;
	p->labelheight = labelheight;
	p->height = pheight;
//	p->setRects();
	m_Controls.move(p,x,y,p->row,p->col);

	pwidth = p->width;
	x += p->width + borderhorz;
	
	p = m_Controls.getObj(IDC_ALBUMS);
	p->width = pwidth;
	p->labelheight = labelheight;
	p->height = pheight;
//	p->setRects();
	m_Controls.move(p,x,y,p->row,p->col);

	x += p->width + borderhorz;
	
	p = m_Controls.getObj(IDC_SONGS);
	p->width = m_Controls.dialogrect.Width() - (x + border + panelborder);
	p->labelheight = labelheight;
	p->height = pheight;
//	p->setRects();
	m_Controls.move(p,x,y,p->row,p->col);
	
	CDC * cdc = GetDC();

	CDialogClassImpl::FreeResources();
	
	logger.log("resizeControls 1");

	// dialog
	CString bgmain = m_Config.getSkin(MB_SKIN_BACKGROUNDMAIN);
	if (bgmain.GetLength()) {
		CRect rect0,rect1;
		GetWindowRect(rect0);
		ScreenToClient(rect0);
		
		rect0.OffsetRect(3,3);
#pragma hack		
		// hack alert: rect0 needs 2 b Window instead of client
		// for the resizing border
		SetBitmap(bgmain, rect0, LO_STRETCH, CS("frame"));

	}

	// applabel
	x = (m_Controls.dialogrect.Width() / 2) - (bmpaplabel.GetWidth() / 2);
	y = border;
	CRect aplrect(x,y,x+bmpaplabel.GetWidth(),y+bmpaplabel.GetHeight());
	SetBitmap(m_Config.getSkin(MB_SKIN_BUTTONAPPLABELOUT), aplrect, LO_DEFAULT,
		CS("applabel"));

	// l,t,r,b
	// playlist panel
	CRect plrect(m_Controls.dialogrect.left + border,
		m_Controls.dialogrect.top + playpaneltop + bordervert,
		playlistright + panelborder,
		titlebottom + panelborder);
	CString bgpanel = m_Config.getSkin(MB_SKIN_BACKGROUNDPLAYLIST);
	if (bgpanel.GetLength()) {
		SetBitmap(bgpanel, plrect, LO_STRETCH, CS("play panel"));
	}

	// lib panel
	CRect librect(plrect);
	librect.top = genrey-panelborder;
	librect.bottom = statusbottom + panelborder;
	librect.right = m_Controls.dialogrect.right - border;
	bgpanel = m_Config.getSkin(MB_SKIN_BACKGROUNDLIBRARY);
	if (bgpanel.GetLength()) {
		SetBitmap(bgpanel, librect, LO_STRETCH, CS("lib panel"));
	}

	// album panel
	CRect alrect(plrect);
	alrect.left = plrect.right + borderhorz;
	alrect.right = librect.right;
	bgpanel = m_Config.getSkin(MB_SKIN_BACKGROUNDALBUMART);
	if (bgpanel.GetLength()) {
		SetBitmap(bgpanel, alrect, LO_STRETCH, CS("album"));
	}

	// button background
	CRect buttonrect;
	buttonrect.left = ControlBoxLeft;
	buttonrect.top = ControlBoxTop;
	buttonrect.right = ControlBoxLeft + ControlBoxWidth;
	buttonrect.bottom = ControlBoxTop + ControlBoxHeight;
	bgpanel = m_Config.getSkin(MB_SKIN_BUTTONBACKGROUND);
	if (bgpanel.GetLength()) {
		SetBitmap(bgpanel, buttonrect, LO_DEFAULT, CS("controlbox"));
	}

	make(cdc); // the background

	logger.log("resizeControls 2");
	m_Need2Erase = TRUE;

	OnNcPaint() ;
	OnEraseBkgnd(cdc); // force CDialogSK to paint background

	// This sends a WM_NCPAINT to repaint the resize frame
	RedrawWindow(NULL,NULL, RDW_FRAME|RDW_INVALIDATE);

	COLORREF transclr = RGB(255,0,0);

	m_InitDone = TRUE;

	m_Genres.SetBitmaps(cdc, m_Config.getSkin(MB_SKIN_BACKGROUNDTITLES),transclr,
	m_Config.getSkin(MB_SKIN_SCROLLUPARROW),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLDOWNARROW),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLBUTTON),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLBACKGROUND),transclr);
	m_Artists.SetBitmaps(cdc, m_Config.getSkin(MB_SKIN_BACKGROUNDTITLES),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLUPARROW),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLDOWNARROW),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLBUTTON),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLBACKGROUND),transclr);
	m_Albums.SetBitmaps(cdc, m_Config.getSkin(MB_SKIN_BACKGROUNDTITLES),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLUPARROW),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLDOWNARROW),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLBUTTON),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLBACKGROUND),transclr);
	m_Songs.SetBitmaps(cdc, m_Config.getSkin(MB_SKIN_BACKGROUNDTITLES),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLUPARROW),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLDOWNARROW),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLBUTTON),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLBACKGROUND),transclr);
	m_Playlist.SetBitmaps(cdc, 
		m_Config.getSkin(MB_SKIN_BACKGROUNDTITLES),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLUPARROW),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLDOWNARROW),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLBUTTON),transclr,
		m_Config.getSkin(MB_SKIN_SCROLLBACKGROUND),transclr);

	m_PositionSlider.SizeToContent(this);
	m_VolumeSlider.SizeToContent(this);

    m_brush.DeleteObject();
    m_brush.CreateSolidBrush(m_Config.getColorBkNormal());

	ReleaseDC(cdc);
	
	static int counter = 0;
	counter++;
	CString msg("resetControls ");
	msg += numToString(counter) + "\r\n";
	OutputDebugString(msg);

}

void
CPlayerDlg::init() {

    if (m_mlib.init()) {
		MBMessageBox("Error", "Database corrupted. Rebuild it by performing a\r\nScan in Options/Configuration");
		PlayerStatusSet(CString(
			"Database corrupted. Do a scan in Configuration."));
	}
    initDb();
}

void
CPlayerDlg::initDb() {
	//	IRReaderStop();
    if (m_Config.UseGenre()) m_Genres.ResetContent();
    m_Songs.ResetContent();
    m_Albums.ResetContent();
    m_Artists.ResetContent();

	if (m_Config.UseGenre()) {
		m_mlib.getGenres(m_Genres);
		if (_selectedGenre == ""
				|| m_Genres.SelectString(0, _selectedGenre) == LB_ERR) {
			m_Genres.SetCurSel(0);
		}
		m_Artists.SetCurSel(0);
		m_Albums.SetCurSel(0);
		OnSelchangeGenres() ;
	} else {
		_selectedGenre = MBALL;
		m_mlib.getArtists(_selectedGenre, m_Artists);
		m_Artists.SetCurSel(1);
		m_Albums.SetCurSel(0);
		OnSelchangeArtists() ;
	}

	//	IRReaderStart();

}

void CPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)	{
		CRect rect;
		GetWindowRect(rect);
		About dlgAbout(rect);
		dlgAbout.DoModal();
	} else if (nID == SC_RESTORE) {
		m_Need2Erase = TRUE;
		CDialogClassImpl::OnSysCommand(nID, lParam);
		RedrawWindow();
    } else {
		CDialogClassImpl::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPlayerDlg::OnPaint() 
{
//	CString msg="CPlayerDlg OnPaint\r\n"; OutputDebugString(msg);
	CPaintDC dc(this); // device context for painting


	CDialogClassImpl::OnPaint();
//	msg="CPlayerDlg OnPaint done\r\n"; OutputDebugString(msg);
}




// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPlayerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CPlayerDlg::DestroyWindow() 
{
	// delete the Xaudio player
    if (m_Player) {
        Stop();
    }

	//	IRReaderStop();
	return CDialogClassImpl::DestroyWindow();
}

void CPlayerDlg::OnOpenFileButton() 
{
    // open a file
	CFileDialog *dialog;

	dialog = new CFileDialog(TRUE, 
							"mp3",
							"",
							OFN_FILEMUSTEXIST    | 
							OFN_HIDEREADONLY     |
							OFN_EXPLORER,
							"MPEG Audio Files {*.mpg;*.mp1;*.mp2;*.mp3}|*.mpg;*.mp1;*.mp2;*.mp3|All Files {*.*}|*.*||");

	int ret;
	ret = dialog->DoModal();
	if (ret == IDOK) {
        // a file was selected
		m_Player->InputOpen(dialog->GetPathName());
	}

	delete dialog;
	
}

#ifdef asdf
void CPlayerDlg::OnOpenUrlButton() 
{
	CUrlDlg *dialog;

	dialog = new CUrlDlg();

	int ret;
	ret = dialog->DoModal();
	if (ret == IDOK) {
		// an URL was entered
		m_Player->InputOpen(dialog->m_URL);
	}
	delete dialog;
}
#endif
void CPlayerDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
OnHScroll(nSBCode, nPos, pScrollBar) ;
}
void CPlayerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	return;
	if (pScrollBar->m_hWnd == m_PositionSlider.m_hWnd) {
		switch (nSBCode) {
			case SB_ENDSCROLL: 
				adjustPosition();
				break;
			case SB_THUMBPOSITION: 
				adjustPosition(nPos);
				break;
		}
		CDialogClassImpl::OnHScroll(nSBCode, nPos, pScrollBar);
	} else 
	if (pScrollBar->m_hWnd == m_VolumeSlider.m_hWnd) {
		switch (nSBCode) {
			case SB_ENDSCROLL: 
				adjustVolume();
				break;
			case SB_THUMBPOSITION: 
			case SB_THUMBTRACK:
				adjustVolume(nPos);
//				OutputDebugString("vol nPos " + numToString(nPos) + "\r\n");
				break;
		}
		CDialogClassImpl::OnVScroll(nSBCode, nPos, pScrollBar);
	}
}

void CPlayerDlg::OnAbout() 
{
		CRect rect;
		GetWindowRect(rect);
		About dlgAbout(rect);
		dlgAbout.DoModal();
}

void CPlayerDlg::OnControlSelChange() {
    if (m_Control->ptr() == &m_Genres) {
        OnSelchangeGenres();
    } else if (m_Control->ptr() == &m_Artists) {
        OnSelchangeArtists();
    } else if (m_Control->ptr() == &m_Albums) {
        OnSelchangeAlbums();
    } else if (m_Control->ptr() == &m_Songs) {
        OnSelchangeSongs();
    } else if (m_Control->ptr() == &m_Playlist) {
        OnSelchangePlaylist();
    }
}

void CPlayerDlg::OnControlClick() {
    if (m_Control->ptr() == &m_Genres) {
        OnDblclkGenres() ;
    } else if (m_Control->ptr() == &m_Artists) {
        OnDblclkArtists() ;
    } else if (m_Control->ptr() == &m_Albums) {
        OnDblclkAlbums() ;
    } else if (m_Control->ptr() == &m_Songs) {
        OnDblclkSongs();
    } else if (m_Control->ptr() == &m_Playlist) {
        OnDblclkPlaylist();

//    } else if (m_Control->ptr() == m_MenuDlg) {
//        m_MenuDlg->EndDialog(m_MenuDlg->currentbutton());
    }
}
static void
rememberSelections(CMapStringToString & map, CString sel,
                   CExtendedListBox & box) {
    CString val;
    if (map.Lookup(sel, val) == 0 || box.SelectString(0, val) == LB_ERR) {
        box.SetCurSel(0);
    }
#ifdef asdf
    if (map.Lookup(sel, val) == 0) {
        box.SetCurSel(0);
    } else {
        int sel = box.GetCurSel();
        if (box.SelectString(0, val) == LB_ERR) {
            box.SetCurSel(sel);
        }
    }
#endif
}

void CPlayerDlg::OnSelchangeGenres() 
{
	static int lastsel = -1;
	UpdateWindow();

	int sel = m_Genres.GetCurSel();
    if (sel < 0) return;
	if (lastsel != sel) {
		m_LastThingQueuedUp = "";
	}
	lastsel = sel;
	m_Genres.GetText(sel, _selectedGenre);
	m_Artists.ResetContent();	
	m_mlib.getArtists(_selectedGenre, m_Artists);
//	m_Artists.SetCurSel(0);	
    rememberSelections(m_GenreArtist, _selectedGenre, m_Artists);
	OnSelchangeArtists();
//	if (_selectedGenre == MBALL) {
//		PlayerStatusTempSet(m_mlib.getLibraryCounts());
//	} else {
//		PlayerStatusTempSet(_selectedGenre);
//	}

    m_Genres.invalidate();
}

void CPlayerDlg::OnSelchangeArtists() 
{
	static int lastsel = -1;
	int sel = m_Artists.GetCurSel();
    if (sel < 0) return;
	if (lastsel != sel) {
		m_LastThingQueuedUp = "";
	}
	lastsel = sel;
	m_Artists.GetText(sel,_selectedArtist);
    if (m_Config.UseGenre()) {
		m_GenreArtist.SetAt(_selectedGenre, _selectedArtist);
	}

	m_Albums.ResetContent();	
	m_mlib.getAlbums(_selectedGenre,
		_selectedArtist, m_Albums, m_Config.AlbumSortAlpha());
//	m_Albums.SetCurSel(0);	
    rememberSelections(m_ArtistAlbum, _selectedArtist, m_Albums);
	OnSelchangeAlbums();
//	if (!m_Config.UseGenre() && _selectedArtist == MBALL) {
//		PlayerStatusTempSet(m_mlib.getLibraryCounts());
//	} else {
//		PlayerStatusTempSet(_selectedArtist);		
//	}
    m_Artists.invalidate();
}

void CPlayerDlg::OnSelchangeAlbums() 
{
	static int lastsel = -1;
	int sel = m_Albums.GetCurSel();
    if (sel < 0) return;
	if (lastsel != sel) {
		m_LastThingQueuedUp = "";
	}
	lastsel = sel;
	m_Albums.GetText(sel,_selectedAlbum);
    m_ArtistAlbum.SetAt(_selectedArtist, _selectedAlbum);

	m_Songs.ResetContent();	
	m_mlib.getSongs(_selectedGenre,
		_selectedArtist,
		_selectedAlbum,m_Songs);
//	m_Songs.SetCurSel(0);	
	rememberSelections(m_AlbumSong, _selectedAlbum, m_Songs);
    OnSelchangeSongs();
//    PlayerStatusTempSet(_selectedAlbum);
    m_Albums.invalidate();
}

void CPlayerDlg::OnSelchangeSongs() 
{
	static int lastsel = -1;
	static CString lastalbum;
	int sel = m_Songs.GetCurSel();
    if (sel < 0) return;
	if (lastsel != sel) {
		m_LastThingQueuedUp = "";
	}
	lastsel = sel;
	m_Songs.GetText(sel,_selectedSong);
    m_AlbumSong.SetAt(_selectedAlbum, _selectedSong);

    OnSelchangePlaylist();
//    PlayerStatusTempSet(_selectedSong);	
    m_Songs.invalidate();

	if (lastalbum != _selectedAlbum) {
		Song song = m_mlib.getSong(_selectedGenre, _selectedArtist,
			_selectedAlbum, _selectedSong);
		CString file = song->getId3("FILE");
		m_ArtOwnedBy = AOTLibrary;
		displayAlbumArt(file);
	}
	lastalbum = _selectedAlbum;
}

void CPlayerDlg::OnSelchangePlaylist() 
{
	int sel = m_Playlist.GetCurSel();
    if (sel < 0) return;
	m_Playlist.GetText(sel,_selectedPlaylistSong);
//    PlayerStatusTempSet(_selectedPlaylistSong);
    m_Playlist.invalidate();

	CString file = m_mlib._playlist[sel]->getId3("FILE");
	m_ArtOwnedBy = AOTPlaylist;
	displayAlbumArt(file);
}

void CPlayerDlg::updatePlaylist() {
	m_Playlist.ResetContent();
	m_mlib.getPlaylist(m_Playlist);
	m_Playlist.SetCurSel(m_Playlist.GetCount()-1);

	calcDuration();

	UpdateWindow();
    PlayLoop();

    m_Playlist.invalidate();
}

void CPlayerDlg::OnDblclkSongs() 
{
	CString last = _selectedGenre + _selectedArtist + _selectedAlbum +
		_selectedSong;
	if (last != m_LastThingQueuedUp) {
//		m_mlib.addSongToPlaylist(_selectedGenre, _selectedArtist, _selectedAlbum,
//			_selectedSong);

		Song song = new CSong;
	    int sel = m_Songs.GetCurSel();
		DWORD data = m_Songs.GetItemData(sel);
		song = m_mlib.m_SongLib.getSong(data);
		m_mlib.addSongToPlaylist(song);

		updatePlaylist();
		m_LastThingQueuedUp = last;
	} else {
		PlayerStatusTempSet("You just added that!");
	}
}

void CPlayerDlg::OnDblclkAlbums() 
{
	CString last = _selectedGenre + _selectedArtist + _selectedAlbum;
	if (last != m_LastThingQueuedUp) {
		m_mlib.addAlbumToPlaylist(_selectedGenre, _selectedArtist, _selectedAlbum);
		updatePlaylist();
		m_LastThingQueuedUp = last;
	} else {
		PlayerStatusTempSet("You just added that!");
	}
}

void CPlayerDlg::OnDblclkArtists() 
{
	if (_selectedArtist == MBALL) return;
	CString last = _selectedGenre + _selectedArtist;
	if (last != m_LastThingQueuedUp) {
		m_mlib.addArtistToPlaylist(_selectedGenre, _selectedArtist);
		updatePlaylist();
		m_LastThingQueuedUp = last;
	} else {
		PlayerStatusTempSet("You just added that!");
	}
}

void CPlayerDlg::OnDblclkGenres() 
{
	if (_selectedGenre == MBALL) return;
	CString last = _selectedGenre;
	if (last != m_LastThingQueuedUp) {
		m_mlib.addGenreToPlaylist(_selectedGenre);	
		updatePlaylist();
		m_LastThingQueuedUp = last;
	} else {
		PlayerStatusTempSet("You just added that!");
	}
}
void CPlayerDlg::OnDblclkPlaylist() 
{
    int sel = m_Playlist.GetCurSel();
    m_PlaylistCurrent = sel -1;
    Stop();
	m_StopFlag = FALSE;

    PlayLoop();

}

BOOL CPlayerDlg::PreTranslateMessage(MSG* pMsg)
{	// disable ESC & ENTER from killing the dialog
    if (pMsg->message == WM_KEYDOWN) {
        if (pMsg->wParam == VK_RETURN) {
			OnControlClick();
            return TRUE;
		} else if (pMsg->wParam == VK_ESCAPE) {
			return TRUE;
        } else if (pMsg->wParam == VK_DELETE) {
            OnDelete();
            return TRUE;
        }
    }
    return CDialogClassImpl::PreTranslateMessage(pMsg);
}

void CPlayerDlg::OnMenuButton() 
{
    int r = -1;
    if (m_MenuDlg == 0) {
        m_MenuDlg = new CMenuDialog (this);
        *m_Dialog = m_MenuDlg;
	    r = m_MenuDlg->DoModal();
	    delete m_MenuDlg;
	    m_MenuDlg = 0;
		*m_Dialog = this;
    }

    switch (r) {
    case 0:     // exit
//        OnMenuExit();
        break;
    case 1:     // shuffle
        OnMenuShuffleplaylist();
        break;
    case 2:     // randomize
        OnMenuRandomizePlaylist();
        break;
    case 3:     // clear
        OnMenuClearplaylist();
        break;
    case 4:
        SetSavePlaylistFlag(FALSE);
        OnMenuSaveplaylist();
        break;
    case 5:      // load
        OnMenuLoadplaylist();
        break;
    case 6:     // pause
        OnMenuPause();
        break;
    case 7:     // play
        OnMenuPlay();
        break;
    case 8:     // stop
        OnMenuStop();
        break;
    case 9:     // help
        OnMenuHelp();
    }
}

void
CPlayerDlg::OnGenresFocus( ) {
	*m_Control = (CExtendedListBox*)GetDlgItem(IDC_GENRES);
    PlayerStatusTempSet(_selectedGenre);
	m_ArtOwnedBy = AOTLibrary;
}
void
CPlayerDlg::OnArtistsFocus() {
	*m_Control = (CExtendedListBox*)GetDlgItem(IDC_ARTISTS);
    PlayerStatusTempSet(_selectedArtist);
	m_ArtOwnedBy = AOTLibrary;
}
void
CPlayerDlg::OnAlbumsFocus() {
	*m_Control = (CExtendedListBox*)GetDlgItem(IDC_ALBUMS);
    PlayerStatusTempSet(_selectedAlbum);
	m_ArtOwnedBy = AOTLibrary;
}
void
CPlayerDlg::OnSongsFocus() {
	*m_Control = (CExtendedListBox*)GetDlgItem(IDC_SONGS);
    PlayerStatusTempSet(_selectedSong);
	m_ArtOwnedBy = AOTLibrary;
}
void
CPlayerDlg::OnPlaylistFocus() {
	*m_Control = (CExtendedListBox*)GetDlgItem(IDC_PLAYLIST);
    PlayerStatusTempSet(_selectedPlaylistSong);
	m_ArtOwnedBy = AOTPlaylist;
}
void
CPlayerDlg::OnDelete() {
    if (m_Control->ptr() == &m_Playlist) {
	    int sel = m_Playlist.GetCurSel();
        if (sel < 0) { return; }
        if (sel == m_PlaylistCurrent) {
            Stop();
            m_PlaylistCurrent = sel-1;
			PostMessage(WM_PLAYLOOP, (WPARAM)0, (LPARAM)0);
        }
        void  * song = m_Playlist.GetItemDataPtr(sel);
        m_Playlist.DeleteString(sel);
        m_mlib.deleteSongFromPlaylist((PlaylistNode*)song);
        if (m_Playlist.GetCount()) {
            if (sel+1 > m_Playlist.GetCount()) {
                sel--;
                if (sel < 0) sel = 0;
            }
            m_Playlist.SetCurSel(sel);
            m_Playlist.GetText(sel,_selectedPlaylistSong);
            PlayerStatusTempSet(_selectedPlaylistSong);
            UpdateData(TRUE);
            m_mlib.dumpPL(m_PlaylistCurrent);
        } else {
//            PlayerStatusSet(m_mlib.getLibraryCounts());
        }
		calcDuration();
    }
}

void
CPlayerDlg::OnPageUp() {
    if (!m_Control->isList()) {
        return;
    }

    CRect wrect;
    ((CExtendedListBox*)m_Control->ptr())->GetWindowRect( wrect ) ;
    int iheight = ((CExtendedListBox*)m_Control->ptr())->GetItemHeight(0);

    int pagelen = wrect.Height() / iheight;
    m_Control->up(pagelen-1);
    OnControlSelChange();
}

void
CPlayerDlg::OnPageDown() {
    if (!m_Control->isList()) {
        return;
    }

    CRect wrect;
    ((CExtendedListBox*)m_Control->ptr())->GetWindowRect( wrect ) ;
    int iheight = ((CExtendedListBox*)m_Control->ptr())->GetItemHeight(0);

    int pagelen = wrect.Height() / iheight;
    m_Control->down(pagelen-1);
    OnControlSelChange();
}

// WinNT, not 98, not XP
BOOL CALLBACK KillScreenSaverFunc (HWND hWnd, LPARAM lParam){
    PostMessage(hWnd,WM_CLOSE,0,0);

    return TRUE;
}

// Need to try GetVersionEx() to determine version of windows running
// also try keybd_event and SendInput
// PostMessage (GetForegroundWindow(), WM_CLOSE, 0, 0L);

BOOL WakeUp (){
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	BOOL bIsWindowsNT =
		(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);

//    if(bIsWindowsNT) {
        HDESK hDesktop;

//        hDesktop=OpenDesktop("screen-saver",0,FALSE,MAXIMUM_ALLOWED);
		hDesktop=OpenDesktop("screen-saver",0,FALSE,DESKTOP_WRITEOBJECTS);
        if (hDesktop!=NULL) {
            EnumDesktopWindows(hDesktop,(WNDENUMPROC)KillScreenSaverFunc,0);
            CloseDesktop(hDesktop);
        }
//    } else {
        keybd_event(VK_SHIFT,0,0,0);
	    keybd_event(VK_SHIFT,0,KEYEVENTF_KEYUP,0);
//    }

//    PostMessage (GetActiveWindow(), WM_KEYDOWN, VK_SHIFT, 0L);
//    PostMessage (GetActiveWindow(), WM_KEYUP, VK_SHIFT, 0L);
	return true;
}

#ifdef asdf
BOOL CALLBACK KillScreenSaverFunc(HWND hwnd, LPARAM lParam)
{
  if(IsWindowVisible(hwnd))
      PostMessage(hwnd, WM_CLOSE, 0, 0);
  return TRUE;
}

bool EndScreenSaver()
{
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	BOOL bIsWindowsNT =
		(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);

	if(!bIsWindowsNT)
	{
		HWND hWnd = FindWindow("WindowsScreenSaverClass",
NULL); 
 
		if (hWnd) {
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
	}
	else
	{
		HDESK hdesk;

		hdesk = OpenDesktop(TEXT("Screen-saver"),
						   0,
						   FALSE,
						   DESKTOP_READOBJECTS
| DESKTOP_WRITEOBJECTS);
		if (hdesk)
		{
		  EnumDesktopWindows(hdesk, KillScreenSaverFunc, 0);
		  CloseDesktop(hdesk);
		} 
	}



    return TRUE;
}
#endif

LRESULT
CPlayerDlg::OnGraphNotify(UINT wParam, LONG lParam) {
	m_Player->HandleGraphEvent();
	return 0;
}

LRESULT
CPlayerDlg::OnPlayloop(UINT wParam, LONG lParam) {
	PlayLoop();
	return 0;
}

int
CPlayerDlg::HandleIRMessage(int key) {

//    EndScreenSaver();

	int wakeitup = 1;
    switch (key) {
	case IR_MESSAGE_UP: {
        m_Dialog->control().up(1);
		m_Dialog->OnControlSelChange();
		break;}
	case IR_MESSAGE_DOWN: {
        m_Dialog->control().down(1);
		m_Dialog->OnControlSelChange();
		break;}
	case IR_MESSAGE_LEFT: {
//		Tab to previous control
		m_Dialog->PrevDlgCtrl();
		break; }
	case IR_MESSAGE_RIGHT: {
//		Tab to next control
		m_Dialog->NextDlgCtrl();
		break; }
    case IR_MESSAGE_SELECT: {
        m_Dialog->OnControlClick();
        break;}
    case IR_MESSAGE_MENU: {
        m_Dialog->OnMenuButton();
        break;}
    case IR_MESSAGE_EXIT: {
        m_Dialog->OnExit();
        break;}
    case IR_MESSAGE_DELETE: {
        m_Dialog->OnDelete();
        break; }
	case IR_MESSAGE_VOLUP:
		OnVolUp();
		break;
	case IR_MESSAGE_VOLDOWN:
		OnVolDown();
		break;
    case IR_MESSAGE_PAGEUP: {
        m_Dialog->OnPageUp();
        break; }
    case IR_MESSAGE_PAGEDOWN: {
        m_Dialog->OnPageDown();
        break; }
    case IR_MESSAGE_PLAY: {
        OnMenuPlay();
        break;}
	case IR_MESSAGE_PAUSE: {
        OnMenuPause();
        break;}
	case IR_MESSAGE_STOP: {
        OnMenuStop();
        break;}
	case IR_MESSAGE_NEXT: {
        OnNextSong();
        break;}
	case IR_MESSAGE_PREVIOUS: {
        OnPreviousSong();
        break;}
	case IR_MESSAGE_FASTFORWARD: {
		OnFastForward();
		break;}
	case IR_MESSAGE_REVERSE: {
		OnReverse();
		break;}
	case IR_MESSAGE_ALPHAUP: {
        m_Dialog->control().alphaUp();
		m_Dialog->OnControlSelChange();
		break;}
	case IR_MESSAGE_ALPHADOWN: {
        m_Dialog->control().alphaDown();
		m_Dialog->OnControlSelChange();
		break;}
	default:
		wakeitup = 0;
	}

	if (wakeitup) {
	    WakeUp();
	    if (IsIconic()) {
			ShowWindow(SW_RESTORE);
			return 0;
		}
	}
    return 0;
}

afx_msg LRESULT
CPlayerDlg::OnSerialMsg (WPARAM wParam, LPARAM lParam) {
	int key;
	BOOL more = TRUE;
	while (more) {
		more = irman().HandleSerialMsg(wParam, lParam, key);
		HandleIRMessage(key);
	}
	return 0;
}

void
PostMyIdleMessage() {
    // Post it once a second
//	OutputDebugString("PostMyIdleMessage");
    CTime t = CTime::GetCurrentTime();
    static int last = t.GetSecond();
    if (t.GetSecond() != last) {
        last = t.GetSecond();
        HWND hwnd = AfxGetMainWnd()->m_hWnd;
//		OutputDebugString("PostMessage PostMyIdleMessage");
//	    ::PostMessage(hwnd, MB_POST_MYIDLE_MESSAGE, (WPARAM)0, (LPARAM)0);
    }
}
LRESULT
CPlayerDlg::OnPostMyIdle(UINT wParam, LONG lParam) {
//	OutputDebugString("OnPostMyIdle");
    PlayerStatusRevert();
    return 0;
}

// Configuration dlg
void CPlayerDlg::OnMenuOptions() {
	//    IRReaderStop();
	irman().Close();
	m_Config.DoModal();
	m_AlbumArt = m_Config.getSkin(MB_SKIN_ALBUMART);

    PlayerStatusTempSet(m_mlib.getLibraryCounts());
//	resizeControls();
    redraw();
	//    IRReaderStart();
	irman().Close();
	irman().init(RegKeyIrman, IR_MESSAGE_NUMBER_OF, this);
	irman().Open();

	CString menuFunc,menuDesc;
	irman().getDescs(IR_MESSAGE_MENU, menuFunc, menuDesc);
	if (menuDesc == "") menuDesc = "Menu";
	m_HelpMsg = "Press "; m_HelpMsg += menuDesc; m_HelpMsg += " for options.";
	if (m_Config.trialMode() == 1) {
		m_HelpMsg += " Trial Mode.";
	} else if (m_Config.trialMode() == 2) {
		m_HelpMsg = " Trial Expired.";
	}
	PlayerStatusSet(m_HelpMsg);
}
void
CPlayerDlg::redraw() {
	resetControls();
}

void CPlayerDlg::OnMenuClearplaylist() 
{
    m_mlib._playlist.reset();
    m_PlaylistCurrent = -1;
    updatePlaylist();
    Stop();
    InputClose();
    CString libcounts = m_mlib.getLibraryCounts();
	resetPosition();
	
}
void CPlayerDlg::OnCancel() {
	StopTimers();
    saveConfig();
    EndDialog(0);
}
void
CPlayerDlg::ReadWindowPos(int & max, CRect & rect) {
    RegistryKey reg( HKEY_LOCAL_MACHINE, RegKey );
    int x1,x2,y1,y2;

    max = reg.Read(RegWindowMaximized, 0);
    x1 = reg.Read(RegWindowX1, 0);
    y1 = reg.Read(RegWindowY1, 0);
    x2 = reg.Read(RegWindowX2, 0);
    y2 = reg.Read(RegWindowY2, 0);
    CPoint tl(x1,y1);
    CPoint br(x2,y2);
    rect.TopLeft() = tl;
    rect.BottomRight() = br;
//	MoveWindow(100,100,800,600,TRUE);
//	GetClientRect(rect);

}
void
CPlayerDlg::SaveWindowPos() {
    CRect dialog;
    GetWindowRect(dialog);
    CPoint tl = dialog.TopLeft();
    CPoint br = dialog.BottomRight();

    RegistryKey reg( HKEY_LOCAL_MACHINE, RegKey );
    reg.Write(RegWindowMaximized, int(0));
//    WINDOWPLACEMENT lpwndpl;
//    BOOL s = GetWindowPlacement(&lpwndpl );
//    if (s) {
//        if (lpwndpl.showCmd == SW_SHOWMAXIMIZED) {
//            reg.Write(RegWindowMaximized, 1);
//        }
//    }
    if (IsZoomed() || IsIconic()) {
        reg.Write(RegWindowMaximized, 1);
    }

    reg.Write(RegWindowX1, tl.x);
    reg.Write(RegWindowY1, tl.y);
    reg.Write(RegWindowX2, br.x);
    reg.Write(RegWindowY2, br.y);

}

void CPlayerDlg::OnMenuExit() 
{
    if (m_MenuDlg) {
	    m_MenuDlg->EndDialog(0);
    } else {
        saveConfig();
        EndDialog(0);
    }
}
void CPlayerDlg::OnExit() 
{
    if (m_MenuDlg) {
	    m_MenuDlg->EndDialog(0);
    } else {
        saveConfig();
    }
}
void CPlayerDlg::OnMenuLoadplaylist() 
{

    LoadPlaylistDlg * m_LoadPlaylistDlg 
        = new LoadPlaylistDlg (this, &m_mlib, this);
    *m_Dialog = m_LoadPlaylistDlg;
	int r = m_LoadPlaylistDlg->DoModal();
	delete m_LoadPlaylistDlg;
	m_LoadPlaylistDlg = 0;
    *m_Dialog = this;
}

void CPlayerDlg::OnMenuSaveplaylist() 
{

    if (m_Playlist.GetCount() < 1) {
        PlayerStatusTempSet("No Playlist to save!");
        return;
    }
    if (m_SavePlaylistFlag == TRUE) {
	    SavePlaylistDlg *dialog;

	    dialog = new SavePlaylistDlg(this);

	    int ret;
	    ret = dialog->DoModal();
	    if (ret == IDOK) {
            // a file was selected
    //        CString file;
    //        dialog->GetWindowText(file);
    //		m_mlib.savePlaylist(file);
	    }

	    delete dialog;
    } else {
        AutoBuf buf(1000);
        CString name;
        CTime t = CTime::GetCurrentTime();
        name = t.Format("%y%m%d-%H%M%S");
        SavePlaylist(name);
    }
    m_SavePlaylistFlag = TRUE;

	
}

void CPlayerDlg::SavePlaylist(CString name) {
		m_mlib.savePlaylist(name);
}

void CPlayerDlg::OnMenuShuffleplaylist() 
{
	m_mlib.shufflePlaylist();
    m_Playlist.ResetContent();
	m_mlib.getPlaylist(m_Playlist);
	m_Playlist.SetCurSel(0);
	UpdateWindow();
    Stop();
    m_PlaylistCurrent = -1;
	PlayLoop();	
}

void CPlayerDlg::OnMenuRandomizePlaylist() 
{
	m_mlib.RandomizePlaylist();
    m_Playlist.ResetContent();
	m_mlib.getPlaylist(m_Playlist);
	m_Playlist.SetCurSel(0);
	UpdateWindow();
    Stop();
    m_PlaylistCurrent = -1;
	PlayLoop();	
}

void CPlayerDlg::OnMenuPause() 
{
	Pause();
	
}

void CPlayerDlg::OnMenuPlay() 
{
    OnPlayButton();
	
}

void CPlayerDlg::OnMenuStop() 
{
    OnStopButton();
	
}

void CPlayerDlg::OnMenuHelp() 
{
	CString msg,d1,d2,d3,r1,r2,r3;
	int i,n;
	msg = "Use Up,Down,Left,Right to navigate,\r\nSelect to queue up music.\r\n";
	n = irman().numOfKeys();
	AutoBuf buf(200);
	const char * HFORMAT = "%-12s %-12s | %-12s %-12s\r\n";
	sprintf(buf.p, HFORMAT,
		"Function","Remote Key", "Function", "Remote Key");
	msg += buf.p;
	msg += "-------------------------------------------------\r\n";
	int first = 1;
	for(i = 0 ; i < n ; i += 2) {
		irman().getDescs(i, d1, r1);
		irman().getDescs(i+1, d2, r2);
		sprintf(buf.p, HFORMAT,
			d1,r1,d2,r2);
		msg += buf.p;
	}
	MBMessageBox("Help", msg, FALSE);
}

void CPlayerDlg::OnNextSong() {
    Stop();
	PlayLoop();
}
void CPlayerDlg::OnPreviousSong() {
    if (m_PlaylistCurrent <= 0) {
        m_PlaylistCurrent = m_Playlist.GetCount() -2;
    } else {
        m_PlaylistCurrent -= 2;
    }
    OnNextSong();
}
BOOL CPlayerDlg::Play() {
	StartSeekTimer();
	return m_Player->Play();

}
void CPlayerDlg::Stop() {
	StopSeekTimer();
	resetPosition();
	m_Player->Stop();
	CurrentTitleSet("");
}
void CPlayerDlg::OnPlayButton() 
{
	if (m_Config.trialMode() == 1) {
		--m_PlaylistCurrent;
		PlayLoop();
		return;
	} else if (m_Config.trialMode() == 2) {
		return;
	}
	if (m_Player->isPlaying() == FALSE) {
        Play();
        m_StopFlag = FALSE;
	}
}

void CPlayerDlg::OnStopButton() 
{
	Stop();
    m_StopFlag = TRUE;
	PlayerStatusTempSet("Stopped");
}

void
CPlayerDlg::recordTLEN() {
	long pos_secs, dur_secs, pos_pct;
	m_Player->ReadMediaPosition(pos_secs, dur_secs, pos_pct);
	CString tlen = m_mlib._playlist[m_PlaylistCurrent]->getId3("TLEN");
	if (tlen == "") {
		int duration = dur_secs * 1000;
		tlen = numToString(duration);
		m_mlib.setSongVal("TLEN", tlen, 
			m_mlib._playlist[m_PlaylistCurrent]->getId3("TCON"),
			m_mlib._playlist[m_PlaylistCurrent]->getId3("TPE1"),
			m_mlib._playlist[m_PlaylistCurrent]->getId3("TALB"),
			m_mlib._playlist[m_PlaylistCurrent]->getId3("TIT2"));
	}
}

void CPlayerDlg::PlayLoop() {
    static int first = 1;
	static int trialCounter = 0;
    int good = 0;
    do {
        if (m_Player->isStopped()
                && m_mlib._playlist.size() && m_StopFlag == FALSE) {
            if (first) {
                first = 0;
            } else {
				recordTLEN();
				resetPosition();
				InputClose();
            }
            m_PlaylistCurrent++;
            if (m_PlaylistCurrent+1 > m_mlib._playlist.size()) {

                m_PlaylistCurrent = 0;
            }
			if (m_Config.trialMode() == 1) {
				++m_trialCounter;
				if (m_trialCounter > 3) {
					m_trialCounter = 0;
					PlayerStatusSet("Trial Mode Stopped. Press Play to continue.");
					return;
				}
			} else if (m_Config.trialMode() == 2) {
				PlayerStatusSet("Trial Expired.");
				return;
			}
            m_Playlist.SetCurSel(m_PlaylistCurrent);
            CString file = m_mlib._playlist[m_PlaylistCurrent]->getId3("FILE");

			if (FileUtil::IsReadable(file)) {
				good = 1;
			} else {
				good = 0;
			}
        
			CString msg;
            if (good && m_Player->InputOpen(file) && Play()) {
//				Play();
				adjustVolume();
				good = 1;
				m_ArtOwnedBy = AOTPlaylist;
				displayAlbumArt(file);
			} else {
				good = 0;
				if (!FileUtil::IsReadable(file)) {
					msg = "File is unreadable ";
					msg += file;
				} else {
					msg = "Unable to play ";
					msg += file;
				}
				CurrentTitleSet(msg);
				UpdateWindow();
				logger.log(msg);
				SetTimer(MB_PLAYLOOP_TIMER_ID, 1000, NULL);
				break;
			}

//            msg = m_mlib._playlist[m_PlaylistCurrent]->getId3("TPE1");
//            msg += " / ";
//            msg += m_mlib._playlist[m_PlaylistCurrent]->getId3("TALB");
//            msg += " / ";
            msg = m_mlib._playlist[m_PlaylistCurrent]->getId3("TIT2");
			msg += " by ";
			msg += m_mlib._playlist[m_PlaylistCurrent]->getId3("TPE1");
            CurrentTitleSet(msg);
			if (m_Config.trialMode() == 1) {
				PlayerStatusTempSet("Trial Mode. Set Password in Configuration.");
			}
            UpdateWindow();

        } else {
            good = 1;
        }
    } while (!good);
}

void
CPlayerDlg::setDuration(int duration) {
	m_PlaylistDuration = duration;
	CString msg = "Playlist ";
//	msg += "  duration = ";
	AutoBuf buf(500);
	int hour = duration / 3600;
	int sec = duration % 3600;
	int min = sec / 60;
	sec = sec % 60;
	sprintf(buf.p, "%02d:%02d:%02d", hour, min, sec);
	msg += buf.p;
	m_PlaylistLabel.setText(msg);
}
void
CPlayerDlg::addDuration(int duration) {
	m_PlaylistDuration += duration;
	setDuration(m_PlaylistDuration);
}
void
CPlayerDlg::calcDuration() {
	int size = m_mlib._playlist.size() ;
	int i;
	int totduration = 0;
	int duration;
	for(i = 0 ; i < size ; ++i) {
		duration = 0;
		CString sDuration = m_mlib._playlist[i]->getId3("TLEN");
		if (sDuration != "") {
			// TLEN is in milliseconds
			int millis = atoi((LPCTSTR)sDuration);
			if (millis > 999)
				duration = millis / 1000;
		}
		//		if (duration == 0) {
		//			CString file = m_mlib._playlist[i]->getId3("FILE");
		//			duration = m_Player->GetFileDuration(file);
		//		}
		totduration += duration;
	}
	setDuration(totduration);
}

void CPlayerDlg::LoadPlaylist(CString name) {
    m_Playlist.ResetContent();
    CString errormsg;
    int r = m_mlib.loadPlaylist(name,errormsg);
	m_mlib.getPlaylist(m_Playlist);
	m_Playlist.SetCurSel(m_Playlist.GetCount()-1);
	UpdateWindow();
    if (r != 0) {
        errormsg = "There are errors in your playlist\r\n\r\n" + errormsg;
        MBMessageBox("alert", errormsg);
    }
    PlayLoop();
}

void CPlayerDlg::OnContextMenu(CWnd* pWnd, CPoint ScreenPnt) 
{

    CPoint  point; // point inside the list ctrl where right-click occurred
    int  nIndex = 0; // index of the client that was right-clicked
    UINT uFlags = 0;

    // make sure window is active
// GetParentFrame()->ActivateFrame();

 // convert the point to the list control coordinates
    point = ScreenPnt;
    
//    m_Genres.ScreenToClient (&ClntPnt);

    CRect genreRect, artistRect, albumRect, songRect, playlistRect;
	if (m_Config.UseGenre()) {
		m_Genres.GetWindowRect(genreRect);
	}
    m_Artists.GetWindowRect(artistRect);
    m_Albums.GetWindowRect(albumRect);
    m_Songs.GetWindowRect(songRect);
    m_Playlist.GetWindowRect(playlistRect);

    mWindowFlag = 0;

    if (m_Config.UseGenre() && genreRect.PtInRect(point)) {
        m_Genres.SetFocus();
        int sel = m_Genres.GetSelectedItemFromPoint(point);
        if (sel >= 0) {
            m_Genres.SetCurSel(sel);
            OnSelchangeGenres();
            mWindowFlag = 1;
        }
    } else if (artistRect.PtInRect(point)) {
        m_Artists.SetFocus();
        int sel = m_Artists.GetSelectedItemFromPoint(point);
        if (sel >= 0) {
            m_Artists.SetCurSel(sel);
            OnSelchangeArtists();
            mWindowFlag = 2;
        }
    } else if (albumRect.PtInRect(point)) {
        m_Albums.SetFocus();
        int sel = m_Albums.GetSelectedItemFromPoint(point);
        if (sel >= 0) {
            m_Albums.SetCurSel(sel);
            OnSelchangeAlbums();
            mWindowFlag = 3;
        }
    } else if (songRect.PtInRect(point)) {
        m_Songs.SetFocus();
        int sel = m_Songs.GetSelectedItemFromPoint(point);
        if (sel >= 0) m_Songs.SetCurSel(sel);
        mWindowFlag = 4;
    } else if (playlistRect.PtInRect(point)) {
        m_Playlist.SetFocus();
        int sel = m_Playlist.GetSelectedItemFromPoint(point);
        if (sel >= 0) m_Playlist.SetCurSel(sel);
        mWindowFlag = 5;
    }

    if (mWindowFlag) {
        CMenu menu;
        if (menu.LoadMenu(IDR_MENU1)) {
            CMenu * popup;
            popup = menu.GetSubMenu(1);
            ASSERT(popup != NULL);
            popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,ScreenPnt.x,
                ScreenPnt.y, AfxGetMainWnd());
        }
    }
}

void CPlayerDlg::OnUserEditSong() 
{
	if (m_Config.trialMode() == 2) {
		MBMessageBox("Notice", "Trial Expired.");
		return;
	}
	ModifyIDThree *dialog;
    CString genre,artist,album,title,year,track;
    CStringList genreList;
    m_mlib.getGenres(genreList);

	if (m_mlib._playlist.size() > 0) {
		MBMessageBox("Error", "You cannot edit/view song tags with a populated Playlist");
		return;
	}
	if (mWindowFlag > 4) {
		return;
	}

	Song song = new CSong;
    if (mWindowFlag == 4) {
	    int sel = m_Songs.GetCurSel();
		DWORD data = m_Songs.GetItemData(sel);
		song = m_mlib.m_SongLib.getSong(data);

//	    CString songname;
//	    m_Songs.GetText(sel, songname);
//		CString filename = m_mlib.getSongVal("FILE", _selectedGenre,
//			_selectedArtist, _selectedAlbum, songname);

		CString filename = song->getId3("FILE");
		if (!FileUtil::IsReadable(filename)) {
//			song = m_mlib.getSong(_selectedGenre, _selectedArtist,
//				_selectedAlbum, songname);
//		else {
			CString msg = "unable to open ";
			msg += filename;
			MBMessageBox(CString("Error"), msg);
			return;
		}
    } else {
        if (mWindowFlag >= 3) {
			song->setId3("TALB", _selectedAlbum);
	        int sel = m_Songs.GetCurSel();
	        CString songname;
	        m_Songs.GetText(sel, songname);
			Song tmp = m_mlib.getSong(_selectedGenre, _selectedArtist,
				_selectedAlbum, songname);
			song->setId3("TYER", tmp->getId3("TYER"));
        }
        if (mWindowFlag >= 2) {
			song->setId3(CS("TPE1"), _selectedArtist);
        }
//        if (_selectedGenre != MBALL) {
//            Genre_addGenre(id3, _selectedGenre);
			song->setId3(CS("TCON"), _selectedGenre);
//      }
    }

	dialog = new ModifyIDThree(&genreList, song, mWindowFlag);

	int ret;
	ret = dialog->DoModal();
	if (ret == IDOK) {
        m_mlib.modifyID3(song, dialog->m_newSong);
        initDb();
	}
	delete dialog;    	
}

#define MB_MIN_WIDTH 500
#define MB_MIN_HEIGHT 400

void CPlayerDlg::OnSizing(UINT fwSide, LPRECT pRect) 
{
	static int counter = 0;
	counter++;
//	CString msg("OnSizing ");
//	msg += numToString(counter) + "\r\n";
//	OutputDebugString(msg);

	int left,right,top,bottom;
	left = pRect->left;
	right = pRect->right;
	top = pRect->top;
	bottom = pRect->bottom;

	if ((fwSide == 7 || fwSide == 1 || fwSide == 4)
		&& (left > right - MB_MIN_WIDTH))
			left = right - MB_MIN_WIDTH;

	if ((fwSide == 4 || fwSide == 3 || fwSide == 5)
		&& (top > bottom - MB_MIN_HEIGHT))
			top = bottom - MB_MIN_HEIGHT;

	if ((fwSide == 5 || fwSide == 2 || fwSide == 8)
		&& (right < left + MB_MIN_WIDTH))
		right = left + MB_MIN_WIDTH;
	
	if ((fwSide == 7 || fwSide == 6 || fwSide == 8)
		&& (bottom < top + MB_MIN_HEIGHT))
		bottom = top + MB_MIN_HEIGHT;

	pRect->left = left;
	pRect->right = right;
	pRect->top = top;
	pRect->bottom = bottom;
	if (FALSE == m_Resizing) {
		CDialogClassImpl::OnSizing(fwSide, pRect);
	}
	m_Resizing = TRUE;	
}
void CPlayerDlg::OnCaptureChanged(CWnd *pWnd) {
	// This forces rubber band style resizing
	static int counter = 0;
	counter++;
	if (m_Resizing)
	{
		m_Resizing = FALSE;
		resetControls();
	}
	
	CDialog::OnCaptureChanged(pWnd);
}
void CPlayerDlg::OnSize(UINT nType, int cx, int cy) 
{
    static int first = 0;
    if (first < 2) {
        first++;
        return;
    }
	if (!m_Resizing) {
		CDialogClassImpl::OnSize(nType, cx, cy);
		resetControls();
	}
}
void
CPlayerDlg::CurrentTitleSet(LPCTSTR lpmsg) {
	CString msg(lpmsg);
	CurrentTitleSet(msg);
}
void
CPlayerDlg::CurrentTitleSet(CString & msg) {
	m_CurrentTitle.setText(msg);
}
void
CPlayerDlg::PlayerStatusSet(LPCTSTR lpmsg) {
	CString msg(lpmsg);
	PlayerStatusSet(msg);
}

void
CPlayerDlg::PlayerStatusSet(CString & msg) {
    m_PlayerStatus.setText(msg);
    m_PlayerStatusSave = msg;
}
void
CPlayerDlg::PlayerStatusTempSet(LPCTSTR lpmsg) {
	CString msg(lpmsg);
	PlayerStatusTempSet(msg);
}
void
CPlayerDlg::PlayerStatusTempSet(CString & msg) {
	PlayerStatusSet(msg);
    m_PlayerStatusTime = CTime::GetCurrentTime();
	StartStatusTimer();
}
void
CPlayerDlg::PlayerStatusRevert() {

    CTime t = CTime::GetCurrentTime();
    if (t.GetTime() > (m_PlayerStatusTime.GetTime() + PLAYER_STATUS_DELAY)) {
		m_PlayerStatus.setText(m_PlayerStatusSave);
		StopStatusTimer();
	}
	return;
	{
        CString current;
        m_PlayerStatus.GetWindowText(current);
        if (current == m_PlayerStatusSave) {
			m_PlayerStatus.setText(m_HelpMsg);
		} else if (current == m_HelpMsg) {
            m_PlayerStatus.setText(m_PlayerStatusSave);
        } else {
			m_PlayerStatus.setText(m_PlayerStatusSave);
		}
        m_PlayerStatusTime = CTime::GetCurrentTime();
    }
}
void
CPlayerDlg::movePlaylistUp(int sel) {
    m_mlib.movePlaylistUp(m_PlaylistCurrent, sel);
    if (m_PlaylistCurrent == sel) {
        m_PlaylistCurrent--;
    } else if (m_PlaylistCurrent == (sel -1)) {
        m_PlaylistCurrent++;
    }
}
void
CPlayerDlg::movePlaylistDown(int sel) {
    m_mlib.movePlaylistDown(m_PlaylistCurrent, sel);
    if (m_PlaylistCurrent == sel) {
        m_PlaylistCurrent++;
    } else if (m_PlaylistCurrent == (sel +1)) {
        m_PlaylistCurrent--;
    }
}


UINT
CPlayerDlg::OnNcHitTest( CPoint point ) {
    CRect r;//,applabelr; //,positionr,volumer;
	GetClientRect(&r);
	CRect br = r;
	br.TopLeft() = CPoint(br.BottomRight().x-20, br.BottomRight().y-20);
	
	ClientToScreen(&r);
	ClientToScreen(&br);

	UINT rv = CDialogClassImpl::OnNcHitTest(point);

    if(br.PtInRect(point)) {
		return HTBOTTOMRIGHT;
//	} else if (volumer.PtInRect(point)) {
//		m_VolumeSlider.SetCapture();
//		msg += "Vol\r\n";OutputDebugString(msg);
//		return HTCLIENT;
//	} else if (positionr.PtInRect(point)) {
//		m_PositionSlider.SetCapture();
//		msg += "Progress\r\n";OutputDebugString(msg);
//		return HTCLIENT;
//	} else if (applabelr.PtInRect(point)) {
//		msg += "AppLabel\r\n";OutputDebugString(msg);
//		return HTCAPTION;
    } else if (r.PtInRect(point)) {
//		msg += "Dlg\r\n";OutputDebugString(msg);
		return HTCAPTION;
    }

	return rv;
}

void CPlayerDlg::OnButtonMenu() 
{
    CMenu menu;
    if (menu.LoadMenu(IDR_MENU2)) {
        CMenu * popup = menu.GetSubMenu(0);
        ASSERT(popup != NULL);
        CWnd * ctrl;
        ctrl = GetDlgItem(IDC_OPTIONS_BUTTON);
        CRect button;
        ctrl->GetWindowRect(button);
        popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
			crbottomleft(button).x,
            crbottomleft(button).y, 
			AfxGetMainWnd());
    }
}

void CPlayerDlg::OnButtonMinimize() 
{
	ShowWindow(SW_MINIMIZE);
	
}

void CPlayerDlg::OnButtonMaximize() 
{
    WINDOWPLACEMENT lpwndpl;
    BOOL r = GetWindowPlacement(&lpwndpl);
    UINT newstate;
    if (lpwndpl.showCmd == SW_SHOWMAXIMIZED) {
        newstate = SW_SHOWNORMAL;
    } else if (lpwndpl.showCmd == SW_SHOWMINIMIZED
        || lpwndpl.showCmd == SW_SHOWNORMAL) {
        newstate = SW_SHOWMAXIMIZED;
    } else {
        newstate = SW_SHOWNORMAL;
    }

	ShowWindow(newstate);
//	resizeControls();
	
}

HBRUSH CPlayerDlg::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor ) {
	HBRUSH hbr = CDialogClassImpl::OnCtlColor(pDC, pWnd, nCtlColor);

    COLORREF bkcolor, txcolor;
    switch (nCtlColor) {
    case CTLCOLOR_BTN: //   3 Button control
		return hbr;

        bkcolor = m_Config.getColorBkPanel();
        txcolor = m_Config.getColorTxPanel();

        break;
    case CTLCOLOR_DLG: //   4 Dialog box
        bkcolor = m_Config.getColorBkPanel();
        txcolor = m_Config.getColorTxPanel();
        break;
    case CTLCOLOR_EDIT: //   1 Edit control
        bkcolor = m_Config.getColorBkPanel();
        txcolor = m_Config.getColorTxPanel();
        break;
    case CTLCOLOR_LISTBOX: //  2 List-box control
        bkcolor = m_Config.getColorBkNormal();
        txcolor = m_Config.getColorTxPanel();
        break;
    case CTLCOLOR_MSGBOX: //   0 Message box
        bkcolor = m_Config.getColorBkPanel();
        txcolor = m_Config.getColorTxPanel();
        break;
    case CTLCOLOR_SCROLLBAR: //  5 Scroll-bar control
        bkcolor = m_Config.getColorBkPanel();
        txcolor = m_Config.getColorTxPanel();
        break;
    case CTLCOLOR_STATIC: //  6 Static control 
        bkcolor = m_Config.getColorBkPanel();
        txcolor = m_Config.getColorTxPanel();
        break;
    default:
        bkcolor = m_Config.getColorBkPanel();
        txcolor = m_Config.getColorTxPanel();
        break;
    }

    pDC->SetBkColor(bkcolor);
    pDC->SetTextColor(txcolor);


    return (HBRUSH)m_brush;


}


void CPlayerDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {

    switch (nIDCtl)	{
//    case IDC_BUTTON_MINIMIZE:
//		break;
//    case IDC_BUTTON_MAXIMIZE:
//    case IDC_BUTTON_EXIT: 
    case IDC_OPTIONS_BUTTON: {
        CBrush br;
        CString sBtnText;
        CWnd * c = GetDlgItem(nIDCtl);

        c->GetWindowText(sBtnText);
        CDC * cdc = c->GetDC();

        br.CreateSolidBrush(config().getColorBkPanel());

        cdc->SetTextColor(config().getColorTxPanel());
        cdc->SetBkColor(m_Config.getColorBkPanel());
        cdc->FillRect(&lpDrawItemStruct->rcItem, &br);
        cdc->DrawText(sBtnText, &lpDrawItemStruct->rcItem, 
            DT_SINGLELINE | DT_VCENTER | DT_CENTER);
        br.DeleteObject();
        }
        break;
		//    case IDC_BUTTON_MINIMIZE:
		//    case IDC_BUTTON_MAXIMIZE:
		//    case IDC_BUTTON_EXIT: 
		//		{
		//        CBrush br;
		//        CString sBtnText;
		//        CWnd * c = GetDlgItem(nIDCtl);
		//        c->GetWindowText(sBtnText);
		//        CDC * cdc = c->GetDC();
		//        br.CreateSolidBrush(config().getColorBkPanel());
		//        //cdc->SetBkMode(lpDrawItemStruct->hDC, TRANSPARENT);
		//        cdc->SetTextColor(config().getColorTxPanel());
		//        cdc->SetBkColor(m_Config.getColorBkPanel());
		//        cdc->FillRect(&lpDrawItemStruct->rcItem, &br);
		//        cdc->DrawText(sBtnText, &lpDrawItemStruct->rcItem, 
		//            DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		//        br.DeleteObject();
		////        br.CreateSolidBrush(config().getColorTxPanel());
		////        cdc->FrameRect(&lpDrawItemStruct->rcItem, &br);
		//        }
        //break;
    default:
        CDialogClassImpl::OnDrawItem(nIDCtl, lpDrawItemStruct);
        break;	
    }
}

void CPlayerDlg::OnTestMenu() {
//    CScrollBar *s = new CScrollBar();
//    LPSCROLLINFO lpScrollInfo;
//    UINT nMask;
//    s->GetScrollInfo(  lpScrollInfo,  nMask );
//    TestScrollBar d;
//    d.DoModal();

}

BOOL CPlayerDlg::HelpInfo() {
    TCHAR szPath[_MAX_PATH],
      szFname[_MAX_FNAME],
      szDir[_MAX_DIR],
      szDrive[_MAX_DRIVE];

	if (0 == GetModuleFileName(NULL, szPath, sizeof(szPath))) {
		DWORD dwError = GetLastError();
		CString msg;
		msg << "HelpInfo: Error " << dwError;
		logger.log(msg);
		return FALSE;
	}

    _splitpath(szPath, szDrive, 
               szDir, szFname, NULL);
    _makepath(szPath, szDrive, szDir, 
                      MUZIKBROWZER, "chm");

//	HtmlHelp(GetDesktopWindow()->m_hWnd, 
//                 szPath,HH_DISPLAY_TOPIC, NULL);

//	HINSTANCE h = ShellExecute(this->m_hWnd, "open", "KeyHH.exe", szPath,
	HINSTANCE h = ShellExecute(this->m_hWnd, "open",  szPath, NULL,
		szDir, SW_SHOW);
	if ((int)h <= 32) {
		CString msg;
		msg << "ShellExecute on help failed. ShellExecute returned "
			<< (int)h << " on " << szPath << " " << szDir;
		logger.log(msg);
	}

    return(TRUE);
}
BOOL CPlayerDlg::OnHelpInfo(HELPINFO* pHelpInfo) {
	return HelpInfo();

}

void CPlayerDlg::OnMenuExportLibrary() {
//    m_mlib.exportLibrary();
    return;
}

void CPlayerDlg::StopTimers() {
	StopSeekTimer();
	StopStatusTimer();
}
void CPlayerDlg::StartStatusTimer()
{
	StopStatusTimer();
	m_StatusTimerId = SetTimer(MB_STATUS_TIMER_ID, 3000, NULL);
}
void CPlayerDlg::StopStatusTimer()
{
	if (m_StatusTimerId) {
		KillTimer(m_StatusTimerId);
		m_StatusTimerId = 0;
	}
}
void CPlayerDlg::StartSeekTimer() 
{
    // Cancel any pending timer event
    StopSeekTimer();
	m_PositionSlider.SetRange(0,100);	
    // Create a new timer
	m_timerid = SetTimer(MB_SEEK_TIMER_ID, 1000, NULL);
	m_Player->GetFileDuration();

}

void CPlayerDlg::StopSeekTimer() 
{
    // Cancel the timer
    if(m_timerid)        
    {                
		KillTimer(m_timerid);
        m_timerid = 0;
    }
}
void CPlayerDlg::resetPosition() {
	m_PositionLabel.setText("");
	m_PositionSlider.SetPos(0);
}
void CPlayerDlg::OnTimer(UINT nIDEvent) 
{
	switch(nIDEvent) {
	case MB_PLAYLOOP_TIMER_ID:
		PostMessage(WM_PLAYLOOP, (WPARAM)0, (LPARAM)0);
		break;
	case MB_SEEK_TIMER_ID:
		updatePositionLabel();
		break;
	case MB_STATUS_TIMER_ID:
		PlayerStatusRevert();
		break;
	}
}
void CPlayerDlg::updatePositionLabel()
{
	long pos_secs, dur_secs, pos_pct;
	pos_secs = dur_secs = pos_pct = 0;
	m_Player->ReadMediaPosition(pos_secs, dur_secs, pos_pct);
	// handing % to SetPos works cause the Pos slider has a range of
	// 0 - 100
	m_PositionSlider.SetPos(pos_pct);
	char buf[50];
	int mins = pos_secs / 60;
	int secs = pos_secs % 60;
	int dmins,dsecs;
	dmins = dur_secs / 60;
	dsecs = dur_secs % 60;
	sprintf(buf, "%d:%02d / %d:%02d", mins, secs, dmins, dsecs);
	m_PositionLabel.setText ( buf );
}
void CPlayerDlg::adjustPosition() {
	m_Player->Seek(m_PositionSlider.GetPos(), TRUE);
	updatePositionLabel();
}
void CPlayerDlg::adjustPosition(int pct) {
	m_Player->Seek(pct, TRUE);
	updatePositionLabel();
}
LRESULT
CPlayerDlg::OnProgress(UINT wParam, LONG lParam) {
	adjustPosition();
	return 0;
}
void CPlayerDlg::OnFastForward() {
	adjustPosition(m_PositionSlider.GetPos() + 5);
}
void CPlayerDlg::OnReverse() {
	adjustPosition(m_PositionSlider.GetPos() - 5);
}

//void CPlayerDlg::updateVolumeLabel() {
//	AutoBuf buf(10);
//	sprintf(buf.p,"%d", m_VolumeSlider.GetPos());
//	m_VolumeLabel.setText(buf.p);
//}
LRESULT
CPlayerDlg::OnVolume(UINT wParam, LONG lParam) {
	adjustVolume();
	return 0;
}
void CPlayerDlg::adjustVolume() {
	if (m_Player->SetVolume(m_VolumeSlider.GetPos())) {
//		updateVolumeLabel();
	} else {
		logger.log("Unable to set volume");
		PlayerStatusTempSet("Unable to set volume");
	}
}
void CPlayerDlg::adjustVolume(int level) {
	m_VolumeSlider.SetPos(level);
	adjustVolume();
}
void CPlayerDlg::OnVolUp() {
	adjustVolume(m_VolumeSlider.GetPos() + 1);
}
void CPlayerDlg::OnVolDown() {
	adjustVolume(m_VolumeSlider.GetPos() - 1);
}

void CPlayerDlg::InputClose() {
	m_Player->InputClose();
	StopSeekTimer();
}
void CPlayerDlg::Pause() {
	m_Player->Pause();
	StopSeekTimer();
}

void CPlayerDlg::killAlbumArt() {
	m_Picture.unload();
}
void CPlayerDlg::displayAlbumArt(const CString & file) {

	static BOOL first = TRUE;
	// first time up show the dflt skin art

	killAlbumArt();
	size_t size = 0;
	uchar * data = NULL;
	BOOL pic = m_mlib.apic(file, data, size);

	if (!first && pic) {
		m_Picture.load(data,size);
	} else {
		first = FALSE;
		if (m_AlbumArt != "") {
			m_Picture.load(m_AlbumArt);
		} else {
			m_Picture.blank();
		}
	}
	if (data)
		delete [] data;

//	RedrawWindow();

}

afx_msg void
CPlayerDlg::OnNcPaint() {
	CDC* pdc= GetWindowDC();
	EraseBkgndNC(pdc);//, BitmapToCRect * bmcr)
	ReleaseDC(pdc);	
	return;
}

TEST(WmaTag2, read2)
{
	CString file = "c:\\mkm\\src\\muzik\\browzer\\testdata\\x.wma";
	WmaTag wma(file);
	CString info = wma.getInfo();
	CString x;
}

void CPlayerDlg::OnButtonResize() 
{
	resetControls();	
}
