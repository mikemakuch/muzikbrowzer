// ConfigFiles.cpp : implementation file
//

#include "stdafx.h"
#include "FolderDialog.h"
#include "Registry.h"
#include "MyLog.h"
#include "MBMessageBox.h"
#include "ConfigFiles.h"
#include "MyString.h"
#include "FileAndFolder.h"
//#define _WIN32_DCOM
#include "GetTextField.h"
#include <objbase.h>
#include <winbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CConfigFiles property page

IMPLEMENT_DYNCREATE(CConfigFiles, CPropertyPage)

CConfigFiles::CConfigFiles(CWnd *p, PlayerCallbacks * pcb) : CPropertyPage(CConfigFiles::IDD),
    /*m_PlayerDlg(p),*/ m_RunAtStartupUL(0), m_scanNew(FALSE),
	m_AlbumSortAlpha(TRUE), m_AlbumSortDate(FALSE),
	m_playercallbacks(pcb), m_bAdd(FALSE), m_ResetNeeded(FALSE),
	m_OrigRunAtStartup(FALSE)

{
	//{{AFX_DATA_INIT(CConfigFiles)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    setDefaults();
    init();
}
void CConfigFiles::setDefaults() {

}

CConfigFiles::~CConfigFiles()
{
}

void CConfigFiles::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigFiles)
	DDX_Control(pDX, IDC_RUNATSTARTUP,	m_RunAtStartup);
	DDX_Control(pDX, IDC_MDB_LOCATION,	m_MdbLocation);
	DDX_Control(pDX, IDC_DIRLIST,		m_MP3DirList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigFiles, CPropertyPage)
	//{{AFX_MSG_MAP(CConfigFiles)
	ON_BN_CLICKED(IDC_DIRADD, OnDiradd)
	ON_LBN_SELCHANGE(IDC_DIRLIST, OnSelchangeDirlist)
	ON_BN_CLICKED(IDC_DIRREMOVE, OnDirremove)
	ON_BN_CLICKED(IDC_DIRSCAN, OnDirscan)
	ON_BN_CLICKED(IDC_LOCATION_BUTTON, OnLocationButton)
	ON_BN_CLICKED(IDC_DIRSCAN_NEW, OnDirscanNew)
	ON_BN_CLICKED(IDC_ALBUMSORT_DATE, OnAlbumsortDate)
	ON_BN_CLICKED(IDC_ALBUMSORT_ALPHA, OnAlbumsortAlpha)
	ON_BN_CLICKED(IDC_RUNATSTARTUP, OnRunatstartup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigFiles message handlers
void CConfigFiles::init() {

    ReadReg();
}






void CConfigFiles::OnSelchangeDirlist() 
{
    CWnd * button = GetDlgItem(IDC_DIRREMOVE);
    button->EnableWindow(TRUE);
	SetModified(TRUE);
}

void CConfigFiles::OnDirremove() 
{
    int cursel = m_MP3DirList.GetCurSel();
    m_MP3DirList.DeleteString(cursel);
	m_MP3DirList.SetCurSel(
		cursel < m_MP3DirList.GetCount() ? cursel : cursel -1);

    EnableDisableButtons(); 
	SetModified(TRUE);

	m_slMP3DirList.RemoveAll();
	CString tmp;
	for(cursel = 0; cursel < m_MP3DirList.GetCount(); cursel++) {
		m_MP3DirList.GetText(cursel,tmp);
		m_slMP3DirList.AddTail(tmp);
	}

}

class ThreadParams {
public:

	CString (*scanDirectories)(const CStringList & directories,
						  InitDlg * initDlg, BOOL scanNew, BOOL bAdd);
    CStringList * m_dirs;
    InitDlg * m_InitDialog;
    CString m_result;
	BOOL m_scanNew;
	BOOL m_bAdd;
};

static int ScanThread_done = 0;

static UINT
ScanThread(LPVOID pParam) {
    ThreadParams *tp = (ThreadParams *)pParam;

	ScanThread_done = 0;
	//::Sleep(2000);
	tp->m_result = (*tp->scanDirectories)(*(tp->m_dirs), 
		tp->m_InitDialog, tp->m_scanNew, tp->m_bAdd);
    tp->m_InitDialog->SendUpdateStatus(4, "", 0,0); // EndDialog;

    ScanThread_done = 1;
    return 0;
}

void
CConfigFiles::ScanThreadStart(ThreadParams &tp) {

    AfxBeginThread(ScanThread, (LPVOID) &tp);

    return;
}

void
CConfigFiles::ScanThreadStop() {
}

void CConfigFiles::OnDirscan() 
{

	Scan(FALSE);
}
void CConfigFiles::OnDirscanNew() 
{
	Scan(TRUE);
}
void CConfigFiles::Scan(BOOL fornew) {
	m_bAdd=FALSE;
	CString title = "Confirmation";
	CString text;
	if (fornew) {
		text = "Search the music folders below for new music and\r\nadd to Library?\r\n\r\n";
	} else {
		text = "Search the music folders below for all music and\r\nrebuild Library from scratch?\r\n\r\n";
	}

	POSITION pos;
	CString dir;
	for (pos = m_slMP3DirList.GetHeadPosition(); pos != NULL; ) {
        dir = m_slMP3DirList.GetAt(pos);
		text += dir + "\r\n";
		m_slMP3DirList.GetNext(pos);
    }

	int r = MBMessageBox("Confirmation", text, FALSE, TRUE);
	if (r == 0) {
		MBMessageBox("Notice","Search not performed");
		return ;
	}

	CStringList x;
	if (fornew) {
		m_scanNew = TRUE;
		dirScan(x);	
	} else {
		m_scanNew = FALSE;
		dirScan(x);	
	}
}


void CConfigFiles::dirScan(CStringList & mp3list) {

    CStringList dirs;
	CStringList * pcsl;
    CString dir;
    int n = m_slMP3DirList.GetCount();
	if (n < 1) {
		MBMessageBox("Search", "You must enter at least one directory in Settings.",FALSE);
		return;
	}
	if (m_bAdd) {
		pcsl = &mp3list;
	} else {
		pcsl = &m_slMP3DirList;
	}

	POSITION pos;
	for (pos = pcsl->GetHeadPosition(); pos != NULL; ) {
        dir = pcsl->GetAt(pos);
        dirs.AddTail(dir);
		pcsl->GetNext(pos);
    }
    ThreadParams tp;
	// Create worker thread
	CWinThread * pThread = AfxBeginThread(ScanThread, &tp,
		THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);

	InitDlg *m_InitDialog = new InitDlg(0,1, pThread);

    //tp.m_mlib = &m_PlayerDlg->m_mlib;
	tp.scanDirectories = m_playercallbacks->scanDirectories;
    tp.m_dirs = &dirs;
    tp.m_InitDialog = m_InitDialog;
	tp.m_scanNew = m_scanNew;
	tp.m_bAdd = m_bAdd;

	m_InitDialog->DoModal(); // main thread handles dlg

    CString scaninfo = tp.m_result;

	(*m_playercallbacks->initDb)();

    CString libcounts = (*m_playercallbacks->getLibraryCounts)();

	(*m_playercallbacks->statusset)(libcounts);

	(*m_playercallbacks->UpdateWindow)();

    delete m_InitDialog;

    MBMessageBox("Search Results", scaninfo, FALSE, FALSE);

}
void CConfigFiles::OnDiradd() 
{

    // open a file
	int s = m_MP3DirList.GetCurSel();
	CString dflt;
	if (s <  0)
		s = 0;
	if (s = m_MP3DirList.GetCount()) {
		m_MP3DirList.GetText(s-1, dflt);
		dflt = String::upDir(dflt);
	}

	CFileAndFolder dialog(this, dflt);
	dialog.SetShowFiles(FALSE);
	dialog.setTitle("Add folders");
	dialog.setMsg("Select folder(s) to be searched.");

	int ret;
	ret = dialog.DoModal();
	if (ret == IDOK) {
        // a file was selected
		CStringList list;
		dialog.GetPaths(list);
		POSITION pos;
		for(pos = list.GetHeadPosition(); pos != NULL;) {
			CString path = list.GetNext(pos);
			if (path.GetLength())
				m_MP3DirList.AddString(path);
				m_slMP3DirList.AddTail(path);
		}

        EnableDisableButtons(); 
        UpdateData(FALSE);
		SetModified(TRUE);
	}
}
void CConfigFiles::OnLocationButton() 
{
    // open a file
	CString dflt;
	m_MdbLocation.GetWindowText(dflt);
	CFileAndFolder dialog(this, dflt);
	dialog.SetShowFiles(FALSE);
	dialog.setTitle("Set location of Muzikbrowzer files");
	dialog.setMsg("... where Muzikbrowzer's files, playlists, skins reside");

	int ret;
	ret = dialog.DoModal();
	if (ret == IDOK) {
        // a file was selected
		CStringList paths ;
		dialog.GetPaths(paths );
        POSITION pos;
		for(pos = paths.GetHeadPosition(); pos != NULL;) {
			CString path = paths.GetNext(pos);
			if (path.GetLength()) {
				m_MdbLocation.SetWindowText(path);
				EnableDisableButtons(); 
				UpdateData(FALSE);

				(*m_playercallbacks->setDbLocation)(path);
				m_path = path;
				SetModified(TRUE);
				m_LocDirModified = TRUE;
//				return;
			}
		}
		ReadFolders();
		UpdateData(FALSE);
	}	
}


void CConfigFiles::ReadReg() {
    RegistryKey reg( HKEY_LOCAL_MACHINE, RegKey );

    CString Location;
    AutoBuf buf(1000);
    reg.Read(RegDbLocation, buf.p, 999, "");
    Location = buf.p;

    if (Location.GetLength()) {
        m_origMdbLocation = Location;
		m_path = Location;
    } else {
        TCHAR szBuff[_MAX_PATH];
        ::GetModuleFileName(NULL, szBuff, _MAX_PATH);
        int n = strlen(szBuff);
        int i;
        for (i = n-1 ; i > 0 ; --i) {
            if (szBuff[i] == '\\') {
                szBuff[i] = 0;
                i = 0;
            }
        }
        m_origMdbLocation = szBuff;
        reg.Write(RegDbLocation, szBuff);

		(*m_playercallbacks->setDbLocation)(m_origMdbLocation);
        m_path = m_origMdbLocation;
    }

	ReadFolders();

    m_RunAtStartupUL = reg.Read(RegRunAtStartup, 0);
	unsigned long x = reg.Read(RegAlbumSort, 0);
	if (x == 0) {
		m_AlbumSortAlpha = 0;
		m_AlbumSortDate = 1;

	} else {
		m_AlbumSortAlpha = 1;
		m_AlbumSortDate = 0;
	}



}
void CConfigFiles::ReadFolders() {
	m_origMP3DirList.RemoveAll();
	m_slMP3DirList.RemoveAll();
	if (::IsWindow(m_MP3DirList.m_hWnd))
		m_MP3DirList.ResetContent();
	RegistryKey dreg(m_path + "\\Muzikbrowzer.folders");
	dreg.ReadFile();
	unsigned long numdirs/*,numExtensions*/;
    numdirs = dreg.Read(RegNumDirs, 0);
    AutoBuf buf(1000);
    unsigned long i;
    for (i = 0 ; i < numdirs ; ++i) {
        sprintf(buf.p, "%s%02d", RegDirKey, i);
        CString dir;
        //dreg.Read(buf.p, dir, 999, "");
		dir = dreg.ReadCString(buf.p,"");
		if (dir.GetLength()) {
			m_origMP3DirList.AddTail(dir);
			m_slMP3DirList.AddTail(dir);
			if (::IsWindow(m_MP3DirList.m_hWnd))
				m_MP3DirList.AddString(dir);
		}
    }

}
void CConfigFiles::WriteFolders() {
    AutoBuf buf(1000);
    unsigned long num = m_slMP3DirList.GetCount();
//    RegistryKey reg( HKEY_LOCAL_MACHINE, RegKey );
	RegistryKey reg(m_path + "\\Muzikbrowzer.folders");
    reg.Write(RegNumDirs, num);

    m_origMP3DirList.RemoveAll();

    unsigned long i=0;
	POSITION pos;
	CString dir;
	for (pos = m_slMP3DirList.GetHeadPosition(); pos != NULL; ) {
        dir = m_slMP3DirList.GetAt(pos);
		m_origMP3DirList.AddTail(dir);
		m_slMP3DirList.GetNext(pos);
		sprintf(buf.p, "%s%02d", RegDirKey, i);
		reg.Write(buf.p, dir);
		i++;
    }
	reg.WriteFile();
}

void CConfigFiles::StoreReg() {
    AutoBuf buf(1000);


    CString Location;
    m_MdbLocation.GetWindowText(Location);
    m_origMdbLocation = Location;

    const TCHAR * location = (LPCTSTR) Location;

	WriteFolders();
    RegistryKey reg( HKEY_LOCAL_MACHINE, RegKey );
    reg.Write(RegDbLocation, location);
	m_origMdbLocation = location;

    if (m_RunAtStartup.GetCheck() == 0) {
        m_RunAtStartupUL = 0;
        reg.Write(RegRunAtStartup, (unsigned long) 0);
    } else {
        m_RunAtStartupUL = 1;
        reg.Write(RegRunAtStartup, (unsigned long) 1);
    }
	m_OrigRunAtStartup = (BOOL)m_RunAtStartupUL;

    setRunAtStartup();
	reg.Write(RegAlbumSort, (unsigned long)m_AlbumSortAlpha);
	m_InitialAlbumSortAlpha = m_AlbumSortAlpha;


}
void
CConfigFiles::setRunAtStartup() {
    // use this one CSIDL_STARTUP

	IMalloc *m;
	SHGetMalloc(&m);
	if (!m) return;

    LPITEMIDLIST itemList;
    char path[MAX_PATH];
    if (SHGetSpecialFolderLocation(NULL, CSIDL_STARTUP, &itemList) !=
        NOERROR) {
        MBMessageBox("", "Cannot get special folder list");
        return;
    }
    if (!SHGetPathFromIDList(itemList,path)) {
		m->Free(itemList);
        MBMessageBox("","Cannot get special folder path");
        return;
    }
    CString shortCutFile = path;
    shortCutFile += "\\";
    shortCutFile += MUZIKBROWZER;
    shortCutFile += ".lnk";

    if (m_RunAtStartupUL) {
        char shortCutTarget[MAX_PATH+1];
        DWORD r = GetModuleFileName(NULL, shortCutTarget, MAX_PATH);
        if (r == 0) {
			m->Free(itemList);
            MBMessageBox("","GetModuleFileName failed!");
            return;
        }
        CString workingDir = shortCutTarget;
        CString mbdotexe = MUZIKBROWZER;
        mbdotexe += ".exe";
        workingDir = workingDir.Left(workingDir.GetLength()
            - (mbdotexe.GetLength() + 1));


        HRESULT hres;
        IShellLink *psl;

        // Create an IShellLink object and get a pointer to the IShellLink 
        // interface (returned from CoCreateInstance).
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

        hres = CoCreateInstance (CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
        IID_IShellLink, (void **)&psl);
        if (SUCCEEDED (hres)) {
            IPersistFile *ppf;

            // Query IShellLink for the IPersistFile interface for 
            // saving the shortcut in persistent storage.
            hres = psl->QueryInterface (IID_IPersistFile, (void **)&ppf);
            if (SUCCEEDED (hres)) { 
                WORD wsz [MAX_PATH]; // buffer for Unicode string

                // Set the path to the shortcut target.
                hres = psl->SetPath (shortCutTarget);
                if (! SUCCEEDED (hres))
                    MBMessageBox ("","SetPath failed!");

                // Set the description of the shortcut.
                hres = psl->SetDescription (MUZIKBROWZER);
                if (! SUCCEEDED (hres))
                    MBMessageBox ("","SetDescription failed!");

                hres = psl->SetWorkingDirectory(workingDir);
                if (! SUCCEEDED (hres))
                    MBMessageBox ("","SetWorkingDirectory failed!");

                // Ensure that the string consists of ANSI characters.
                MultiByteToWideChar (CP_ACP, 0, shortCutFile, -1, wsz, MAX_PATH);

                // Save the shortcut via the IPersistFile::Save member function.
                hres = ppf->Save (wsz, TRUE);
                if (! SUCCEEDED (hres))
                    MBMessageBox ("","Save failed!");

                // Release the pointer to IPersistFile.
                ppf->Release ();
            }
            // Release the pointer to IShellLink.
            psl->Release ();
            CoUninitialize();
        }
    } else {

//where
//pszShortcutFile is the file that the shortcut will point to. 
//pszLink is the shortcut you are creating with a LNK extension. 
//pszDesc is the description of the file. This is the string "Shortcut to
//filename", where filename is the name of the shortcut target. 
//You have to call CoInitialize before calling this.
//Check out the Article I mentioned earlier for more details. Its
//available on MSDN. 

        if (!DeleteFile (shortCutFile)) {
            CString msg = "Cannot delete shortcut";
            msg += "\r\n";
            msg += shortCutFile;
            //MBMessageBox ("",msg);
        }
    }
	m->Free(itemList);
}


BOOL CConfigFiles::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	POSITION pos;
    m_MP3DirList.ResetContent();
    for (pos = m_origMP3DirList.GetHeadPosition(); pos != NULL; ) {
        CString extension = m_origMP3DirList.GetAt(pos);
        m_MP3DirList.AddString(extension);
	    m_origMP3DirList.GetNext(pos);
    }

    m_MdbLocation.SetWindowText(m_origMdbLocation);


    if (m_RunAtStartupUL) {
        m_RunAtStartup.SetCheck(1);
		m_OrigRunAtStartup = TRUE;
    } else {
        m_RunAtStartup.SetCheck(0);
		m_OrigRunAtStartup = FALSE;
    }
	


	if (m_AlbumSortAlpha) {
		CheckRadioButton(IDC_ALBUMSORT_DATE,IDC_ALBUMSORT_ALPHA,
			IDC_ALBUMSORT_ALPHA);
		m_InitialAlbumSortAlpha = TRUE;
	} else {
		CheckRadioButton(IDC_ALBUMSORT_DATE,IDC_ALBUMSORT_ALPHA,
			IDC_ALBUMSORT_DATE);
		m_InitialAlbumSortAlpha = FALSE;
	}

    UpdateWindow();

    UpdateData(FALSE);

    EnableDisableButtons(); 
	m_LocDirModified = FALSE;
	m_bAdd = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigFiles::EnableDisableButtons() {
    CString location;
    m_MdbLocation.GetWindowText(location);
    CWnd * scan = GetDlgItem(IDC_DIRSCAN);
	CWnd * scannew = GetDlgItem(IDC_DIRSCAN_NEW);
    if (m_MP3DirList.GetCount() < 1 || location.GetLength() < 1) {
        scan->EnableWindow(FALSE);
		scannew->EnableWindow(FALSE);
    } else {
        scan->EnableWindow(TRUE);
		scannew->EnableWindow(TRUE);
    }

}

void CConfigFiles::OnAlbumsortDate() 
{
	m_AlbumSortAlpha = FALSE;
	m_AlbumSortDate = TRUE;
	CheckRadioButton(IDC_ALBUMSORT_DATE,IDC_ALBUMSORT_ALPHA,IDC_ALBUMSORT_DATE);
	UpdateData(FALSE);
	SetModified(TRUE);
}

void CConfigFiles::OnAlbumsortAlpha() 
{
	m_AlbumSortAlpha = TRUE;
	m_AlbumSortDate = FALSE;
	CheckRadioButton(IDC_ALBUMSORT_DATE,IDC_ALBUMSORT_ALPHA,IDC_ALBUMSORT_ALPHA);
	UpdateData(FALSE);
	SetModified(TRUE);
	
}
BOOL CConfigFiles::AlbumSortAlpha()
{
	return m_AlbumSortAlpha;
}


BOOL CConfigFiles::OnApply() 
{
	StoreReg();
	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CConfigFiles::OnOK() 
{
	CPropertyPage::OnOK();
    StoreReg();
	SetModified(FALSE);
	if (m_LocDirModified) {
		(*m_playercallbacks->initDb)();
	}
	if (m_AlbumSortAlpha != m_InitialAlbumSortAlpha) {
		m_ResetNeeded = TRUE;
		//MBMessageBox("Notice","Changes to the \"Album sort\" parameter will take effect\r\nthe next time you start Muzikbrowzer. Alternatively hit \"F5\".",FALSE);
	}
}
void CConfigFiles::OnCancel() 
{
//	m_MP3DirList.ResetContent();
//	m_slMP3DirList.RemoveAll();

//	POSITION pos;
//	for (pos = m_origMP3DirList.GetHeadPosition(); pos != NULL; ) {
//		CString dir = m_origMP3DirList.GetAt(pos);
//		m_MP3DirList.AddString(dir);
//		m_slMP3DirList.AddTail(dir);
//		m_origMP3DirList.GetNext(pos);
//	}
	m_MdbLocation.SetWindowText(m_origMdbLocation);
	m_path = m_origMdbLocation;
	ReadFolders();

	if (m_InitialAlbumSortAlpha) {
		CheckRadioButton(IDC_ALBUMSORT_DATE,IDC_ALBUMSORT_ALPHA,
			IDC_ALBUMSORT_ALPHA);
		m_AlbumSortDate = FALSE;
		m_AlbumSortAlpha = TRUE;
	} else {
		CheckRadioButton(IDC_ALBUMSORT_DATE,IDC_ALBUMSORT_ALPHA,
			IDC_ALBUMSORT_DATE);
		m_AlbumSortDate = TRUE;
		m_AlbumSortAlpha = FALSE;
	}

    if (m_OrigRunAtStartup) {
        m_RunAtStartup.SetCheck(1);
    } else {
        m_RunAtStartup.SetCheck(0);
    }
	StoreReg();
//    UpdateData(FALSE);
	CPropertyPage::OnCancel();
}


void CConfigFiles::AddMusic() {
	CString dflt;

	if (!m_slMP3DirList.IsEmpty()) {
		dflt = m_slMP3DirList.GetTail();
		dflt = String::upDir(dflt);
	}

	CFileAndFolder dialog(this, dflt);

	dialog.setTitle("Add music to Muzikbrowzer library");
	dialog.setMsg("Select file(s)/folder(s) to be added.");
	int ret;
	ret = dialog.DoModal();
	if (ret == IDOK) {
		CStringList list;
		dialog.GetPaths(list);
		POSITION pos;
		for(pos = list.GetHeadPosition(); pos != NULL;) {
			CString path = list.GetNext(pos);
			if (path.GetLength())
				m_slMP3DirList.AddTail(path);
		}
		WriteFolders();
		m_bAdd = TRUE;
		dirScan(list);
		m_bAdd = FALSE;
	}
}




void CConfigFiles::OnRunatstartup() 
{
	UpdateData(FALSE);
	SetModified(TRUE);
	
}
