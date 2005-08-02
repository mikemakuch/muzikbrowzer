// DialogSK.cpp : implementation file
//

#include "stdafx.h"
#include "CDialogSK.h"
#include "MyLog.h"
#include "MyString.h"
#include "FileUtils.h"


class BitmapToCRect
{
public:
	BitmapToCRect(HBITMAP bm, CRect & rect, LayOutStyle los, 
		DWORD width, DWORD height, CString & desc)
		
		: m_hBitmap(bm),
		m_rect(rect), m_loStyle(los), m_width(width), m_height(height),
		m_desc(desc)
	{};
	~BitmapToCRect() {
		::DeleteObject(m_hBitmap);
		::DeleteObject(m_hMask);
	};

	HBITMAP m_hBitmap;
	HBITMAP m_hMask;
	CRect   m_rect;
	LayOutStyle m_loStyle;
	DWORD m_width;
	DWORD m_height;
	CString m_desc;

};
static lpfnSetLayeredWindowAttributes g_pSetLayeredWindowAttributes;
/////////////////////////////////////////////////////////////////////////////
// CDialogSK dialog


//  these variables should have been defined in some standard header but is not
#define WS_EX_LAYERED 0x00080000 
#define LWA_COLORKEY 1 // Use color as the transparency color.
#define LWA_ALPHA    2 // Use bAlpha to determine the opacity of the layer


//  ===========================================================================
//  Function pointer for lyering API in User32.dll
//  ===========================================================================

CDialogSK::CDialogSK(CWnd* pParent /*=NULL*/) :m_NumBitmapToCRect(0)
{
    //{{AFX_DATA_INIT(CBkDialogST)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    Init();
}

CDialogSK::CDialogSK(UINT uResourceID, CWnd* pParent)
: CDialog(uResourceID, pParent), m_NumBitmapToCRect(0)
{
    Init();
}


CDialogSK::CDialogSK(LPCTSTR pszResourceID, CWnd* pParent)
: CDialog(pszResourceID, pParent), m_NumBitmapToCRect(0)
{
    Init();
}

CDialogSK::~CDialogSK()
{
    FreeResources();
}


BOOL 
CDialogSK::OnInitDialog()
{
    CDialog::OnInitDialog();
    
	return TRUE;
}

BOOL
CDialogSK::SetTransparent (BYTE bAlpha)
{
    if (g_pSetLayeredWindowAttributes == NULL)
        return FALSE;

    if (bAlpha < 255)
    {
        //  set layered style for the dialog
        SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        
        //  call it with 255 as alpha - opacity
        g_pSetLayeredWindowAttributes(m_hWnd, 0, bAlpha, LWA_ALPHA);
    }
    else
    {
        SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);

        // Ask the window and its children to repaint
        ::RedrawWindow(m_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
    }

    return TRUE;
}


BOOL
CDialogSK::SetTransparentColor (COLORREF main, COLORREF panel, BOOL bTrans)
{
	m_Panel = panel;

    if (g_pSetLayeredWindowAttributes == NULL)
        return FALSE;

    if (bTrans)
    {
        //  set layered style for the dialog
        SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        //  call it with 0 alpha for the given color
        g_pSetLayeredWindowAttributes(m_hWnd, main, 0, LWA_COLORKEY);
    }
    else
    {
        SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);

        // Ask the window and its children to repaint
        ::RedrawWindow(m_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
    }

    return TRUE;
}

void CDialogSK::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDialogSK)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogSK, CDialog)
//{{AFX_MSG_MAP(CDialogSK)
	ON_WM_LBUTTONDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogSK message handlers

void CDialogSK::OnLButtonDown(UINT nFlags, CPoint point) 
{
    if (m_bEasyMove)
        PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));	

    CDialog::OnLButtonDown(nFlags, point);
}

void CDialogSK::Init()
{
	m_NumBitmapToCRect = 0;
	memset(m_BitmapToCRect, 0, sizeof(m_BitmapToCRect));
    m_bEasyMove = TRUE;
	m_Need2Erase = TRUE;
	m_bmBackground = NULL;
	m_bmFrame = NULL;

	static BOOL first = TRUE;
	if (first) {
		//  get the function from the user32.dll 
		HMODULE hUser32 = GetModuleHandle(_T("USER32.DLL"));
		g_pSetLayeredWindowAttributes = (lpfnSetLayeredWindowAttributes)
                        GetProcAddress(hUser32, "SetLayeredWindowAttributes");
		first = FALSE;
	}
}


void CDialogSK::FreeResources()
{
	int i;
	for (i = 0 ; i < m_NumBitmapToCRect; i++) {
		if (m_BitmapToCRect[i]) {
			delete m_BitmapToCRect[i];
			m_BitmapToCRect[i] = NULL;
		}
	}
	if (m_bmBackground)
		delete m_bmBackground;
	if (m_bmFrame)
		delete m_bmFrame;
	Init();

}

DWORD CDialogSK::SetBitmap(int nBitmap, CRect & rect, LayOutStyle los
						   , CString & desc)
{
    HBITMAP    hBitmap       = NULL;
    HINSTANCE  hInstResource = NULL;
    
    // Find correct resource handle
    hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(nBitmap), RT_BITMAP);
    
    // Load bitmap In
    hBitmap = (HBITMAP)::LoadImage(hInstResource, MAKEINTRESOURCE(nBitmap),
                                   IMAGE_BITMAP, 0, 0, 0);
    
    return SetBitmap(hBitmap, rect, los, desc);
}

DWORD CDialogSK::SetBitmap(LPCTSTR lpszFileName, CRect & rect, LayOutStyle los
						   , CString & desc)
{
    HBITMAP    hBitmap       = NULL;
    hBitmap = (HBITMAP)::LoadImage(0, lpszFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    return SetBitmap(hBitmap, rect, los, desc);
}

DWORD CDialogSK::SetBitmap(HBITMAP hBitmap, CRect & rect, LayOutStyle los
						   , CString & desc)
{
//	CString msg("CDialogSk w,h=");
//	msg += numToString(rect.Width()) + "," + numToString(rect.Height()) + "\r\n";
//	OutputDebugString(msg);
    int nRetValue;
    BITMAP  csBitmapSize;
    
    if (hBitmap)
    {
        // Get bitmap size
        nRetValue = ::GetObject(hBitmap, sizeof(csBitmapSize), &csBitmapSize);
        if (nRetValue == 0)
        {
            FreeResources();
            return 0;
        }
        DWORD width = (DWORD)csBitmapSize.bmWidth;
        DWORD height = (DWORD)csBitmapSize.bmHeight;

		if (m_BitmapToCRect[m_NumBitmapToCRect]) {
			delete m_BitmapToCRect[m_NumBitmapToCRect];
		}

		m_BitmapToCRect[m_NumBitmapToCRect++] 
			= new BitmapToCRect(hBitmap, rect, los, width, height, desc);
    }
    
    if (IsWindow(this->GetSafeHwnd()))
        Invalidate();
    
    return 1;
    
}

HBITMAP CDialogSK::CreateBitmapMask(HBITMAP hSourceBitmap, 
				DWORD dwidth, DWORD dheight,
				DWORD swidth, DWORD sheight,
				LayOutStyle los)
{
	HBITMAP		hMask		= NULL;
	HDC			hdcSrc		= NULL;
	HDC			hdcDest		= NULL;
	HDC			hdcTmp		= NULL;
	HBITMAP		hbmSrcT		= NULL;
	HBITMAP		hbmDestT	= NULL;
	HBITMAP		hbmNewSrc	= NULL;
	HBITMAP		hbmTmp		= NULL;
	COLORREF	crSaveBk;
	COLORREF	crSaveDestText;

	HDC hdc = ::CreateCompatibleDC(NULL);
	hMask = ::CreateCompatibleBitmap(hdc, dwidth,dheight);
	if (hMask == NULL)	return NULL;

	hdcSrc	= ::CreateCompatibleDC(NULL);
	hdcDest	= ::CreateCompatibleDC(NULL);
	
	hbmSrcT = (HBITMAP)::SelectObject(hdcSrc, hSourceBitmap);
	hbmDestT = (HBITMAP)::SelectObject(hdcDest, hMask);

	crSaveBk = ::SetBkColor(hdcSrc, m_Panel);

//	if (los == LO_STRETCH) {
//		::StretchBlt(hdcDest, 0, 0, dwidth, dheight,
//			hdcSrc, 0, 0, swidth, sheight, SRCCOPY);
//	} else {
		::BitBlt(hdcDest, 0, 0, dwidth, dheight,
			hdcSrc, 0, 0, SRCCOPY);
//	}

	crSaveDestText = ::SetTextColor(hdcSrc, RGB(255, 255, 255));
	::SetBkColor(hdcSrc,RGB(0, 0, 0));

//	if (los == LO_STRETCH) {
//		::StretchBlt(hdcSrc, 0, 0, dwidth, dheight,
//			hdcDest, 0, 0, swidth, sheight, SRCAND);
//	} else {
		::BitBlt(hdcSrc, 0, 0, dwidth,dheight,
			hdcDest, 0, 0, SRCAND);
//	}

	::SetTextColor(hdcDest, crSaveDestText);

	::SetBkColor(hdcSrc, crSaveBk);
	::SelectObject(hdcSrc, hbmSrcT);
	::SelectObject(hdcDest, hbmDestT);

	::DeleteDC(hdcSrc);
	::DeleteDC(hdcDest);
	::DeleteDC(hdc);

	return hMask;
} // End of CreateBitmapMask

void CDialogSK::make(CDC * wDC) {
//	CString msg = "make bg\r\n";
//	OutputDebugString(msg);
	int i;
	BitmapToCRect * bmcr;

	CDC dc;
	dc.CreateCompatibleDC(NULL);
	CRect wrect;
	CBitmap * oldbm ;
	CRect crect;
	GetWindowRect(wrect);
	ScreenToClient(wrect);
	GetClientRect(crect);
	int frameborder = (wrect.Width() - crect.Width()) / 2;

	m_bmFrame = new CBitmap();
	m_bmFrame->CreateCompatibleBitmap(wDC, wrect.Width(), wrect.Height());
	oldbm = dc.SelectObject((CBitmap*)m_bmFrame);

	for (i = 0 ; i < m_NumBitmapToCRect; i++) {
		bmcr = m_BitmapToCRect[i];
		applyOneBg(&dc, bmcr, (i>0)*frameborder);
	}		
	dc.SelectObject(oldbm);

	CDC * pDC = GetDC();
	dc.DeleteDC();
	dc.CreateCompatibleDC(NULL);
    

	m_bmBackground = new CBitmap();
	m_bmBackground->CreateCompatibleBitmap(pDC, crect.Width(), crect.Height());

	oldbm = (CBitmap*)dc.SelectObject((CBitmap*)m_bmFrame);

	CDC dc2;
	dc2.CreateCompatibleDC(NULL);
	CBitmap * oldbm2 = (CBitmap*) dc2.SelectObject(m_bmBackground);
	dc2.BitBlt(0,0,crect.Width(),crect.Height(),
		&dc,frameborder,frameborder, SRCCOPY);
	dc2.SelectObject(oldbm2);
	dc.SelectObject(oldbm);

//	FileUtil::BmpSave(*m_bmBackground, "c:\\mkm\\bmps\\bgBg.bmp");


	
	ReleaseDC(pDC);
	dc.DeleteDC();

	//FileUtil::BmpSave(*m_bmFrame, "c:\\mkm\\bmps\\bgFrame.bmp");


}
// OnEraseBkgnd 4/26/05
// Big note to self: Putting a logger.log or AutoLog in this
// function caused weirdo problemos where the background gets
// erased in the wrong places!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// I spent a couple days trying to nail this down and this
// is all I could find.
BOOL CDialogSK::OnEraseBkgnd(CDC* pDC) 
{
	if (!m_Need2Erase) {
		//m_Need2Erase = TRUE;
//		OutputDebugString("CDialogSK::NOT OnEraseBkgnd\r\n");
		return TRUE;
	}
	CRect rect;
	GetClientRect(rect);
	CDC dc;
	dc.CreateCompatibleDC(pDC);
	CBitmap * old = dc.SelectObject(m_bmBackground);
	pDC->BitBlt(0,0,rect.Width(),rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(old);
//	OutputDebugString("CDialogSK::OnEraseBkgnd\r\n");
	return TRUE;
}
BOOL CDialogSK::EraseBkgndNC(CDC *pDC) {

	CRect rect;
	GetWindowRect(rect);
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	CBitmap * old = dc.SelectObject(m_bmFrame);
	pDC->BitBlt(0,0,rect.Width(),rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(old);
//	OutputDebugString("CDialogSK::OnEraseBkgndNC\r\n");
	return TRUE;
}
BOOL CDialogSK::applyOneBg(CDC* pDC, BitmapToCRect * bmcr, int offset)
{
    if (!bmcr)
        return FALSE;
	
    CDC dc;
    dc.CreateCompatibleDC(NULL);

    HBITMAP    pbmpOldBmp = NULL;
	HBITMAP hMask = NULL;
	HDC hdcTmp = (HDC)::CreateCompatibleDC(NULL);

    pbmpOldBmp = (HBITMAP)::SelectObject(dc.m_hDC, bmcr->m_hBitmap);

	int x,y,swidth,sheight,dwidth,dheight;
	x = bmcr->m_rect.left+offset;
	y =  bmcr->m_rect.top+offset;
	dwidth = bmcr->m_rect.Width();
	dheight = bmcr->m_rect.Height();
	swidth = bmcr->m_width;
	sheight = bmcr->m_height;
	
    if ( bmcr->m_loStyle == LO_DEFAULT || bmcr->m_loStyle == LO_RESIZE )
    {
		if (offset) { // do the transparent stuff
			::SelectObject(dc.m_hDC, pbmpOldBmp);

			hMask = CreateBitmapMask(bmcr->m_hBitmap, swidth, sheight,
				swidth, sheight, bmcr->m_loStyle);
//			FileUtil::BmpLog(hMask, "SK-Mask");

			pbmpOldBmp = (HBITMAP)::SelectObject(dc.m_hDC, bmcr->m_hBitmap);

			// from DrawTheBitmap
			// And the mask
			HBITMAP hOldMask = (HBITMAP)::SelectObject(hdcTmp, hMask);
			::StretchBlt(pDC->m_hDC,x,y,dwidth,dheight,hdcTmp,0,0,
				swidth,sheight,SRCAND);
			::SelectObject(hdcTmp, hOldMask);
				::DeleteObject(hMask);
			pDC->BitBlt(bmcr->m_rect.left+offset, bmcr->m_rect.top+offset, 
				bmcr->m_rect.Width(), bmcr->m_rect.Height(), &dc, 0, 0, SRCPAINT);
		} else {
			pDC->BitBlt(bmcr->m_rect.left+offset, bmcr->m_rect.top+offset, 
				bmcr->m_rect.Width(), bmcr->m_rect.Height(), &dc, 0, 0, SRCCOPY);
		}
//		FileUtil::BmpLog(pDC->m_hDC, "SK-Bmp",dwidth,dheight,x,y);
    }
    else if (bmcr->m_loStyle == LO_TILE)
    {
        int ixOrg, iyOrg;

        for (iyOrg = 0; iyOrg < bmcr->m_rect.Height(); iyOrg += bmcr->m_height)
        {
            for (ixOrg = 0; ixOrg < bmcr->m_rect.Width(); ixOrg += bmcr->m_width)
            {
                pDC->BitBlt (ixOrg+offset, iyOrg+offset, bmcr->m_rect.Width(), 
					bmcr->m_rect.Height(), &dc, 0, 0, SRCCOPY);
            }
        }
    }
    else if (bmcr->m_loStyle == LO_CENTER)
    {
        int ixOrg = (bmcr->m_rect.Width() - bmcr->m_width) / 2;
        int iyOrg = (bmcr->m_rect.Height() - bmcr->m_height) / 2;
        pDC->BitBlt(ixOrg+offset, iyOrg+offset, bmcr->m_rect.Width(), bmcr->m_rect.Height(), &dc, 0, 0, SRCCOPY);
    }
    else if ( bmcr->m_loStyle == LO_STRETCH)
    {
		if (offset) { // do the transparent stuff
			::SelectObject(dc.m_hDC, pbmpOldBmp);

			hMask = CreateBitmapMask(bmcr->m_hBitmap, swidth, sheight,
				swidth, sheight, bmcr->m_loStyle);

			pbmpOldBmp = (HBITMAP)::SelectObject(dc.m_hDC, bmcr->m_hBitmap);

			// from DrawTheBitmap
			// And the mask
			HBITMAP hOldMask = (HBITMAP)::SelectObject(hdcTmp, hMask);
			::StretchBlt(pDC->m_hDC,x,y,dwidth,dheight,hdcTmp,0,0,
				swidth,sheight,SRCAND);
			::SelectObject(hdcTmp, hOldMask);
			::DeleteObject(hMask);
			pDC->StretchBlt(x, y, dwidth, dheight, 
				&dc, 0, 0, swidth, sheight, SRCPAINT);
		} else {
	        pDC->StretchBlt(x, y, dwidth, dheight, 
				&dc, 0, 0, swidth, sheight, SRCCOPY);
		}

    }
    
	::SelectObject(dc.m_hDC, pbmpOldBmp);
	::DeleteDC(hdcTmp);
	
	
    return TRUE;
}

void
CDialogSK::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
//	if (m_hBitmap != NULL)
//	{
		Invalidate();
//	}	
}

void
CDialogSK::EnableEasyMove(BOOL pEnable)
{
    m_bEasyMove = pEnable;
}

//void
//CDialogSK::SetStyle(LayOutStyle style)
//{
//    m_loStyle = style;
//    if(m_loStyle == LO_RESIZE /* && m_hBitmap */)
//    {
//        SetWindowPos(0, 0, 0, m_dwWidth, m_dwHeight, SWP_NOMOVE | SWP_NOREPOSITION );
//    }
//}
