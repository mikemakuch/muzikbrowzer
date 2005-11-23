#ifndef _MISC_H_
#define _MISC_H_

#include "stdafx.h"
#include "ControlSK.h"

CString MBFormatError(LONG e);

CPoint crtopright(CRect & rect);
CPoint crtopleft(CRect & rect);
CPoint crbottomright(CRect & rect);
CPoint crbottomleft(CRect & rect);

class CObjectInt : public CObject
{
	public:
		CObjectInt(int v = 0) { m_int = v; }
		int m_int;
};
class BitmapToCRect
{
public:
	BitmapToCRect(HBITMAP bm, CRect & rect, LayOutStyle los, 
		DWORD width, DWORD height, CString desc, HDC hdc=NULL)
		
		: m_hBitmap(bm),
		m_rect(rect), m_loStyle(los), m_width(width), m_height(height),
		m_desc(desc),m_dc(NULL), m_hMask(NULL)
	{
		if (hdc) {
			m_dc = new CDC();
			m_dc->Attach(hdc);
		}

	};
	~BitmapToCRect() {
		if (m_hBitmap)
			::DeleteObject(m_hBitmap);
		if (m_hMask)
			::DeleteObject(m_hMask);
		if (m_dc) {
			m_dc->Detach();
			delete m_dc;
		}
	};

	HBITMAP m_hBitmap;
	HBITMAP m_hMask;
	CRect   m_rect;
	LayOutStyle m_loStyle;
	DWORD m_width;
	DWORD m_height;
	CString m_desc;
	CDC * m_dc;

};
class MBUtil {
public:
	static BOOL RgbTriple(const char * val, unsigned long & rgb);
	static BOOL ConfigFontValidate(const char * font);
	static CString CrToRGB(COLORREF rgb);
	static void BmpToDC(CDC* pDC, BitmapToCRect * bmcr, 
		BOOL doTrans, COLORREF bkcolor,int offset);
	
	static void CutAndTileBmp(CDC* pDC, BitmapToCRect * bmcr, 
		int offset,UINT BITBLTTYPE );

//	static void CutAndTileBmp(CDC* pDC, HDC hdcSrc, HBITMAP bmp,
//		CRect & destrect, LayOutStyle los, int srcwidth, int srcheight,
//		int offset,UINT BITBLTTYPE );
	
	static HBITMAP CreateBitmapMask(HBITMAP hSourceBitmap,
				DWORD dwidth, DWORD dheight,
				DWORD swidth, DWORD sheight,
				LayOutStyle los,COLORREF bkcolor);
};

#endif
