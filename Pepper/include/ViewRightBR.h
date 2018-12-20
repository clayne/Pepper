#pragma once
#include "PepperDoc.h"
#include "ChildFrm.h"
#include "ListEx.h"
#include <memory>

class CViewRightBR : public CScrollView
{
	DECLARE_DYNCREATE(CViewRightBR)
protected:
	CViewRightBR() {}
	virtual ~CViewRightBR() {}
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	int ShowResource(std::vector<std::byte>* pData, UINT uResType);
	DECLARE_MESSAGE_MAP()
private:
	CWnd* m_pActiveWnd { };
	libpe_ptr m_pLibpe { };
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	LISTEXINFO m_stListInfo;
	CListEx m_stListTLSCallbacks;
	LOGFONT m_lf { }, m_hdrlf { };
	CImageList m_stImgRes;
	bool m_fDrawRes { false };
	COLORREF m_clrBk { RGB(230, 230, 230) };
	COLORREF m_clrBkImgList { RGB(250, 250, 250) };
	//HWND for RT_DIALOG.
	HWND m_hwndResTemplatedDlg { };
	BITMAP m_stBmp { };
	int m_iResTypeToDraw { };
	//Width and height of whole image to draw.
	int m_iImgResWidth { }, m_iImgResHeight { };
	//Vector for RT_GROUP_ICON/CURSOR.
	std::vector<std::unique_ptr<CImageList>> m_vecImgRes { };
	std::wstring m_strResStrings;
	std::wstring m_strResVerInfo;
	CEdit m_stEditResStrings; //Edit control for RT_STRING, RT_VERSION
	CFont m_fontEditRes; //Font for m_stEditResStrings.
	std::map<int, std::wstring> m_mapVerInfoStrings {
		{ 0, L"FileDescription" },
	{ 1, L"FileVersion" },
	{ 2, L"InternalName" },
	{ 3, L"CompanyName" },
	{ 4, L"LegalCopyright" },
	{ 5, L"OriginalFilename" },
	{ 6, L"ProductName" },
	{ 7, L"ProductVersion" }
	};
	bool m_fJustOneTime { true }; //To set splitter's size once correctly.
private:
	int CreateListTLSCallbacks();
};

/****************************************************************
* Struct for RT_GROUP_ICON/CURSOR.								*
****************************************************************/
#pragma pack( push, 2 )
struct GRPICONDIRENTRY
{
	BYTE   bWidth;               // Width, in pixels, of the image
	BYTE   bHeight;              // Height, in pixels, of the image
	BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
	BYTE   bReserved;            // Reserved
	WORD   wPlanes;              // Color Planes
	WORD   wBitCount;            // Bits per pixel
	DWORD  dwBytesInRes;         // how many bytes in this resource?
	WORD   nID;                  // the ID
};
struct GRPICONDIR
{
	WORD			  idReserved;   // Reserved (must be 0)
	WORD			  idType;	    // Resource type (1 for icons)
	WORD			  idCount;	    // How many images?
	GRPICONDIRENTRY   idEntries[1]; // The entries for each image
};
using LPGRPICONDIR = const GRPICONDIR*;
#pragma pack( pop )

struct LANGANDCODEPAGE {
	WORD wLanguage;
	WORD wCodePage;
};
