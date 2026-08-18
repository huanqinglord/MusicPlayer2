#pragma once
#include "BaseDialog.h"
#include "SongInfo.h"

class CMyGroupsDlg : public CBaseDialog
{
    DECLARE_DYNAMIC(CMyGroupsDlg)
public:
    explicit CMyGroupsDlg(CWnd* pParent = nullptr, bool embedded = false);
    virtual ~CMyGroupsDlg();
    virtual void OnCancel() override;
protected:
    struct SongGroup { std::wstring name; std::vector<SongKey> songs; };
    CListCtrl m_group_list;
    CListCtrl m_song_list;
    CEdit m_search_edit;
    CStatic m_group_title;
    CStatic m_group_count;
    CStatic m_empty_state;
    CToolTipCtrl m_tooltip;
    CBrush m_background_brush;
    CImageList m_group_row_height_image;
    int m_group_hover_row{ -1 };
    bool m_embedded{};
    std::vector<SongGroup> m_groups;
    std::vector<SongKey> m_manual_songs;
    std::vector<SongInfo> m_current_songs;
    std::vector<int> m_visible_song_indices;
    virtual CString GetDialogName() const override;
    virtual bool InitializeControls() override;
    virtual void DoDataExchange(CDataExchange* pDX) override;
    virtual BOOL OnInitDialog() override;
    virtual BOOL PreTranslateMessage(MSG* pMsg) override;
    void LoadGroups();
    void SaveGroups() const;
    void ShowGroups(int select_index = 0);
    void LoadCurrentGroupSongs();
    void ShowSongs();
    void UpdateButtons();
    void AddSelectedSongs(int group_row);
    void AddFolderToGroup(int group_row);
    void ShowGroupMenu(int group_row);
    void ShowSongMenu(int song_index);
    std::vector<SongInfo> GetPlayableSongs(const std::vector<SongInfo>& songs) const;
    std::vector<SongInfo> GetSongsForGroup(int group_row);
    void AddSongsToGroup(const std::vector<SongInfo>& songs);
    void UpdateTooltip(CWnd* source, CPoint point);
    int GetSelectedGroupIndex() const;
    bool IsGroupNameValid(const std::wstring& name, int ignored_index = -1) const;
    afx_msg void OnNewGroup();
    afx_msg void OnRenameGroup();
    afx_msg void OnDeleteGroup();
    afx_msg void OnSearchChanged();
    afx_msg void OnBack();
    afx_msg void OnPlayAll();
    afx_msg void OnAddFiles();
    afx_msg void OnAddFolder();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    afx_msg void OnGroupCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnGroupClicked(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSongClicked(NMHDR* pNMHDR, LRESULT* pResult);
    DECLARE_MESSAGE_MAP()
};
