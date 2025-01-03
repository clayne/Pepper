/****************************************************************************************************
* Copyright © 2018-2024 Jovibor https://github.com/jovibor/                                         *
* This software is available under the Apache-2.0 License.                                          *
* Official git repository: https://github.com/jovibor/Pepper/                                       *
* Pepper is a PE32 (x86) and PE32+ (x64) binares viewer/editor.                                     *
****************************************************************************************************/
#include "stdafx.h"
#include "CViewRightBL.h"
#include "res/resource.h"
#include "strsafe.h"
#include <format>

import Utility;

IMPLEMENT_DYNCREATE(CViewRightBL, CView)

BEGIN_MESSAGE_MAP(CViewRightBL, CView)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_RESOURCE_BOTTOM, &CViewRightBL::OnTreeSelChanged)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_RESOURCE_BOTTOM, &CViewRightBL::OnTreeRClick)
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
END_MESSAGE_MAP()

BOOL CViewRightBL::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_CLIPCHILDREN;

	return CView::PreCreateWindow(cs);
}

void CViewRightBL::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_pChildFrame = static_cast<CChildFrame*>(GetParentFrame());
	m_pMainDoc = static_cast<CPepperDoc*>(GetDocument());
	m_pFileLoader = &m_pMainDoc->GetFileLoader();
	m_stFileInfo = m_pMainDoc->GetFileInfo();

	//Hex control for SecurityDir and TLSdir.
	m_hcs.hWndParent = m_hWnd;
	m_hcs.uID = IDC_HEX_RIGHT_BL;
	m_hcs.dwStyle = WS_CHILD;
	m_stHexEdit->Create(m_hcs);

	m_stlcs.hWndParent = m_hWnd;
	m_stlcs.pColors = &Utility::g_stListColors;
	m_stlcs.dwHdrHeight = 35;
	m_stlcs.fSortable = true;

	const auto pDC = GetDC();
	const auto iLOGPIXELSY = GetDeviceCaps(pDC->m_hDC, LOGPIXELSY);
	m_lf.lfHeight = m_hdrlf.lfHeight = -MulDiv(11, iLOGPIXELSY, 72);
	ReleaseDC(pDC);

	StringCchCopyW(m_lf.lfFaceName, 9, L"Consolas");
	m_stlcs.pLFList = &m_lf;
	m_hdrlf.lfWeight = FW_BOLD;
	StringCchCopyW(m_hdrlf.lfFaceName, 16, L"Times New Roman");
	m_stlcs.pLFHdr = &m_hdrlf;

	CreateListExportFuncs();
	CreateListImportFuncs();
	CreateTreeResources();
	CreateListRelocsEntry();
	CreateListDelayImpFuncs();
}

void CViewRightBL::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* /*pHint*/)
{
	const auto iMsg = LOWORD(lHint);
	if (iMsg == MSG_MDITAB_ACTIVATE || iMsg == MSG_MDITAB_DISACTIVATE) {
		OnMDITabActivate(iMsg == MSG_MDITAB_ACTIVATE);
		return; //No further handling if it's tab Activate/Disactivate messages.
	}

	if (!m_pChildFrame || iMsg == IDC_HEX_RIGHT_TR)
		return;

	CRect rc;
	GetClientRect(&rc);

	switch (iMsg) {
	case IDC_LIST_DOSHEADER_ENTRY:
		ShowDosHdrHexEntry(HIWORD(lHint));
		break;
	case IDC_LIST_RICHHEADER_ENTRY:
		ShowRichHdrHexEntry(HIWORD(lHint));
		break;
	case IDC_LIST_NTHEADER_ENTRY:
		ShowNtHdrHexEntry();
		break;
	case IDC_LIST_FILEHEADER_ENTRY:
		ShowFileHdrHexEntry(HIWORD(lHint));
		break;
	case IDC_LIST_OPTIONALHEADER_ENTRY:
		ShowOptHdrHexEntry(HIWORD(lHint));
		break;
	case IDC_LIST_DATADIRECTORIES_ENTRY:
		ShowDataDirsHexEntry(HIWORD(lHint));
		break;
	case IDC_LIST_SECHEADERS_ENTRY:
		ShowSecHdrHexEntry(HIWORD(lHint));
		break;
	case IDC_LIST_EXPORT:
		if (m_hwndActive != m_listExportFuncs.GetHWND()) {
			if (m_hwndActive)
				::ShowWindow(m_hwndActive, SW_HIDE);

			m_hwndActive = m_listExportFuncs.GetHWND();
		}
		m_listExportFuncs.SetWindowPos(m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		break;
	case IDC_LIST_IMPORT_ENTRY:
		ShowImportListEntry(HIWORD(lHint));
		break;
	case IDC_TREE_RESOURCE:
		if (m_hwndActive != m_treeResBottom.m_hWnd) {
			if (m_hwndActive)
				::ShowWindow(m_hwndActive, SW_HIDE);

			m_hwndActive = m_treeResBottom.m_hWnd;
		}
		m_treeResBottom.SetWindowPos(this, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		break;
	case IDC_LIST_SECURITY_ENTRY:
		ShowSecurityHexEntry(HIWORD(lHint));
		break;
	case IDC_LIST_LOADCONFIG_ENTRY:
		ShowLCDHexEntry(HIWORD(lHint));
		break;
	case IDC_LIST_RELOCATIONS_ENTRY:
		ShowRelocsListEntry(HIWORD(lHint));
		break;
	case IDC_LIST_TLS:
		ShowTLSHex();
		break;
	case IDC_LIST_DELAYIMPORT_ENTRY:
		ShowDelayImpListEntry(HIWORD(lHint));
		break;
	case IDC_LIST_DEBUG_ENTRY:
		ShowDebugHexEntry(HIWORD(lHint));
		break;
	case ID_DOC_EDITMODE:
		OnDocEditMode();
		break;
	default:
		if (m_hwndActive) {
			::ShowWindow(m_hwndActive, SW_HIDE);
		}
		break;
	}
}

void CViewRightBL::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (m_hwndActive)
		::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CViewRightBL::OnDraw(CDC* /*pDC*/)
{
}

BOOL CViewRightBL::OnEraseBkgnd(CDC* pDC)
{
	return CView::OnEraseBkgnd(pDC);
}

void CViewRightBL::OnTreeSelChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	const auto& pstResRoot = m_pMainDoc->GetResources();
	if (!pstResRoot)
		return;

	const auto pTree = reinterpret_cast<LPNMTREEVIEWW>(pNMHDR);
	const auto [idlvlRoot, idlvl2, idlvl3, eResType] = m_vecResId.at(m_treeResBottom.GetItemData(pTree->itemNew.hItem));
	if (idlvl2 >= 0 && idlvl3 >= 0) {
		const auto& rootvec = pstResRoot->vecResData;
		const auto& lvl2tup = rootvec[idlvlRoot].stResLvL2;
		const auto& lvl2vec = lvl2tup.vecResData;
		if (lvl2vec.empty())
			return;

		const auto& lvl3tup = lvl2vec[idlvl2].stResLvL3;
		const auto& lvl3vec = lvl3tup.vecResData;
		if (lvl3vec.empty())
			return;

		//Resource pData and resource type to show in CViewRightBR.
		m_stResData.wTypeID = rootvec[idlvlRoot].stResDirEntry.Id;
		m_stResData.wsvTypeStr = rootvec[idlvlRoot].wstrResName;
		m_stResData.wNameID = lvl2vec[idlvl2].stResDirEntry.Id;
		m_stResData.wLangID = lvl3vec[idlvl3].stResDirEntry.Id;
		m_stResData.spnData = lvl3vec[idlvl3].vecRawResData;

		m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_SHOW_RESOURCE_RBR, 0), reinterpret_cast<CObject*>(&m_stResData));
	}
	else { //Update by default, with no pData — to clear the view.
		m_pMainDoc->UpdateAllViews(this, MAKELPARAM(IDC_SHOW_RESOURCE_RBR, 0), nullptr);
	}
}

void CViewRightBL::OnTreeRClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	const auto& pstResRoot = m_pMainDoc->GetResources();
	if (!pstResRoot)
		return;

	const auto dwMsgPos = GetMessagePos();
	const CPoint ptScreen(GET_X_LPARAM(dwMsgPos), GET_Y_LPARAM(dwMsgPos));
	auto ptClient = ptScreen;
	m_treeResBottom.ScreenToClient(&ptClient);
	const auto hTreeItem = m_treeResBottom.HitTest(ptClient);
	if (!hTreeItem)
		return;

	m_treeResBottom.SelectItem(hTreeItem); //This also sets m_stResData in OnTreeSelChanged.
	CMenu menu;
	menu.CreatePopupMenu();
	std::wstring_view wsvMenu;
	const auto [iLvLRootID, iLvL2ID, iLvL3ID, eResType] = m_vecResId.at(m_treeResBottom.GetItemData(hTreeItem));
	m_eResType = eResType;
	UINT_PTR uMenuID;
	using enum EResType;
	if (iLvL2ID < 0 || iLvL3ID < 0) { //Header item clicked (RT_ICON, RT_BITMAP, etc...)
		switch (eResType) {
		case RTYPE_CURSOR:
			wsvMenu = L"Extract all Cursors...";
			break;
		case RTYPE_BITMAP:
			wsvMenu = L"Extract all Bitmaps...";
			break;
		case RTYPE_ICON:
			wsvMenu = L"Extract all Icons...";
			break;
		case RTYPE_PNG:
			wsvMenu = L"Extract all PNG...";
			break;
		default:
			break;
		}
		uMenuID = IDM_EXTRACT_ALLRES;
	}
	else { //Child node clicked.
		switch (eResType) {
		case RTYPE_CURSOR:
			wsvMenu = L"Extract Cursor...";
			break;
		case RTYPE_BITMAP:
			wsvMenu = L"Extract Bitmap...";
			break;
		case RTYPE_ICON:
			wsvMenu = L"Extract Icon...";
			break;
		case RTYPE_PNG:
			wsvMenu = L"Extract PNG...";
			break;
		default:
			break;
		}
		uMenuID = IDM_EXTRACT_RES;
	}
	if (wsvMenu.empty()) //Clicked on the resource that is unextractable.
		return;

	menu.AppendMenuW(MF_STRING, uMenuID, wsvMenu.data());
	menu.TrackPopupMenuEx(TPM_LEFTALIGN, ptScreen.x, ptScreen.y, this, nullptr);
}

BOOL CViewRightBL::OnCommand(WPARAM wParam, LPARAM lParam)
{
	const auto wMenuID = LOWORD(wParam);
	if (wMenuID == IDM_EXTRACT_RES) {
		ExtractResToFile(m_eResType, m_stResData.spnData);
	}
	else if (wMenuID == IDM_EXTRACT_ALLRES) {
		std::wstring wstrDocName = m_pDocument->GetPathName().GetString();
		wstrDocName = wstrDocName.substr(wstrDocName.find_last_of(L'\\') + 1); //Doc name with .extension.
		ExtractAllResToFile(m_pMainDoc->GetResources(), m_eResType, wstrDocName);
	}

	return CView::OnCommand(wParam, lParam);
}

BOOL CViewRightBL::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT * pResult)
{
	return CView::OnNotify(wParam, lParam, pResult);
}

void CViewRightBL::OnDocEditMode()
{
	if (m_stHexEdit->IsDataSet())
		m_stHexEdit->SetMutable(m_pMainDoc->IsEditMode());
}

void CViewRightBL::OnMDITabActivate(bool fActivate)
{
	if (fActivate) { //Show all windows from the vector, when tab is activated.
		for (const auto hWnd : m_vecHWNDVisible) {
			if (::IsWindow(hWnd)) {
				::ShowWindow(hWnd, SW_SHOW);
			}
		}
		m_vecHWNDVisible.clear();
	}
	else { //Hide all opened HexCtrl dialog windows and add them to the vector, when tab is deactivated.
		for (const auto eWnd : g_arrHexDlgs) {
			const auto hWnd = m_stHexEdit->GetWndHandle(eWnd, false);
			if (::IsWindow(hWnd) && ::IsWindowVisible(hWnd)) {
				m_vecHWNDVisible.emplace_back(hWnd);
				::ShowWindow(hWnd, SW_HIDE);
			}
		}
	}
}

void CViewRightBL::CreateListExportFuncs()
{
	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_EXPORT_FUNCS;
	m_listExportFuncs.Create(m_stlcs);
	m_listExportFuncs.ShowWindow(SW_HIDE);
	m_listExportFuncs.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listExportFuncs.SetHdrColumnColor(0, g_clrOffset);
	m_listExportFuncs.InsertColumn(1, L"Function RVA", LVCFMT_CENTER, 100);
	m_listExportFuncs.InsertColumn(2, L"Ordinal", LVCFMT_CENTER, 100);
	m_listExportFuncs.InsertColumn(3, L"Name RVA", LVCFMT_CENTER, 100);
	m_listExportFuncs.InsertColumn(4, L"Name", LVCFMT_CENTER, 250);
	m_listExportFuncs.InsertColumn(5, L"Forwarder Name", LVCFMT_CENTER, 400);

	const auto& pExport = m_pMainDoc->GetExport();
	if (!pExport)
		return;

	int listindex = 0;
	const auto dwOffset = m_pMainDoc->GetOffsetFromRVA(pExport->stExportDesc.AddressOfFunctions);
	m_listExportFuncs.SetRedraw(FALSE); //to increase the speed of List populating
	for (const auto& iterFuncs : pExport->vecFuncs) {
		m_listExportFuncs.InsertItem(listindex, std::format(L"{:08X}", static_cast<DWORD>(dwOffset + sizeof(DWORD) * iterFuncs.dwOrdinal)).data());
		m_listExportFuncs.SetItemText(listindex, 1, std::format(L"{:08X}", iterFuncs.dwFuncRVA).data());
		m_listExportFuncs.SetItemText(listindex, 2, std::format(L"{}", iterFuncs.dwOrdinal).data());
		m_listExportFuncs.SetItemText(listindex, 3, std::format(L"{:08X}", iterFuncs.dwNameRVA).data());
		m_listExportFuncs.SetItemText(listindex, 4, StrToWstr(iterFuncs.strFuncName).data());
		m_listExportFuncs.SetItemText(listindex, 5, StrToWstr(iterFuncs.strForwarderName).data());
		++listindex;
	}
	m_listExportFuncs.SetRedraw(TRUE);
}

void CViewRightBL::CreateListImportFuncs()
{
	if (!m_pMainDoc->GetImport())
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_IMPORT_ENTRY;
	m_listImportEntry.Create(m_stlcs);
	m_listImportEntry.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listImportEntry.SetHdrColumnColor(0, g_clrOffset);
	m_listImportEntry.InsertColumn(1, L"Function Name", LVCFMT_CENTER, 175);
	m_listImportEntry.InsertColumn(2, L"Ordinal / Hint", LVCFMT_CENTER, 100);
	m_listImportEntry.InsertColumn(3, L"AddressOfData", LVCFMT_CENTER, 150);
	m_listImportEntry.InsertColumn(4, L"Thunk RVA", LVCFMT_CENTER, 150);
}

void CViewRightBL::CreateTreeResources()
{
	const auto& pstResRoot = m_pMainDoc->GetResources();
	if (!pstResRoot)
		return;

	m_treeResBottom.Create(TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CRect(0, 0, 0, 0), this, IDC_TREE_RESOURCE_BOTTOM);
	m_treeResBottom.ShowWindow(SW_HIDE);

	//Scaling factor for HighDPI displays.
	const auto pDC = GetDC();
	const auto fScale = GetDeviceCaps(pDC->m_hDC, LOGPIXELSY) / 96.0F;
	ReleaseDC(pDC);
	const auto iImgSize = static_cast<int>(16 * fScale);

	m_imglTreeRes.Create(iImgSize, iImgSize, ILC_COLOR32, 0, 4);
	const auto iIconDirsID = m_imglTreeRes.Add(AfxGetApp()->LoadIconW(IDI_TREE_PERESOURCE));
	m_treeResBottom.SetImageList(&m_imglTreeRes, TVSIL_NORMAL);
	long ilvlRoot = 0;

	//Creating a treeCtrl and setting, with SetItemData(),
	//a unique id for each node, that is an index in vector (m_vecResId),
	//that holds tuple of three IDs of resource — Type, Name, LangID.
	for (const auto& iterRoot : pstResRoot->vecResData) {
		EResType eResType { EResType::RTYPE_UNSUPPORTED }; //By default set resource to unsupported.
		const auto pResDirEntry = &iterRoot.stResDirEntry;
		std::wstring wstrResName;
		if (pResDirEntry->NameIsString) {
			if (iterRoot.wstrResName == L"PNG") {
				eResType = EResType::RTYPE_PNG;
			}
			wstrResName = std::format(L"«{}»", iterRoot.wstrResName);
		}
		else { //Setting TreeCtrl root node name depending on Resource TypeID.
			if (const auto iter = MapResID.find(pResDirEntry->Id); iter != MapResID.end()) {
				wstrResName = std::format(L"{} [Id: {}]", iter->second, pResDirEntry->Id);
				eResType = static_cast<EResType>(pResDirEntry->Id);
			}
			else {
				wstrResName = std::format(L"{}", pResDirEntry->Id);
			}
		}
		const auto hTreeTop = m_treeResBottom.InsertItem(wstrResName.data(), iIconDirsID, iIconDirsID);

		m_vecResId.emplace_back(ilvlRoot, -1, -1, eResType);
		m_treeResBottom.SetItemData(hTreeTop, m_vecResId.size() - 1);
		long ilvl2 = 0;
		const auto& refResLvL2 = iterRoot.stResLvL2; //Resource level 2.

		for (const auto& iterLvL2 : refResLvL2.vecResData) {
			const auto pResDirEntry2 = &iterLvL2.stResDirEntry;
			m_vecResId.emplace_back(ilvlRoot, ilvl2, -1, eResType);
			long ilvl3 = 0;
			const auto& refResLvL3 = iterLvL2.stResLvL3;

			for (const auto& iterLvL3 : refResLvL3.vecResData) {
				const auto pResDirEntry3 = &iterLvL3.stResDirEntry;
				if (pResDirEntry2->NameIsString) { //Checking level2 name, not 3.
					wstrResName = std::format(L"Name: {} lang: {}", iterLvL2.wstrResName, pResDirEntry3->Id);
				}
				else {
					wstrResName = std::format(L"Id: {} lang: {}", iterLvL2.stResDirEntry.Id, pResDirEntry3->Id);
				}
				m_vecResId.emplace_back(ilvlRoot, ilvl2, ilvl3, eResType);
				m_treeResBottom.SetItemData(m_treeResBottom.InsertItem(wstrResName.data(), hTreeTop), m_vecResId.size() - 1);
				++ilvl3;
			}
			++ilvl2;
		}
		++ilvlRoot;
	}
}

void CViewRightBL::CreateListRelocsEntry()
{
	if (!m_pMainDoc->GetRelocations())
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_RELOCATIONS_ENTRY;
	m_listRelocsEntry.Create(m_stlcs);
	m_listRelocsEntry.ShowWindow(SW_HIDE);
	m_listRelocsEntry.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listRelocsEntry.SetHdrColumnColor(0, g_clrOffset);
	m_listRelocsEntry.InsertColumn(1, L"Reloc type", LVCFMT_CENTER, 250);
	m_listRelocsEntry.InsertColumn(2, L"Offset to apply", LVCFMT_CENTER, 120);
}

void CViewRightBL::CreateListDelayImpFuncs()
{
	if (!m_pMainDoc->GetDelayImport())
		return;

	m_stlcs.dwStyle = 0;
	m_stlcs.uID = IDC_LIST_DELAYIMPORT_ENTRY;
	m_listDelayImportEntry.Create(m_stlcs);
	m_listDelayImportEntry.InsertColumn(0, L"Offset", LVCFMT_CENTER, 90);
	m_listDelayImportEntry.SetHdrColumnColor(0, g_clrOffset);
	m_listDelayImportEntry.InsertColumn(1, L"Function Name", LVCFMT_CENTER, 300);
	m_listDelayImportEntry.InsertColumn(2, L"Ordinal / Hint", LVCFMT_CENTER, 100);
	m_listDelayImportEntry.InsertColumn(3, L"ImportNameTable AddresOfData", 0, 220);
	m_listDelayImportEntry.InsertColumn(4, L"IAT AddresOfData", LVCFMT_CENTER, 200);
	m_listDelayImportEntry.InsertColumn(5, L"BoundIAT AddresOfData", LVCFMT_CENTER, 230);
	m_listDelayImportEntry.InsertColumn(6, L"UnloadInfoTable AddresOfData", LVCFMT_CENTER, 240);
}

void CViewRightBL::ShowDosHdrHexEntry(DWORD dwEntry)
{
	if (dwEntry >= g_mapDOSHeader.size())
		return;

	if (m_hwndActive != m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN)) {
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN);
	}

	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	const auto& ref = g_mapDOSHeader.at(dwEntry);
	m_pFileLoader->ShowOffsetInWholeFile(ref.dwOffset, ref.dwSize, m_stHexEdit.get());
}

void CViewRightBL::ShowRichHdrHexEntry(DWORD dwEntry)
{
	const auto& pRichHeader = m_pMainDoc->GetRichHeader();
	if (!pRichHeader || dwEntry >= pRichHeader->size())
		return;

	if (m_hwndActive != m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN)) {
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	//Each «Rich» takes 8 bytes (two DWORDs).
	m_pFileLoader->ShowOffsetInWholeFile(pRichHeader->at(dwEntry).dwOffset, 8, m_stHexEdit.get());
}

void CViewRightBL::ShowNtHdrHexEntry()
{
	const auto& pNTHdr = m_pMainDoc->GetNTHeader();
	if (!pNTHdr)
		return;

	if (m_hwndActive != m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN)) {
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_pFileLoader->ShowOffsetInWholeFile(pNTHdr->dwOffset, sizeof(DWORD), m_stHexEdit.get()); //Shows NTHDR signature.
}

void CViewRightBL::ShowFileHdrHexEntry(DWORD dwEntry)
{
	const auto& pNTHdr = m_pMainDoc->GetNTHeader();
	if (!pNTHdr || dwEntry >= g_mapFileHeader.size())
		return;

	if (m_hwndActive != m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN)) {
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	const auto dwOffset = pNTHdr->dwOffset + offsetof(IMAGE_NT_HEADERS32, FileHeader) + g_mapFileHeader.at(dwEntry).dwOffset;
	m_pFileLoader->ShowOffsetInWholeFile(dwOffset, g_mapFileHeader.at(dwEntry).dwSize, m_stHexEdit.get());
}

void CViewRightBL::ShowOptHdrHexEntry(DWORD dwEntry)
{
	const auto& pNTHdr = m_pMainDoc->GetNTHeader();
	if (!pNTHdr)
		return;

	if (m_hwndActive != m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN)) {
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN);
	}

	DWORD dwOffset { };
	DWORD dwSize { };
	if (m_stFileInfo.eFileType == EFileType::PE32) {
		dwOffset = pNTHdr->dwOffset + offsetof(IMAGE_NT_HEADERS32, OptionalHeader) + g_mapOptHeader32.at(dwEntry).dwOffset;
		dwSize = g_mapOptHeader32.at(dwEntry).dwSize;
	}
	else if (m_stFileInfo.eFileType == EFileType::PE64) {
		dwOffset = pNTHdr->dwOffset + offsetof(IMAGE_NT_HEADERS64, OptionalHeader) + g_mapOptHeader64.at(dwEntry).dwOffset;
		dwSize = g_mapOptHeader64.at(dwEntry).dwSize;
	}

	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_pFileLoader->ShowOffsetInWholeFile(dwOffset, dwSize, m_stHexEdit.get());
}

void CViewRightBL::ShowDataDirsHexEntry(DWORD dwEntry)
{
	const auto& pNTHdr = m_pMainDoc->GetNTHeader();
	if (!pNTHdr)
		return;

	if (m_hwndActive != m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN)) {
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN);
	}

	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	const auto dwOffset = m_stFileInfo.eFileType == EFileType::PE32 ? pNTHdr->dwOffset + offsetof(IMAGE_NT_HEADERS32, OptionalHeader)
		+ offsetof(IMAGE_OPTIONAL_HEADER32, DataDirectory) + sizeof(IMAGE_DATA_DIRECTORY) * dwEntry
		: pNTHdr->dwOffset + offsetof(IMAGE_NT_HEADERS64, OptionalHeader)
		+ offsetof(IMAGE_OPTIONAL_HEADER64, DataDirectory) + sizeof(IMAGE_DATA_DIRECTORY) * dwEntry;
	m_pFileLoader->ShowOffsetInWholeFile(dwOffset, sizeof(IMAGE_DATA_DIRECTORY), m_stHexEdit.get());
}

void CViewRightBL::ShowSecHdrHexEntry(DWORD dwEntry)
{
	const auto& pSecHeaders = m_pMainDoc->GetSecHeaders();
	if (!pSecHeaders || dwEntry >= pSecHeaders->size())
		return;

	if (m_hwndActive != m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN)) {
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	const auto& ref = pSecHeaders->at(dwEntry).stSecHdr;
	m_pFileLoader->ShowFilePiece(ref.PointerToRawData, ref.SizeOfRawData, m_stHexEdit.get());
}

void CViewRightBL::ShowLCDHexEntry(DWORD dwEntry)
{
	const auto& pLCD = m_pMainDoc->GetLoadConfig();
	if (!pLCD || dwEntry >= g_mapLCD32.size())
		return;

	if (m_hwndActive != m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN)) {
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	DWORD dwOffset = pLCD->dwOffset;
	DWORD dwSize;
	if (m_stFileInfo.eFileType == EFileType::PE32) {
		dwOffset += g_mapLCD32.at(dwEntry).dwOffset;
		dwSize = g_mapLCD32.at(dwEntry).dwSize;
	}
	else {
		dwOffset += g_mapLCD64.at(dwEntry).dwOffset;
		dwSize = g_mapLCD64.at(dwEntry).dwSize;
	}
	m_pFileLoader->ShowOffsetInWholeFile(dwOffset, dwSize, m_stHexEdit.get());
}

void CViewRightBL::ShowImportListEntry(DWORD dwEntry)
{
	const auto& pImport = m_pMainDoc->GetImport();
	if (!pImport || dwEntry >= pImport->size())
		return;

	m_listImportEntry.DeleteAllItems();

	if (m_hwndActive)
		::ShowWindow(m_hwndActive, SW_HIDE);
	m_hwndActive = m_listImportEntry.GetHWND();

	int listindex = 0;
	const auto& rImp = pImport->at(dwEntry).stImportDesc;
	auto dwThunkOffset = m_pMainDoc->GetOffsetFromRVA(rImp.OriginalFirstThunk ? rImp.OriginalFirstThunk : rImp.FirstThunk);
	auto dwThunkRVA = rImp.OriginalFirstThunk ? rImp.OriginalFirstThunk : rImp.FirstThunk;

	m_listImportEntry.SetRedraw(false);
	for (const auto& iterFuncs : pImport->at(dwEntry).vecImportFunc) {
		m_listImportEntry.InsertItem(listindex, std::format(L"{:08X}", dwThunkOffset).data());
		dwThunkOffset += m_stFileInfo.eFileType == EFileType::PE32 ? sizeof(IMAGE_THUNK_DATA32) : sizeof(IMAGE_THUNK_DATA64);
		m_listImportEntry.SetItemText(listindex, 1, StrToWstr(iterFuncs.strFuncName).data());
		m_listImportEntry.SetItemText(listindex, 2, std::format(L"{:04X}", m_stFileInfo.eFileType == EFileType::PE32 ?
			((iterFuncs.unThunk.stThunk32.u1.Ordinal & IMAGE_ORDINAL_FLAG32) ?
				IMAGE_ORDINAL32(iterFuncs.unThunk.stThunk32.u1.Ordinal) : iterFuncs.stImpByName.Hint)
			: ((iterFuncs.unThunk.stThunk64.u1.Ordinal & IMAGE_ORDINAL_FLAG64) ?
				IMAGE_ORDINAL64(iterFuncs.unThunk.stThunk64.u1.Ordinal) : iterFuncs.stImpByName.Hint)).data());

		std::wstring wstrAddr;
		if (m_stFileInfo.eFileType == EFileType::PE32) {
			wstrAddr = std::format(L"{:08X}", iterFuncs.unThunk.stThunk32.u1.AddressOfData);
		}
		else if (m_stFileInfo.eFileType == EFileType::PE64) {
			wstrAddr = std::format(L"{:016X}", iterFuncs.unThunk.stThunk64.u1.AddressOfData);
		}
		m_listImportEntry.SetItemText(listindex, 3, wstrAddr.data());
		m_listImportEntry.SetItemText(listindex, 4, std::format(L"{:08X}", dwThunkRVA).data());
		dwThunkRVA += m_stFileInfo.eFileType == EFileType::PE32 ? sizeof(IMAGE_THUNK_DATA32) : sizeof(IMAGE_THUNK_DATA64);
		++listindex;
	}
	m_listImportEntry.SetRedraw(true);

	CRect rc;
	GetClientRect(&rc);
	m_listImportEntry.SetWindowPos(m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
}

void CViewRightBL::ShowSecurityHexEntry(unsigned nSertId)
{
	const auto& pSec = m_pMainDoc->GetSecurity();
	if (!pSec || nSertId >= pSec->size())
		return;

	const auto& secEntry = pSec->at(nSertId).stWinSert;
	const auto dwStart = pSec->at(nSertId).dwOffset + offsetof(PEWIN_CERTIFICATE, bCertificate);
	const auto dwCertSize = static_cast<DWORD_PTR>(secEntry.dwLength) - offsetof(PEWIN_CERTIFICATE, bCertificate);
	m_pFileLoader->ShowFilePiece(dwStart, dwCertSize, m_stHexEdit.get());

	if (m_hwndActive != m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN)) {
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
}

void CViewRightBL::ShowRelocsListEntry(DWORD dwEntry)
{
	const auto& pReloc = m_pMainDoc->GetRelocations();
	if (!pReloc || dwEntry >= pReloc->size())
		return;

	m_listRelocsEntry.DeleteAllItems();
	int listindex = 0;
	m_listRelocsEntry.SetRedraw(false);
	for (const auto& iterRelocs : pReloc->at(dwEntry).vecRelocData) {
		m_listRelocsEntry.InsertItem(listindex, std::format(L"{:08X}", iterRelocs.dwOffset).data());

		if (const auto type = MapRelocType.find(iterRelocs.wRelocType); type != MapRelocType.end()) {
			m_listRelocsEntry.SetItemText(listindex, 1, type->second.data());
		}
		else {
			m_listRelocsEntry.SetItemText(listindex, 1, std::format(L"{}", iterRelocs.wRelocType).data());
		}

		m_listRelocsEntry.SetItemText(listindex, 2, std::format(L"{:04X}", iterRelocs.wRelocOffset).data());
		++listindex;
	}
	m_listRelocsEntry.SetRedraw(true);

	CRect rc;
	GetClientRect(&rc);
	m_listRelocsEntry.SetWindowPos(m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_hwndActive = m_listRelocsEntry.GetHWND();
}

void CViewRightBL::ShowDelayImpListEntry(DWORD dwEntry)
{
	const auto& pDelayImport = m_pMainDoc->GetDelayImport();
	if (!pDelayImport || dwEntry >= pDelayImport->size())
		return;

	m_listDelayImportEntry.DeleteAllItems();

	int listindex = 0;
	auto dwThunkOffset = m_pMainDoc->GetOffsetFromRVA(pDelayImport->at(dwEntry).stDelayImpDesc.ImportNameTableRVA);

	m_listDelayImportEntry.SetRedraw(FALSE);
	for (const auto& iterFuncs : pDelayImport->at(dwEntry).vecDelayImpFunc) {
		m_listDelayImportEntry.InsertItem(listindex, std::format(L"{:08X}", dwThunkOffset).data());
		dwThunkOffset += m_stFileInfo.eFileType == EFileType::PE32 ? sizeof(IMAGE_THUNK_DATA32) : sizeof(IMAGE_THUNK_DATA64);
		m_listDelayImportEntry.SetItemText(listindex, 1, StrToWstr(iterFuncs.strFuncName).data());
		m_listDelayImportEntry.SetItemText(listindex, 2, std::format(L"{:04X}", m_stFileInfo.eFileType == EFileType::PE32 ?
			((iterFuncs.unThunk.st32.stImportNameTable.u1.Ordinal & IMAGE_ORDINAL_FLAG32) ?
				IMAGE_ORDINAL32(iterFuncs.unThunk.st32.stImportNameTable.u1.Ordinal) : iterFuncs.stImpByName.Hint)
			: ((iterFuncs.unThunk.st64.stImportNameTable.u1.Ordinal & IMAGE_ORDINAL_FLAG64) ?
				IMAGE_ORDINAL64(iterFuncs.unThunk.st64.stImportNameTable.u1.Ordinal) : iterFuncs.stImpByName.Hint)).data());

		std::wstring wstrAddr;
		if (m_stFileInfo.eFileType == EFileType::PE32) {
			wstrAddr = std::format(L"{:08X}", iterFuncs.unThunk.st32.stImportNameTable.u1.AddressOfData);
		}
		else if (m_stFileInfo.eFileType == EFileType::PE64) {
			wstrAddr = std::format(L"{:016X}", iterFuncs.unThunk.st64.stImportNameTable.u1.AddressOfData);
		}
		m_listDelayImportEntry.SetItemText(listindex, 3, wstrAddr.data());

		if (m_stFileInfo.eFileType == EFileType::PE32) {
			wstrAddr = std::format(L"{:08X}", iterFuncs.unThunk.st32.stImportAddressTable.u1.AddressOfData);
		}
		else if (m_stFileInfo.eFileType == EFileType::PE64) {
			wstrAddr = std::format(L"{:016X}", iterFuncs.unThunk.st64.stImportAddressTable.u1.AddressOfData);
		}
		m_listDelayImportEntry.SetItemText(listindex, 4, wstrAddr.data());

		if (m_stFileInfo.eFileType == EFileType::PE32) {
			wstrAddr = std::format(L"{:08X}", iterFuncs.unThunk.st32.stBoundImportAddressTable.u1.AddressOfData);
		}
		else if (m_stFileInfo.eFileType == EFileType::PE64) {
			wstrAddr = std::format(L"{:016X}", iterFuncs.unThunk.st64.stBoundImportAddressTable.u1.AddressOfData);
		}
		m_listDelayImportEntry.SetItemText(listindex, 5, wstrAddr.data());

		if (m_stFileInfo.eFileType == EFileType::PE32) {
			wstrAddr = std::format(L"{:08X}", iterFuncs.unThunk.st32.stUnloadInformationTable.u1.AddressOfData);
		}
		else if (m_stFileInfo.eFileType == EFileType::PE64) {
			wstrAddr = std::format(L"{:016X}", iterFuncs.unThunk.st64.stUnloadInformationTable.u1.AddressOfData);
		}
		m_listDelayImportEntry.SetItemText(listindex, 6, wstrAddr.data());

		++listindex;
	}
	m_listDelayImportEntry.SetRedraw(true);

	CRect rect;
	GetClientRect(&rect);
	m_listDelayImportEntry.SetWindowPos(m_hWnd, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
	m_hwndActive = m_listDelayImportEntry.GetHWND();
}

void CViewRightBL::ShowDebugHexEntry(DWORD dwEntry)
{
	const auto& pDebug = m_pMainDoc->GetDebug();
	if (!pDebug || dwEntry >= pDebug->size())
		return;

	const auto& rDebugDir = pDebug->at(dwEntry).stDebugDir;
	m_pFileLoader->ShowFilePiece(rDebugDir.PointerToRawData, rDebugDir.SizeOfData, m_stHexEdit.get());

	if (m_hwndActive != m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN)) {
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
}

void CViewRightBL::ShowTLSHex()
{
	const auto& pTLS = m_pMainDoc->GetTLS();
	if (!pTLS)
		return;

	if (m_hwndActive != m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN)) {
		if (m_hwndActive)
			::ShowWindow(m_hwndActive, SW_HIDE);
		m_hwndActive = m_stHexEdit->GetWndHandle(EHexWnd::WND_MAIN);
	}
	CRect rc;
	GetClientRect(&rc);
	::SetWindowPos(m_hwndActive, m_hWnd, 0, 0, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	ULONGLONG ullStartAdr { };
	ULONGLONG ullEndAdr { };
	if (m_stFileInfo.eFileType == EFileType::PE32) {
		ullStartAdr = pTLS->unTLS.stTLSDir32.StartAddressOfRawData;
		ullEndAdr = pTLS->unTLS.stTLSDir32.EndAddressOfRawData;
	}
	else if (m_stFileInfo.eFileType == EFileType::PE64) {
		ullStartAdr = pTLS->unTLS.stTLSDir64.StartAddressOfRawData;
		ullEndAdr = pTLS->unTLS.stTLSDir64.EndAddressOfRawData;
	}

	const auto dwOffsetStart = m_pMainDoc->GetOffsetFromVA(ullStartAdr);
	const auto dwOffsetEnd = m_pMainDoc->GetOffsetFromVA(ullEndAdr);
	const auto dwSize = dwOffsetEnd > dwOffsetStart ? dwOffsetEnd - dwOffsetStart : 0U;
	m_pFileLoader->ShowFilePiece(dwOffsetStart, dwSize, m_stHexEdit.get());
}

auto CViewRightBL::GetListByID(UINT_PTR uListID)->CListEx*
{
	switch (uListID) {
	case IDC_LIST_EXPORT_FUNCS:
		return &m_listExportFuncs;
	case IDC_LIST_IMPORT_ENTRY:
		return &m_listImportEntry;
	case IDC_LIST_DELAYIMPORT_ENTRY:
		return &m_listDelayImportEntry;
	case IDC_LIST_RELOCATIONS_ENTRY:
		return &m_listRelocsEntry;
	default:
		return { };
	}
}

void CViewRightBL::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (const auto pList = GetListByID(nIDCtl); pList != nullptr) {
		pList->DrawItem(lpDrawItemStruct);
		return;
	}

	CView::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CViewRightBL::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if (const auto pList = GetListByID(nIDCtl); pList != nullptr) {
		pList->MeasureItem(lpMeasureItemStruct);
		return;
	}

	CView::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}