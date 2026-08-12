#include "stdafx.h"
#include "CPlayerUIHelper.h"
#include "MusicPlayer2.h"
#include "Player.h"


CPlayerUIHelper::CPlayerUIHelper()
{
}


CPlayerUIHelper::~CPlayerUIHelper()
{
}

UIColors CPlayerUIHelper::GetUIColors(bool dark, bool draw_alpha)
{
    UIColors colors{};
    //if (theApp.m_app_setting_data.dark_mode)
    if (dark)
    {
        colors.color_text = RGB(255, 255, 255);
        colors.color_text_lable = RGB(200, 198, 196);
        colors.color_text_2 = RGB(161, 159, 157);
        colors.color_text_heighlight = RGB(255, 255, 255);
        colors.color_text_disabled = RGB(117, 117, 117);
        if (draw_alpha)
        {
            colors.color_back = RGB(36, 36, 36);
            colors.color_panel_back = RGB(48, 48, 48);
        }
        else
        {
            colors.color_back = RGB(32, 32, 32);
            colors.color_panel_back = RGB(44, 44, 44);
        }

        colors.color_lyric_back = theApp.m_app_setting_data.theme_color.dark3;
        if (draw_alpha)
            colors.color_control_bar_back = theApp.m_app_setting_data.theme_color.dark2;
        else
            colors.color_control_bar_back = RGB(44, 44, 44);
        colors.color_surface_hover = RGB(56, 56, 56);
        colors.color_surface_pressed = RGB(62, 62, 62);
        colors.color_border = RGB(60, 60, 60);
        colors.color_accent = theApp.m_app_setting_data.theme_color.original_color;
        colors.color_danger = RGB(196, 43, 28);
        colors.color_spectrum = theApp.m_app_setting_data.theme_color.light2;
        colors.color_spectrum_cover = theApp.m_app_setting_data.theme_color.original_color;
        colors.color_progress_back = theApp.m_app_setting_data.theme_color.dark1;
        if (draw_alpha)
            colors.color_button_back = theApp.m_app_setting_data.theme_color.dark1_5;
        else
            colors.color_button_back = GRAY(76);
        colors.color_button_checked = theApp.m_app_setting_data.theme_color.dark3;
        colors.color_stack_indicator = theApp.m_app_setting_data.theme_color.light2;
        colors.color_scrollbar_handle = theApp.m_app_setting_data.theme_color.dark1;
        if (draw_alpha)
        {
            colors.color_button_pressed = theApp.m_app_setting_data.theme_color.light2;
            colors.color_button_hover = theApp.m_app_setting_data.theme_color.light1;
            colors.color_list_selected = theApp.m_app_setting_data.theme_color.dark3;
        }
        else
        {
            colors.color_button_pressed = theApp.m_app_setting_data.theme_color.dark1;
            colors.color_button_hover = theApp.m_app_setting_data.theme_color.dark1_5;
            colors.color_list_selected = theApp.m_app_setting_data.theme_color.dark2;
        }
        colors.color_statusbar_progress_back = theApp.m_app_setting_data.theme_color.dark3;
    }
    else
    {
        colors.color_text = RGB(26, 26, 26);
        colors.color_text_lable = RGB(96, 94, 92);
        colors.color_text_2 = RGB(117, 117, 117);
        colors.color_text_heighlight = RGB(26, 26, 26);
        colors.color_text_disabled = RGB(161, 159, 157);
        colors.color_back = RGB(243, 243, 243);
        if (draw_alpha)
            colors.color_panel_back = RGB(251, 251, 251);
        else
            colors.color_panel_back = RGB(251, 251, 251);
        colors.color_lyric_back = theApp.m_app_setting_data.theme_color.light3;
        colors.color_control_bar_back = theApp.m_app_setting_data.theme_color.light3;
        colors.color_surface_hover = RGB(245, 245, 245);
        colors.color_surface_pressed = RGB(238, 238, 238);
        colors.color_border = RGB(229, 229, 229);
        colors.color_accent = theApp.m_app_setting_data.theme_color.original_color;
        colors.color_danger = RGB(196, 43, 28);
        colors.color_spectrum = theApp.m_app_setting_data.theme_color.original_color;
        colors.color_spectrum_cover = theApp.m_app_setting_data.theme_color.original_color;
        colors.color_progress_back = theApp.m_app_setting_data.theme_color.light2_5;
        colors.color_stack_indicator = theApp.m_app_setting_data.theme_color.dark0;
        colors.color_scrollbar_handle = theApp.m_app_setting_data.theme_color.light2;
        if (draw_alpha)
        {
            colors.color_button_checked = theApp.m_app_setting_data.theme_color.light2;
            colors.color_button_back = theApp.m_app_setting_data.theme_color.light2;
            colors.color_button_hover = theApp.m_app_setting_data.theme_color.light1_5;
            colors.color_button_pressed = theApp.m_app_setting_data.theme_color.light1;
        }
        else
        {
            colors.color_button_checked = theApp.m_app_setting_data.theme_color.light2_5;
            colors.color_button_back = theApp.m_app_setting_data.theme_color.light2_5;
            colors.color_button_hover = theApp.m_app_setting_data.theme_color.light2;
            colors.color_button_pressed = theApp.m_app_setting_data.theme_color.light1_5;
        }
        colors.color_list_selected = theApp.m_app_setting_data.theme_color.light2;
        colors.color_statusbar_progress_back = theApp.m_app_setting_data.theme_color.light2_5;
    }

    return colors;
}

bool CPlayerUIHelper::IsMidiLyric()
{
    return CPlayer::GetInstance().IsMidi() && theApp.m_play_setting_data.midi_use_inner_lyric && !CPlayer::GetInstance().MidiNoLyric();
}

bool CPlayerUIHelper::IsDrawStatusBar()
{
    return theApp.m_app_setting_data.always_show_statusbar || CPlayer::GetInstance().m_loading || (theApp.IsMeidaLibUpdating() && theApp.m_media_update_para.num_added > 0)
        /*|| CPlayer::GetInstance().GetABRepeatMode() != CPlayer::AM_NONE*/;

}

double CPlayerUIHelper::GetScrollTextPixel()
{
    //界面刷新频率越高，即界面刷新时间间隔越小，则每次滚动的像素值就要越小
    double pixel = static_cast<double>(theApp.m_app_setting_data.ui_refresh_interval) * 0.0125 + 0.2;
    pixel = static_cast<double>(theApp.GetDPI()) * pixel / 96;
    if (pixel < 0.1)
        pixel = 0.1;
    if (pixel > 0.5 && pixel < 1)
        pixel = 1;
    return pixel;
}
