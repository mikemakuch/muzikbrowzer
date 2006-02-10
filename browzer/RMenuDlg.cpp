// RMenuDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RMenuDlg.h"
#include "PlayerDlg.h"
#include "MBGlobals.h"
#include "VirtualControl.h"
#include "SkinDefs.h"
#include "MBMessageBox.h"
#include "GetTextField.h"
#include "MyLog.h"
#include "Misc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// RMenuDlg dialog


RMenuDlg::RMenuDlg(CWnd* pParent, PlayerCallbacks * pcb)
				
	: CDialogSK(RMenuDlg::IDD, pParent), 
	m_parentcallbacks(pcb),
    m_maxbutton(9),
    m_currentbutton(0), m_Control(new VirtualControl),
	m_reg(pcb->mbconfig()->getSkin(MB_SKIN_DEF))
{
	//{{AFX_DATA_INIT(RMenuDlg)
	//}}AFX_DATA_INIT
	m_bPlayer = FALSE;



	MBCONFIG_READ_SKIN_DEFS_POBJ(m_parentcallbacks->mbconfig(),m_reg);
}
RMenuDlg::~RMenuDlg() {
	delete m_Control;

}


void RMenuDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogSK::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(RMenuDlg)
	DDX_Control(pDX, IDC_PMENUSTOP, m_Stop);
	DDX_Control(pDX, IDC_PMENUSHUFFLEPLAYLIST, m_Shuffle);
	DDX_Control(pDX, IDC_PMENUSAVEPLAYLIST, m_Save);
	DDX_Control(pDX, IDC_PMENURANDOMIZEPLAYLIST, m_Random);
	DDX_Control(pDX, IDC_PMENUPLAY, m_Play);
	DDX_Control(pDX, IDC_PMENUPAUSE, m_Pause);
	DDX_Control(pDX, IDC_PMENULOADPLAYLIST, m_Load);
	DDX_Control(pDX, IDC_PMENUHELP, m_Help);
	DDX_Control(pDX, IDC_PMENUEXIT, m_Exit);
	DDX_Control(pDX, IDC_PMENUCLEARPLAYLIST, m_Clear);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(RMenuDlg, CDialogSK)
	//{{AFX_MSG_MAP(RMenuDlg)
    ON_BN_CLICKED(IDC_PMENUCLEARPLAYLIST, OnMenuClearplaylist)
	ON_BN_CLICKED(IDC_PMENUEXIT, OnMenuExit)
    ON_BN_CLICKED(IDC_PMENULOADPLAYLIST, OnMenuLoadplaylist)
	ON_BN_CLICKED(IDC_PMENUPAUSE, OnMenuPause)
	ON_BN_CLICKED(IDC_PMENUPLAY, OnMenuPlay)
    ON_BN_CLICKED(IDC_PMENUSHUFFLEPLAYLIST, OnMenuShuffleplaylist)
	ON_BN_CLICKED(IDC_PMENUSTOP, OnMenuStop)
	ON_BN_CLICKED(IDC_PMENUHELP, OnMenuHelp)
    ON_BN_CLICKED(IDC_PMENURANDOMIZEPLAYLIST, OnMenuRandomizePlaylist)
	ON_BN_CLICKED(IDC_PMENUSAVEPLAYLIST, OnMenuSavePlaylist)
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_NCHITTEST()
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// RMenuDlg message handlers
BOOL RMenuDlg::OnInitDialog() 
{

	m_Need2Erase = TRUE;
	CDialogSK::OnInitDialog();

	ModifyStyle(WS_CAPTION,0,0); // delete the title bar

	MBCONFIG_READ_TRANS_COLORS(m_reg,m_crTransMain,m_crTransPanel);
	SetTransparentColor(m_crTransMain,m_crTransPanel); 

	COLORREF crColHdrInUL,crColHdrInLR,crColHdrOutUL,crColHdrOutLR;
	COLORREF crDataInUL,crDataInLR,crDataOutUL,crDataOutLR;
	COLORREF crStatusInUL,crStatusInLR,crStatusOutUL,crStatusOutLR;

	MBCONFIG_READ_COLOR_3D(m_reg,MB3DCOLHDRCOLOR,
		crColHdrInUL,crColHdrInLR,crColHdrOutUL,crColHdrOutLR);
	
	MBCONFIG_READ_COLOR_3D(m_reg,MB3DDATACOLOR,
		crDataInUL,crDataInLR,crDataOutUL,crDataOutLR);
	
	MBCONFIG_READ_COLOR_3D(m_reg,MB3DSTATUSCOLOR,
		crStatusInUL,crStatusInLR,crStatusOutUL,crStatusOutLR);
	
	COLORREF crOtherFg,crOtherBg,
		crButtonsFgOut,  crButtonsBgOut,
		crButtonsFgHover,crButtonsBgHover,
		crButtonsFgIn,crButtonsBgIn;
	BOOL OtherFlat,OtherBorder;

	MBCONFIG_READ_OTHER_COLORS(m_reg,
		crOtherFg,crOtherBg,
		crButtonsFgOut,  crButtonsBgOut,
		crButtonsFgHover,crButtonsBgHover,
		crButtonsFgIn,crButtonsBgIn,
		OtherFlat,OtherBorder);

	BOOL threeDDataWindows	= m_reg.Read(MB3DDATA,0);
	BOOL threeDColHdrs		= m_reg.Read(MB3DCOLHDRS,0);
	BOOL threeDStatus		= m_reg.Read(MB3DSTATUS,0);

	COLORREF crColHdrFg,crColHdrBg;
	crColHdrFg = m_parentcallbacks->mbconfig()->getColorTxColHdr();
	crColHdrBg = m_parentcallbacks->mbconfig()->getColorBkColHdr();

#define _LABEL_COLORS_ crColHdrFg,crColHdrBg,\
	crColHdrInUL,crColHdrInLR,crColHdrOutUL,crColHdrOutLR,\
	threeDColHdrs

	CRect wrect;
	GetWindowRect(wrect);

	int border = m_parentcallbacks->mbconfig()->getDlgBorderWidth();
	int borderpanel = m_reg.Read("BorderPanel",5);
	int borderhorz = m_parentcallbacks->mbconfig()->getDlgBorderHorz();
	int bordervert = m_parentcallbacks->mbconfig()->getDlgBorderVert();

	CRect exitrect;
	m_Exit.GetWindowRect(exitrect);
	int edge = (border * 2) + (borderpanel * 2);
	int cwidth = exitrect.Width() + edge;
	int cheight = (exitrect.Height() * 10) + (bordervert * 10) + edge;

	wrect.right = wrect.left + cwidth;
	wrect.bottom = wrect.top + cheight;
	
	MoveWindow(wrect);
	ScreenToClient(wrect);
	CRect clientrect(wrect);
	
	clientrect.DeflateRect(border,border,border,border);
	
	LayOutStyle BackgroundMainType,BackgroundPanelType;
	MBCONFIG_READ_BACKGROUND_TYPES(m_reg,BackgroundMainType,BackgroundPanelType);

//	wrect.OffsetRect(3,3);
#pragma hack		
	// hack alert: rect0 needs 2 b Window instead of client
	// for the resizing border
	FreeResources();
	SetBitmap(m_parentcallbacks->mbconfig()->getSkin(MB_SKIN_BACKGROUNDMAIN), 
		wrect, BackgroundMainType, CS("frame"));
	SetBitmap(m_parentcallbacks->mbconfig()->getSkin(MB_SKIN_BACKGROUNDPLAYLIST),
		clientrect, BackgroundPanelType, CS("play panel"));
	CDC * cdc = GetDC();
	make(cdc);
	ReleaseDC(cdc);
	OnNcPaint() ;
	EraseBkgndNC();//(cdc); // force CDialogSK to paint background
	// This sends a WM_NCPAINT to repaint the resize frame
	RedrawWindow(NULL,NULL, RDW_FRAME|RDW_INVALIDATE);

	CButtonST *button[10];
	button[0] = &m_Exit;
	button[1] = &m_Shuffle;
	button[2] = &m_Random;
	button[3] = &m_Clear;
	button[4] = &m_Save;
	button[5] = &m_Load;
	button[6] = &m_Pause;
	button[7] = &m_Play;
	button[8] = &m_Stop;
	button[9] = &m_Help;
	int i;
	int x = border + borderpanel;
	x -= 3;
	int y = x;
	for(i = 0 ; i < 10 ; i++) {
		button[i]->SetFlat(FALSE);
		button[i]->DrawBorder(TRUE);
		button[i]->SetColors(crButtonsFgHover,crButtonsBgHover,
			crButtonsFgOut,crButtonsBgOut,
			crButtonsFgIn,crButtonsBgIn);
		button[i]->MoveWindow(x,y,exitrect.Width(),exitrect.Height());
		y += bordervert + exitrect.Height();
	}
	
	CRect rect,prect;
	m_pParentWnd->GetWindowRect(prect);
	GetWindowRect(rect);
	x = (prect.right - prect.left) / 2;
	x -= (rect.Width() / 2);
	y = (prect.bottom - prect.top) / 2;
	y -= (rect.Height() / 2);
	x += prect.left;
	y += prect.top;
	MoveWindow(x,y,rect.Width(),rect.Height());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void RMenuDlg::OnMenuExit() 
{
//	((CPlayerDlg*)m_pParentWnd)->OnMenuExit();
	EndDialog(0);
}
void RMenuDlg::OnMenuShuffleplaylist() 
{
	((CPlayerDlg*)m_pParentWnd)->OnMenuShuffleplaylist();
	EndDialog(0);
}
void RMenuDlg::OnMenuRandomizePlaylist() 
{
	// TODO: Add your control notification handler code here
	
}
void RMenuDlg::OnMenuClearplaylist() 
{
    ((CPlayerDlg*)m_pParentWnd)->OnMenuClearplaylist();
    EndDialog(0);
}
void RMenuDlg::OnMenuSavePlaylist() 
{
    ((CPlayerDlg*)m_pParentWnd)->SetSavePlaylistFlag(FALSE);
	((CPlayerDlg*)m_pParentWnd)->OnMenuSaveplaylist();
	EndDialog(0);	
}
void RMenuDlg::OnMenuLoadplaylist() 
{
	((CPlayerDlg*)m_pParentWnd)->OnMenuLoadplaylist();
	EndDialog(0);
}
void RMenuDlg::OnMenuPause() 
{
	((CPlayerDlg*)m_pParentWnd)->OnMenuPause();
	EndDialog(0);
}
void RMenuDlg::OnMenuPlay() 
{
	((CPlayerDlg*)m_pParentWnd)->OnMenuPlay();
	EndDialog(0);
}
void RMenuDlg::OnMenuStop() 
{
	((CPlayerDlg*)m_pParentWnd)->OnMenuStop();
	EndDialog(0);
}
void RMenuDlg::OnMenuHelp() 
{
	((CPlayerDlg*)m_pParentWnd)->OnMenuHelp();
	EndDialog(0);
}
void RMenuDlg::OnControlClick() {
	EndDialog(m_currentbutton);
}
void RMenuDlg::OnExit() {
	EndDialog(0);
}


