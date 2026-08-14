#include "stdafx.h"
#include "MusicPlayer2.h"
#include "MyGroupsDlg.h"
#include "AudioCommon.h"
#include "InputDlg.h"
#include "Player.h"
#include "SelectItemDlg.h"
#include "SongDataManager.h"
#include "MusicPlayerDlg.h"

IMPLEMENT_DYNAMIC(CMyGroupsDlg, CBaseDialog)
CMyGroupsDlg::CMyGroupsDlg(CWnd* pParent, bool embedded) : CBaseDialog(IDD_MY_GROUPS_DIALOG, pParent), m_embedded(embedded) {}
CMyGroupsDlg::~CMyGroupsDlg() = default;
CString CMyGroupsDlg::GetDialogName() const { return m_embedded ? CString() : CString(L"MyGroupsDlg"); }

bool CMyGroupsDlg::InitializeControls()
{
    SetWindowTextW(L"\u6211\u7684\u5206\u7ec4");
    SetMinSize(theApp.DPI(720), theApp.DPI(430));
    return false;
}

void CMyGroupsDlg::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_MY_GROUP_LIST, m_group_list);
    DDX_Control(pDX, IDC_MY_GROUP_SONG_LIST, m_song_list);
    DDX_Control(pDX, IDC_MY_GROUP_SEARCH, m_search_edit);
}

BEGIN_MESSAGE_MAP(CMyGroupsDlg, CBaseDialog)
    ON_BN_CLICKED(IDC_MY_GROUP_BACK, &CMyGroupsDlg::OnBack)
    ON_EN_CHANGE(IDC_MY_GROUP_SEARCH, &CMyGroupsDlg::OnSearchChanged)
    ON_NOTIFY(NM_CLICK, IDC_MY_GROUP_LIST, &CMyGroupsDlg::OnGroupClicked)
    ON_NOTIFY(NM_CLICK, IDC_MY_GROUP_SONG_LIST, &CMyGroupsDlg::OnSongClicked)
    ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CMyGroupsDlg::OnInitDialog()
{
    CBaseDialog::OnInitDialog();
    SetIcon(IconMgr::IconType::IT_Playlist, FALSE);
    SetButtonIcon(IDC_MY_GROUP_BACK, IconMgr::IconType::IT_Arrow_Left);
    m_group_list.SetExtendedStyle(m_group_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
    m_group_list.InsertColumn(0, L"", LVCFMT_LEFT, theApp.DPI(105));
    m_group_list.InsertColumn(1, L"", LVCFMT_CENTER, theApp.DPI(24));
    m_group_list.InsertColumn(2, L"", LVCFMT_CENTER, theApp.DPI(24));
    m_group_list.InsertColumn(3, L"", LVCFMT_CENTER, theApp.DPI(24));
    m_song_list.SetExtendedStyle(m_song_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_song_list.InsertColumn(0, L"\u6587\u4ef6\u540d", LVCFMT_LEFT, theApp.DPI(220));
    m_song_list.InsertColumn(1, L"\u6587\u4ef6\u8def\u5f84", LVCFMT_LEFT, theApp.DPI(340));
    m_song_list.InsertColumn(2, L"\u64cd\u4f5c", LVCFMT_CENTER, theApp.DPI(150));
    m_tooltip.Create(this, TTS_ALWAYSTIP | TTS_NOPREFIX);
    m_tooltip.AddTool(GetDlgItem(IDC_MY_GROUP_BACK), theApp.m_str_table.LoadText(L"UI_TIP_BTN_BACK").c_str());
    m_tooltip.AddTool(&m_group_list, L"");
    m_tooltip.AddTool(&m_song_list, L"");
    LoadGroups();
    ShowGroups();
    return TRUE;
}

BOOL CMyGroupsDlg::PreTranslateMessage(MSG* pMsg)
{
    if (m_embedded && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
    {
        OnBack();
        return TRUE;
    }
    if (pMsg->message == WM_MOUSEMOVE)
    {
        CWnd* source = CWnd::FromHandle(pMsg->hwnd);
        if (source == &m_group_list || source == &m_song_list)
        {
            CPoint point(GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam));
            UpdateTooltip(source, point);
        }
    }
    m_tooltip.RelayEvent(pMsg);
    return CBaseDialog::PreTranslateMessage(pMsg);
}

void CMyGroupsDlg::OnCancel()
{
    if (m_embedded)
        OnBack();
    else
        CBaseDialog::OnCancel();
}

void CMyGroupsDlg::OnBack()
{
    if (CMusicPlayerDlg* main_dlg = CMusicPlayerDlg::GetInstance())
        main_dlg->HideMyGroupsPage();
}

void CMyGroupsDlg::OnSize(UINT nType, int cx, int cy)
{
    CBaseDialog::OnSize(nType, cx, cy);
    if (!m_group_list.GetSafeHwnd() || cx <= 0 || cy <= 0)
        return;

    const int margin = theApp.DPI(12);
    const int gap = theApp.DPI(12);
    const int header_height = theApp.DPI(32);
    const int list_top = margin + header_height + gap;
    const int group_width = max(theApp.DPI(180), cx * 28 / 100);
    const int content_height = max(1, cy - list_top - margin);

    if (CWnd* back = GetDlgItem(IDC_MY_GROUP_BACK))
        back->MoveWindow(margin, margin, theApp.DPI(40), header_height);
    m_search_edit.MoveWindow(margin + group_width + gap, margin, max(1, cx - group_width - gap - margin * 2), header_height);
    m_group_list.MoveWindow(margin, list_top, group_width, content_height);
    m_song_list.MoveWindow(margin + group_width + gap, list_top, max(1, cx - group_width - gap - margin * 2), content_height);

    CRect song_client_rect;
    m_song_list.GetClientRect(&song_client_rect);
    const int song_width = song_client_rect.Width();
    if (song_width > 0)
    {
        m_song_list.SetColumnWidth(0, song_width * 35 / 100);
        m_song_list.SetColumnWidth(1, song_width * 45 / 100);
        m_song_list.SetColumnWidth(2, song_width * 20 / 100);
    }
}

void CMyGroupsDlg::LoadGroups()
{
    m_groups.clear();
    CFile file;
    if (!file.Open((theApp.m_playlist_dir + L"my_groups.dat").c_str(), CFile::modeRead | CFile::shareDenyWrite)) return;
    try
    {
        CArchive ar(&file, CArchive::load);
        CString version;
        int group_count{};
        ar >> version >> group_count;
        if (version != L"1" || group_count < 0 || group_count > 1000) return;
        for (int i{}; i < group_count; ++i)
        {
            SongGroup group;
            CString name;
            int song_count{};
            ar >> name >> song_count;
            group.name = name;
            if (song_count < 0 || song_count > MAX_SONG_NUM) return;
            for (int j{}; j < song_count; ++j)
            {
                CString path;
                int cue_track{};
                ar >> path >> cue_track;
                if (!path.IsEmpty()) group.songs.emplace_back(std::wstring(path), cue_track);
            }
            if (!group.name.empty()) m_groups.push_back(std::move(group));
        }
    }
    catch (CException* exception) { exception->Delete(); m_groups.clear(); }
}

void CMyGroupsDlg::SaveGroups() const
{
    CFile file;
    if (!file.Open((theApp.m_playlist_dir + L"my_groups.dat").c_str(), CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive)) return;
    CArchive ar(&file, CArchive::store);
    ar << CString(L"1") << static_cast<int>(m_groups.size());
    for (const SongGroup& group : m_groups)
    {
        ar << CString(group.name.c_str()) << static_cast<int>(group.songs.size());
        for (const SongKey& song : group.songs) ar << CString(song.path.c_str()) << song.cue_track;
    }
}

void CMyGroupsDlg::ShowGroups(int select_index)
{
    m_group_list.DeleteAllItems();
    int row = m_group_list.InsertItem(0, L"\u6240\u6709\u6b4c\u66f2");
    m_group_list.SetItemText(row, 1, L"\u25b6");
    for (const SongGroup& group : m_groups)
    {
        row = m_group_list.InsertItem(m_group_list.GetItemCount(), group.name.c_str());
        m_group_list.SetItemText(row, 1, L"\u25b6");
        m_group_list.SetItemText(row, 2, L"\u270e");
        m_group_list.SetItemText(row, 3, L"\u2715");
    }
    m_group_list.InsertItem(m_group_list.GetItemCount(), L"+");
    select_index = (std::max)(0, (std::min)(select_index, static_cast<int>(m_groups.size())));
    m_group_list.SetItemState(select_index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    LoadCurrentGroupSongs();
}

int CMyGroupsDlg::GetSelectedGroupIndex() const
{
    return m_group_list.GetNextItem(-1, LVNI_SELECTED);
}

void CMyGroupsDlg::LoadCurrentGroupSongs()
{
    m_current_songs.clear();
    const int group_index = GetSelectedGroupIndex();
    if (group_index <= 0)
    {
        std::vector<SongInfo> scanned_songs;
        for (const std::wstring& folder : theApp.m_media_lib_setting_data.media_folders)
            CAudioCommon::GetAudioFiles(folder, scanned_songs, MAX_SONG_NUM, true);
        std::set<SongKey> added;
        for (SongInfo& song : scanned_songs)
            if (!song.file_path.empty() && added.insert(SongKey(song)).second) m_current_songs.push_back(std::move(song));
        CSongDataManager::GetInstance().LoadSongsInfo(m_current_songs);
    }
    else if (group_index - 1 < static_cast<int>(m_groups.size()))
    {
        for (const SongKey& key : m_groups[group_index - 1].songs)
        {
            SongInfo song = CSongDataManager::GetInstance().GetSongInfo(key);
            if (song.file_path.empty())
            {
                song.file_path = key.path;
                song.track = key.cue_track;
                song.is_cue = key.cue_track != 0;
            }
            m_current_songs.push_back(std::move(song));
        }
    }
    std::sort(m_current_songs.begin(), m_current_songs.end(), [](const SongInfo& a, const SongInfo& b) {
        return CCommon::StringCompareInLocalLanguage(a.GetTitle(), b.GetTitle()) < 0;
    });
    ShowSongs();
    UpdateButtons();
}

void CMyGroupsDlg::ShowSongs()
{
    CString search_text;
    m_search_edit.GetWindowTextW(search_text);
    const std::wstring keyword = search_text.GetString();
    m_song_list.DeleteAllItems();
    m_visible_song_indices.clear();
    for (int i{}; i < static_cast<int>(m_current_songs.size()); ++i)
    {
        const SongInfo& song = m_current_songs[i];
        if (!keyword.empty() && !theApp.m_chinese_pingyin_res.IsStringMatchWithPingyin(keyword, song.GetTitle())
            && !theApp.m_chinese_pingyin_res.IsStringMatchWithPingyin(keyword, song.GetArtist())
            && !theApp.m_chinese_pingyin_res.IsStringMatchWithPingyin(keyword, song.file_path)) continue;
        const int row = m_song_list.InsertItem(m_song_list.GetItemCount(), song.GetFileName().c_str());
        m_song_list.SetItemText(row, 1, song.file_path.c_str());
        m_song_list.SetItemText(row, 2, GetSelectedGroupIndex() > 0 ? L"\u25b6    \u2212    +    \u2261" : L"\u25b6         +    \u2261");
        m_visible_song_indices.push_back(i);
    }
}

void CMyGroupsDlg::UpdateButtons()
{
}

std::vector<SongInfo> CMyGroupsDlg::GetSongsForGroup(int group_row)
{
    if (group_row < 0 || group_row > static_cast<int>(m_groups.size())) return {};
    m_group_list.SetItemState(group_row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    LoadCurrentGroupSongs();
    return m_current_songs;
}

std::vector<SongInfo> CMyGroupsDlg::GetPlayableSongs(const std::vector<SongInfo>& songs) const
{
    std::vector<SongInfo> playable;
    for (const SongInfo& song : songs)
        if (!song.file_path.empty() && CCommon::FileExist(song.file_path)) playable.push_back(song);
    return playable;
}

bool CMyGroupsDlg::IsGroupNameValid(const std::wstring& name, int ignored_index) const
{
    if (name.empty() || name == L"\u6240\u6709\u6b4c\u66f2") return false;
    for (int i{}; i < static_cast<int>(m_groups.size()); ++i)
        if (i != ignored_index && m_groups[i].name == name) return false;
    return true;
}

void CMyGroupsDlg::OnNewGroup()
{
    CInputDlg dlg(this); dlg.SetTitle(L"\u65b0\u5efa\u5206\u7ec4"); dlg.SetInfoText(L"\u8bf7\u8f93\u5165\u5206\u7ec4\u540d\u79f0\uff1a");
    if (dlg.DoModal() != IDOK) return;
    const std::wstring name = dlg.GetEditText().GetString();
    if (!IsGroupNameValid(name)) { MessageBox(L"\u5206\u7ec4\u540d\u79f0\u4e0d\u80fd\u4e3a\u7a7a\u3001\u4e0d\u80fd\u91cd\u590d\uff0c\u4e5f\u4e0d\u80fd\u4f7f\u7528\u201c\u6240\u6709\u6b4c\u66f2\u201d\u3002", L"\u6211\u7684\u5206\u7ec4", MB_ICONWARNING); return; }
    m_groups.push_back({ name, {} }); SaveGroups(); ShowGroups(static_cast<int>(m_groups.size()));
}

void CMyGroupsDlg::OnRenameGroup()
{
    const int selected = GetSelectedGroupIndex();
    if (selected <= 0 || selected - 1 >= static_cast<int>(m_groups.size())) return;
    CInputDlg dlg(this); dlg.SetTitle(L"\u91cd\u547d\u540d\u5206\u7ec4"); dlg.SetInfoText(L"\u8bf7\u8f93\u5165\u65b0\u7684\u5206\u7ec4\u540d\u79f0\uff1a"); dlg.SetEditText(m_groups[selected - 1].name.c_str());
    if (dlg.DoModal() != IDOK) return;
    const std::wstring name = dlg.GetEditText().GetString();
    if (!IsGroupNameValid(name, selected - 1)) { MessageBox(L"\u5206\u7ec4\u540d\u79f0\u4e0d\u80fd\u4e3a\u7a7a\u3001\u4e0d\u80fd\u91cd\u590d\uff0c\u4e5f\u4e0d\u80fd\u4f7f\u7528\u201c\u6240\u6709\u6b4c\u66f2\u201d\u3002", L"\u6211\u7684\u5206\u7ec4", MB_ICONWARNING); return; }
    m_groups[selected - 1].name = name; SaveGroups(); ShowGroups(selected);
}

void CMyGroupsDlg::OnDeleteGroup()
{
    const int selected = GetSelectedGroupIndex();
    if (selected <= 0 || selected - 1 >= static_cast<int>(m_groups.size())) return;
    if (MessageBox(L"\u786e\u5b9a\u5220\u9664\u5f53\u524d\u5206\u7ec4\u5417\uff1f\u6b4c\u66f2\u6587\u4ef6\u4e0d\u4f1a\u88ab\u5220\u9664\u3002", L"\u6211\u7684\u5206\u7ec4", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
    m_groups.erase(m_groups.begin() + selected - 1); SaveGroups(); ShowGroups((std::min)(selected, static_cast<int>(m_groups.size())));
}

void CMyGroupsDlg::AddSongsToGroup(const std::vector<SongInfo>& checked)
{
    if (checked.empty()) return;
    if (m_groups.empty()) { MessageBox(L"\u8bf7\u5148\u65b0\u5efa\u4e00\u4e2a\u5206\u7ec4\u3002", L"\u6211\u7684\u5206\u7ec4", MB_ICONINFORMATION); return; }
    std::vector<std::wstring> names;
    for (const SongGroup& group : m_groups) names.push_back(group.name);
    CSelectItemDlg dlg(names, this); dlg.SetTitle(L"\u9009\u62e9\u76ee\u6807\u5206\u7ec4"); dlg.SetDlgIcon(IconMgr::IconType::IT_Playlist);
    if (dlg.DoModal() != IDOK) return;
    const std::wstring selected_name = dlg.GetSelectedItem();
    auto group = std::find_if(m_groups.begin(), m_groups.end(), [&](const SongGroup& item) { return item.name == selected_name; });
    if (group == m_groups.end()) return;
    for (const SongInfo& song : checked)
    {
        const SongKey key(song);
        if (std::find(group->songs.begin(), group->songs.end(), key) == group->songs.end()) group->songs.push_back(key);
    }
    SaveGroups();
    if (GetSelectedGroupIndex() > 0 && m_groups[GetSelectedGroupIndex() - 1].name == selected_name) LoadCurrentGroupSongs();
}

void CMyGroupsDlg::OnGroupClicked(NMHDR* pNMHDR, LRESULT* pResult)
{
    const LPNMITEMACTIVATE item = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    const int plus_row = static_cast<int>(m_groups.size()) + 1;
    if (item->iItem == plus_row) OnNewGroup();
    else if (item->iItem >= 0 && item->iItem <= static_cast<int>(m_groups.size()))
    {
        m_group_list.SetItemState(item->iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        if (item->iSubItem == 1)
        {
            const auto songs = GetPlayableSongs(GetSongsForGroup(item->iItem));
            if (!songs.empty()) CPlayer::GetInstance().OpenSongsInDefaultPlaylist(songs, true);
        }
        else if (item->iSubItem == 2 && item->iItem > 0) OnRenameGroup();
        else if (item->iSubItem == 3 && item->iItem > 0) OnDeleteGroup();
        else LoadCurrentGroupSongs();
    }
    *pResult = 0;
}

void CMyGroupsDlg::OnSongClicked(NMHDR* pNMHDR, LRESULT* pResult)
{
    const LPNMITEMACTIVATE item = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    if (item->iItem >= 0 && item->iItem < static_cast<int>(m_visible_song_indices.size()) && item->iSubItem == 2)
    {
        const SongInfo song = m_current_songs[m_visible_song_indices[item->iItem]];
        CRect rect;
        m_song_list.GetSubItemRect(item->iItem, 2, LVIR_BOUNDS, rect);
        const int action = rect.Width() > 0 ? (item->ptAction.x - rect.left) * 4 / rect.Width() : -1;
        if (action == 0 && CCommon::FileExist(song.file_path)) CPlayer::GetInstance().OpenSongsInDefaultPlaylist({ song }, true);
        else if (action == 1 && GetSelectedGroupIndex() > 0)
        {
            auto& songs = m_groups[GetSelectedGroupIndex() - 1].songs;
            const SongKey key(song);
            songs.erase(std::remove(songs.begin(), songs.end(), key), songs.end());
            SaveGroups(); LoadCurrentGroupSongs();
        }
        else if (action == 2) AddSongsToGroup({ song });
        else if (action == 3 && CCommon::FileExist(song.file_path)) CPlayer::GetInstance().OpenSongsInDefaultPlaylist({ song }, false);
    }
    *pResult = 0;
}

void CMyGroupsDlg::OnSearchChanged() { ShowSongs(); }

void CMyGroupsDlg::UpdateTooltip(CWnd* source, CPoint point)
{
    LVHITTESTINFO hit{};
    hit.pt = point;
    CString text;
    if (source == &m_group_list)
    {
        m_group_list.SubItemHitTest(&hit);
        if (hit.iItem == static_cast<int>(m_groups.size()) + 1) text = L"\u65b0\u589e\u5206\u7ec4";
        else if (hit.iSubItem == 1) text = L"\u64ad\u653e\u5206\u7ec4";
        else if (hit.iSubItem == 2 && hit.iItem > 0) text = L"\u91cd\u547d\u540d\u5206\u7ec4";
        else if (hit.iSubItem == 3 && hit.iItem > 0) text = L"\u5220\u9664\u5206\u7ec4";
    }
    else
    {
        m_song_list.SubItemHitTest(&hit);
        if (hit.iSubItem == 2 && hit.iItem >= 0)
        {
            CRect rect; m_song_list.GetSubItemRect(hit.iItem, 2, LVIR_BOUNDS, rect);
            const int action = rect.Width() > 0 ? (point.x - rect.left) * 4 / rect.Width() : -1;
            if (action == 0) text = L"\u64ad\u653e";
            else if (action == 1 && GetSelectedGroupIndex() > 0) text = L"\u4ece\u5206\u7ec4\u79fb\u9664";
            else if (action == 2) text = L"\u6dfb\u52a0\u5230\u5206\u7ec4";
            else if (action == 3) text = L"\u6dfb\u52a0\u5230\u64ad\u653e\u5217\u8868";
        }
    }
    m_tooltip.UpdateTipText(text, source);
}
