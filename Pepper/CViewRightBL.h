/****************************************************************************************************
* Copyright © 2018-2024 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#pragma once
#include "CChildFrm.h"
#include "CFileLoader.h"
#include "CPepperDoc.h"
#include "CTreeEx.h"
#include "HexCtrl.h"

import Utility;

using namespace HEXCTRL;
using namespace LISTEX;
using namespace Utility;

class CViewRightBL : public CView {
private:
	[[nodiscard]] auto GetListByID(UINT_PTR uListID) -> CListEx*;
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	void OnInitialUpdate()override;
	BOOL OnCommand(WPARAM wParam, LPARAM lParam)override;
	void OnDraw(CDC* pDC)override; // overridden to draw this view
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void OnUpdate(CView*, LPARAM, CObject*)override;
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)override;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTreeSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeRClick(NMHDR* pNMHDR, LRESULT* pResult);
	void CreateListExportFuncs();
	void CreateListImportFuncs();
	void CreateTreeResources();
	void CreateListRelocsEntry();
	void CreateListDelayImpFuncs();
	void ShowDosHdrHexEntry(DWORD dwEntry);
	void ShowRichHdrHexEntry(DWORD dwEntry);
	void ShowNtHdrHexEntry(); //There is only one entry in this header, no dwEntry needed.
	void ShowFileHdrHexEntry(DWORD dwEntry);
	void ShowOptHdrHexEntry(DWORD dwEntry);
	void ShowDataDirsHexEntry(DWORD dwEntry);
	void ShowSecHdrHexEntry(DWORD dwEntry);
	void ShowLCDHexEntry(DWORD dwEntry);
	void ShowImportListEntry(DWORD dwEntry);
	void ShowDelayImpListEntry(DWORD dwEntry);
	void ShowRelocsListEntry(DWORD dwEntry);
	void ShowDebugHexEntry(DWORD dwEntry);
	void ShowTLSHex();
	void ShowSecurityHexEntry(unsigned nSertId);
	BOOL PreCreateWindow(CREATESTRUCT& cs)override;
	void OnDocEditMode();
	void OnMDITabActivate(bool fActivate);
	DECLARE_MESSAGE_MAP();
	DECLARE_DYNCREATE(CViewRightBL);
private:
	PEFILEINFO m_stFileInfo;
	CChildFrame* m_pChildFrame { };
	CPepperDoc* m_pMainDoc { };
	CFileLoader* m_pFileLoader { };
	HWND m_hwndActive { };
	IHexCtrlPtr m_stHexEdit { CreateHexCtrl() };
	HEXCREATE m_hcs { };
	LISTEXCREATE m_stlcs;
	CListEx m_listExportFuncs;
	CListEx m_listImportEntry;
	CListEx m_listDelayImportEntry;
	CListEx m_listRelocsEntry;
	CTreeEx m_treeResBottom;
	CImageList m_imglTreeRes;
	std::vector<std::tuple<long, long, long, EResType>> m_vecResId { }; //Lvl: ROOT, 2, 3 and res type for tree node.
	LOGFONTW m_lf { };
	LOGFONTW m_hdrlf { };
	PERESFLAT m_stResData;
	EResType m_eResType;
	std::vector<HWND> m_vecHWNDVisible;
};