// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

// wxwindows headers, see wxwindows samples for description
#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/fontdlg.h>
#include <wx/statline.h>
#include <algorithm>

#include "wx_mplot.h"
#include "wx_gui.h"
#include "wx_dlg.h"
#include "wx_pane.h"
#include "data.h"
#include "logic.h"
#include "sum.h"
#include "func.h"
#include "ui.h"

using namespace std;


enum {
    ID_plot_popup_za                = 25001, 
    ID_plot_popup_sum               = 25011,
    ID_plot_popup_groups                   ,
    ID_plot_popup_peak                     ,

    ID_plot_popup_c_background             ,
    ID_plot_popup_c_inactive_data          ,
    ID_plot_popup_c_sum                    ,
    ID_plot_popup_c_inv                    ,

    ID_plot_popup_axes                     ,
    ID_plot_popup_plabels                  ,

    ID_plot_popup_pt_size           = 25210,// and next 10 ,
    ID_peak_popup_info              = 25250,
    ID_peak_popup_del                      ,
    ID_peak_popup_guess                    ,

    ID_CAD_COLOR,
    ID_CAD_FONT,

    ID_CPL_FONT,
    ID_CPL_SHOW,
    ID_CPL_RADIO,
    ID_CPL_TEXT,
};


//===============================================================
//                MainPlot (plot with data and fitted curves) 
//===============================================================
BEGIN_EVENT_TABLE(MainPlot, FPlot)
    EVT_PAINT (           MainPlot::OnPaint)
    EVT_MOTION (          MainPlot::OnMouseMove)
    EVT_LEAVE_WINDOW (    MainPlot::OnLeaveWindow)
    EVT_LEFT_DOWN (       MainPlot::OnButtonDown)
    EVT_RIGHT_DOWN (      MainPlot::OnButtonDown)
    EVT_MIDDLE_DOWN (     MainPlot::OnButtonDown)
    EVT_LEFT_DCLICK (     MainPlot::OnLeftDClick)
    EVT_LEFT_UP   (       MainPlot::OnButtonUp)
    EVT_RIGHT_UP (        MainPlot::OnButtonUp)
    EVT_MIDDLE_UP (       MainPlot::OnButtonUp)
    EVT_KEY_DOWN   (      MainPlot::OnKeyDown)
    EVT_MENU (ID_plot_popup_za,     MainPlot::OnZoomAll)
    EVT_MENU_RANGE (ID_plot_popup_sum, ID_plot_popup_peak,  
                                    MainPlot::OnPopupShowXX)
    EVT_MENU_RANGE (ID_plot_popup_c_background, ID_plot_popup_c_sum, 
                                    MainPlot::OnPopupColor)
    EVT_MENU_RANGE (ID_plot_popup_pt_size, ID_plot_popup_pt_size + max_radius, 
                                    MainPlot::OnPopupRadius)
    EVT_MENU (ID_plot_popup_c_inv,  MainPlot::OnInvertColors)
    EVT_MENU (ID_plot_popup_axes,   MainPlot::OnConfigureAxes)
    EVT_MENU (ID_plot_popup_plabels,MainPlot::OnConfigurePLabels)
    EVT_MENU (ID_peak_popup_info,   MainPlot::OnPeakInfo)
    EVT_MENU (ID_peak_popup_del,    MainPlot::OnPeakDelete)
    EVT_MENU (ID_peak_popup_guess,  MainPlot::OnPeakGuess)
END_EVENT_TABLE()

MainPlot::MainPlot (wxWindow *parent, PlotShared &shar) 
    : FPlot (parent, shar), BgManager(shar),
      basic_mode(mmd_zoom), mode(mmd_zoom), 
      pressed_mouse_button(0), ctrl(false), over_peak(-1)
{ }

void MainPlot::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    frame->draw_crosshair(-1, -1); 
    wxPaintDC dc(this);
    if (backgroundBrush.Ok())
        dc.SetBackground (backgroundBrush);
    dc.Clear();
    Draw(dc);
    vert_line_following_cursor(mat_redraw);//draw, if necessary, vertical lines
    peak_draft(mat_redraw);
    move_peak(mat_redraw);
    frame->update_app_title();
}

fp y_of_data_for_draw_data(vector<Point>::const_iterator i, Sum const* /*sum*/)
{
    return i->y;
}

void MainPlot::draw_dataset(wxDC& dc, int n, bool set_pen)
{
    bool shadowed;
    int offset;
    bool r = frame->get_sidebar()->howto_plot_dataset(n, shadowed, offset);
    if (!r)
        return;
    wxColour col = get_data_color(n);
    if (shadowed) {
        wxColour const& bg_col = get_bg_color();
        col.Set((col.Red() + bg_col.Red())/2,
                (col.Green() + bg_col.Green())/2,
                (col.Blue() + bg_col.Blue())/2);
    }
    if (set_pen)
        draw_data(dc, y_of_data_for_draw_data, AL->get_data(n), 0, 
                  col, wxNullColour, offset);
    else
        draw_data(dc, y_of_data_for_draw_data, AL->get_data(n), 0, 
                  dc.GetPen().GetColour(), dc.GetPen().GetColour(), offset);
}

void MainPlot::Draw(wxDC &dc, bool monochrome)
{
    int focused_data = AL->get_active_ds_position();
    Sum const* sum = AL->get_sum(focused_data);

    set_scale();

    frame->draw_crosshair(-1, -1); //erase crosshair before redrawing plot

    prepare_peaktops(sum);

    if (monochrome) {
        dc.SetPen(*wxBLACK_PEN);
        dc.SetBrush(*wxBLACK_BRUSH);
    }
    //draw datasets
    for (int i = 0; i < AL->get_ds_count(); i++) {
        if (i != focused_data) {
            draw_dataset(dc, i, !monochrome);
        }
    }
    // focused dataset is drawed at the end (to be at the top)
    draw_dataset(dc, focused_data, !monochrome);

    if (xtics_visible)
        draw_xtics(dc, AL->view, !monochrome);
    if (ytics_visible)
        draw_ytics(dc, AL->view, !monochrome);

    if (peaks_visible)
        draw_peaks(dc, sum, !monochrome);
    if (groups_visible)
        draw_groups(dc, sum, !monochrome);
    if (sum_visible)
        draw_sum(dc, sum, !monochrome);
    if (x_axis_visible) 
        draw_x_axis(dc, !monochrome);
    if (y_axis_visible) 
        draw_y_axis(dc, !monochrome);

    if (visible_peaktops(mode) && !monochrome) 
        draw_peaktops(dc, sum); 
    if (mode == mmd_bg) {
        draw_background(dc); 
    }
    else {
        if (plabels_visible)
            draw_plabels(dc, sum, !monochrome);
    }
}


bool MainPlot::visible_peaktops(MouseModeEnum mode)
{
    return (mode == mmd_zoom || mode == mmd_add || mode == mmd_peak);
}

void MainPlot::draw_x_axis (wxDC& dc, bool set_pen)
{
    if (set_pen)
        dc.SetPen (xAxisPen);
    dc.DrawLine (0, y2Y(0), GetClientSize().GetWidth(), y2Y(0));
}

void MainPlot::draw_y_axis (wxDC& dc, bool set_pen)
{
    if (set_pen)
        dc.SetPen (xAxisPen);
    dc.DrawLine (x2X(0), 0, x2X(0), GetClientSize().GetHeight());
}

void MainPlot::draw_sum(wxDC& dc, Sum const* sum, bool set_pen)
{
    if (set_pen)
        dc.SetPen(sumPen);
    int n = GetClientSize().GetWidth();
    vector<fp> xx(n), yy(n);
    vector<int> YY(n);
    for (int i = 0; i < n; ++i) 
        xx[i] = X2x(i);
    sum->calculate_sum_value(xx, yy);
    for (int i = 0; i < n; ++i) 
        YY[i] = y2Y(yy[i]);
    for (int i = 1; i < n; i++) 
        dc.DrawLine (i-1, YY[i-1], i, YY[i]); 
}


//TODO draw groups
void MainPlot::draw_groups (wxDC& /*dc*/, Sum const*, bool)
{
}

void MainPlot::draw_peaks(wxDC& dc, Sum const* sum, bool set_pen)
{
    fp level = 0;
    vector<int> const& idx = sum->get_ff_idx();
    int n = GetClientSize().GetWidth();
    vector<fp> xx(n), yy(n);
    vector<int> YY(n);
    for (int i = 0; i < n; ++i) 
        xx[i] = X2x(i);
    for (int k = 0; k < size(idx); k++) {
        fill(yy.begin(), yy.end(), 0.);
        Function const* f = AL->get_function(idx[k]);
        int from=0, to=n-1;
        fp left, right;
        if (f->get_nonzero_range(level, left, right)) {
            from = max(from, x2X(left));
            to = min(to, x2X(right));
        }
        if (set_pen)
            dc.SetPen (peakPen[k % max_peak_pens]);
        f->calculate_value(xx, yy);
        for (int i = from; i <= to; ++i) 
            YY[i] = y2Y(yy[i]);
        for (int i = from+1; i <= to; i++) 
            dc.DrawLine (i-1, YY[i-1], i, YY[i]); 
    }
}

void MainPlot::draw_peaktops (wxDC& dc, Sum const* sum)
{
    dc.SetPen (xAxisPen);
    dc.SetBrush (*wxTRANSPARENT_BRUSH);
    for (vector<wxPoint>::const_iterator i = shared.peaktops.begin(); 
                                           i != shared.peaktops.end(); i++) {
        dc.DrawRectangle (i->x - 1, i->y - 1, 3, 3);
    }
    draw_peaktop_selection(dc, sum);
}

void MainPlot::draw_peaktop_selection (wxDC& dc, Sum const* sum)
{
    int n = frame->get_sidebar()->get_focused_func();
    if (n == -1)
        return;
    vector<int> const& idx = sum->get_ff_idx();
    vector<int>::const_iterator t = find(idx.begin(), idx.end(), n);
    if (t != idx.end()) {
        wxPoint const&p = shared.peaktops[t-idx.begin()];
        dc.SetLogicalFunction (wxINVERT);
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawCircle(p.x, p.y, 4);
    }
}

void MainPlot::draw_plabels (wxDC& dc, Sum const* sum, bool set_pen)
{
    prepare_peak_labels(sum); //TODO re-prepare only when peaks where changed
    dc.SetFont(plabelFont);
    vector<wxRect> previous;
    vector<int> const& idx = sum->get_ff_idx();
    for (int k = 0; k < size(idx); k++) {
        const wxPoint &peaktop = shared.peaktops[k];
        if (set_pen)
            dc.SetTextForeground(peakPen[k % max_peak_pens].GetColour());

        wxString label = s2wx(plabels[k]);
        wxCoord w, h;
        if (vertical_plabels) {
            dc.GetTextExtent (label, &h, &w); // w and h swapped
            h = 0; // Y correction is not needed
        }
        else
            dc.GetTextExtent (label, &w, &h);
        int X = peaktop.x - w/2;
        int Y = peaktop.y - h - 2;
        wxRect rect(X, Y, w, h);

        // eliminate labels overlap 
        // perhaps more sophisticated algorithm for automatic label placement
        // should be used
        const int mrg = 0; //margin around label, can be negative
        int counter = 0;
        vector<wxRect>::const_iterator i = previous.begin();
        while (i != previous.end() && counter < 10) {
            //if not intersection 
            if (i->x > rect.GetRight()+mrg || rect.x > i->GetRight()+mrg
                || i->y > rect.GetBottom()+mrg || rect.y > i->GetBottom()+mrg) 
                ++i;
            else { // intersects -- try upper rectangle
                rect.SetY(i->y - h - 2); 
                i = previous.begin(); //and check for intersections with all...
                ++counter;
            }
        }
        previous.push_back(rect);
        if (vertical_plabels)
            dc.DrawRotatedText(label, rect.x, rect.y, 90);
        else
            dc.DrawText(label, rect.x, rect.y);
    }
}


/*
static bool operator< (const wxPoint& a, const wxPoint& b) 
{ 
    return a.x != b.x ? a.x < b.x : a.y < b.y; 
}
*/

void MainPlot::prepare_peaktops(Sum const* sum)
{
    int H =  GetClientSize().GetHeight();
    int Y0 = y2Y(0);
    vector<int> const& idx = sum->get_ff_idx();
    int n = idx.size();
    shared.peaktops.resize(n);
    for (int k = 0; k < n; k++) {
        Function const *f = AL->get_function(idx[k]);
        fp x;
        int X, Y;
        if (f->has_center()) {
            x = f->center();
            X = x2X (x - sum->zero_shift(x));
        }
        else {
            X = k * 10;
            x = X2x(X);
            x += sum->zero_shift(x);
        }
        //FIXME: check if these zero_shift()'s above are needed
        Y = y2Y(f->calculate_value(x));
        if (Y < 0 || Y > H) 
            Y = Y0;
        shared.peaktops[k] = wxPoint(X, Y);
    }
}


void MainPlot::prepare_peak_labels(Sum const* sum)
{
    vector<int> const& idx = sum->get_ff_idx();
    plabels.resize(idx.size());
    for (int k = 0; k < size(idx); k++) {
        Function const *f = AL->get_function(idx[k]);
        string label = plabel_format;
        if (f->is_peak()) {
            string::size_type pos = 0; 
            while ((pos = label.find("<", pos)) != string::npos) {
                string::size_type right = label.find(">", pos+1); 
                if (right == string::npos)
                    break;
                string tag(label, pos+1, right-pos-1);
                if (tag == "area")
                    label.replace(pos, right-pos+1, S(f->area()));
                else if (tag == "height")
                    label.replace(pos, right-pos+1, S(f->height()));
                else if (tag == "center")
                    label.replace(pos, right-pos+1, S(f->center()));
                else if (tag == "fwhm")
                    label.replace(pos, right-pos+1, S(f->fwhm()));
                else if (tag == "ib")
                    label.replace(pos, right-pos+1, S(f->area()/f->height()));
                else if (tag == "name")
                    label.replace(pos, right-pos+1, f->name);
                else if (tag == "br")
                    label.replace(pos, right-pos+1, "\n");
                else
                    ++pos;
            }
            plabels[k] = label;
        }
        else
            plabels[k] = "";
    }
}


void MainPlot::draw_background(wxDC& dc, bool set_pen)
{
    if (set_pen)
        dc.SetPen (bg_pointsPen);
    dc.SetBrush (*wxTRANSPARENT_BRUSH);
    // bg line
    int X = -1, Y = -1;
    for (vector<t_xy>::const_iterator i=bgline.begin(); i != bgline.end(); i++){
        int X_ = X, Y_ = Y;
        X = x2X(i->x);
        Y = y2Y(i->y);
        if (X_ >= 0 && (X != X_ || Y != Y_)) 
            dc.DrawLine (X_, Y_, X, Y); 
    }
    // bg points (circles)
    for (bg_const_iterator i = bg.begin(); i != bg.end(); i++) {
        dc.DrawCircle(x2X(i->x), y2Y(i->y), 3);
    }
}

void MainPlot::read_settings(wxConfigBase *cf)
{
    cf->SetPath(wxT("/MainPlot/Colors"));
    backgroundBrush.SetColour (cfg_read_color(cf, wxT("bg"), 
                                                      wxColour(wxT("BLACK"))));
    for (int i = 0; i < max_data_pens; i++)
        dataColour[i] = cfg_read_color(cf, 
                                            wxString::Format(wxT("data/%i"), i),
                                               wxColour(0, 255, 0));
    //activeDataPen.SetStyle (wxSOLID);
    wxColour inactive_data_col = cfg_read_color(cf,wxT("inactive_data"),
                                                      wxColour (128, 128, 128));
    inactiveDataPen.SetColour (inactive_data_col);
    sumPen.SetColour(cfg_read_color (cf, wxT("sum"), wxColour(wxT("YELLOW"))));
    bg_pointsPen.SetColour(cfg_read_color(cf, wxT("BgPoints"), 
                                                   wxColour(wxT("RED"))));
    for (int i = 0; i < max_group_pens; i++)
        groupPen[i].SetColour(cfg_read_color(cf, 
                                         wxString::Format(wxT("group/%i"), i),
                                             wxColour(173, 216, 230)));
    for (int i = 0; i < max_peak_pens; i++)
        peakPen[i].SetColour(cfg_read_color(cf, 
                                            wxString::Format(wxT("peak/%i"), i),
                                            wxColour(255, 0, 0)));
                            
    cf->SetPath(wxT("/MainPlot/Visible"));
    peaks_visible = cfg_read_bool(cf, wxT("peaks"), true); 
    plabels_visible = cfg_read_bool(cf, wxT("plabels"), false); 
    groups_visible = cfg_read_bool(cf, wxT("groups"), false);  
    sum_visible = cfg_read_bool(cf, wxT("sum"), true);
    cf->SetPath(wxT("/MainPlot"));
    point_radius = cf->Read (wxT("point_radius"), 1);
    line_between_points = cfg_read_bool(cf,wxT("line_between_points"), false);
    plabelFont = cfg_read_font(cf, wxT("plabelFont"), *wxNORMAL_FONT);
    plabel_format = wx2s(cf->Read(wxT("plabel_format"), wxT("<area>")));
    vertical_plabels = cfg_read_bool(cf, wxT("vertical_plabels"), false);
    x_max_tics = cf->Read(wxT("xMaxTics"), 7);
    y_max_tics = cf->Read(wxT("yMaxTics"), 7);
    x_tic_size = cf->Read(wxT("xTicSize"), 4);
    y_tic_size = cf->Read(wxT("yTicSize"), 4);
    x_reversed = cfg_read_bool (cf, wxT("xReversed"), false); 
    FPlot::read_settings(cf);
    Refresh();
}

void MainPlot::save_settings(wxConfigBase *cf) const
{
    cf->SetPath(wxT("/MainPlot"));
    cf->Write (wxT("point_radius"), point_radius);
    cf->Write (wxT("line_between_points"), line_between_points);
    cfg_write_font (cf, wxT("plabelFont"), plabelFont);
    cf->Write(wxT("plabel_format"), s2wx(plabel_format));
    cf->Write (wxT("vertical_plabels"), vertical_plabels);
    cf->Write(wxT("xMaxTics"), x_max_tics);
    cf->Write(wxT("yMaxTics"), y_max_tics);
    cf->Write(wxT("xTicSize"), x_tic_size);
    cf->Write(wxT("yTicSize"), y_tic_size);
    cf->Write(wxT("xReversed"), x_reversed);

    cf->SetPath(wxT("/MainPlot/Colors"));
    cfg_write_color (cf, wxT("bg"), backgroundBrush.GetColour());
    for (int i = 0; i < max_data_pens; i++)
        cfg_write_color(cf, wxString::Format(wxT("data/%i"), i), dataColour[i]);
    cfg_write_color (cf, wxT("inactive_data"), inactiveDataPen.GetColour());
    cfg_write_color (cf, wxT("sum"), sumPen.GetColour());
    cfg_write_color (cf, wxT("BgPoints"), bg_pointsPen.GetColour());
    for (int i = 0; i < max_group_pens; i++)
        cfg_write_color(cf, wxString::Format(wxT("group/%i"), i), 
                        groupPen[i].GetColour());
    for (int i = 0; i < max_peak_pens; i++)
        cfg_write_color(cf, wxString::Format(wxT("peak/%i"), i), 
                        peakPen[i].GetColour());

    cf->SetPath(wxT("/MainPlot/Visible"));
    cf->Write (wxT("peaks"), peaks_visible);
    cf->Write (wxT("plabels"), plabels_visible);
    cf->Write (wxT("groups"), groups_visible);
    cf->Write (wxT("sum"), sum_visible);
    cf->SetPath(wxT("/MainPlot"));
    FPlot::save_settings(cf);
}

void MainPlot::OnLeaveWindow (wxMouseEvent& WXUNUSED(event))
{
    frame->set_status_text("", sbf_coord);
    frame->draw_crosshair(-1, -1);
}

void MainPlot::set_scale()
{
    View const &v = AL->view;
    fp x_abs_scale = GetClientSize().GetWidth() / (v.right - v.left);
    shared.xUserScale = x_reversed ? -x_abs_scale : x_abs_scale;
    shared.xLogicalOrigin = x_reversed ? v.right : v.left;
    int H = GetClientSize().GetHeight();
    fp h = v.top - v.bottom;
    fp label_width = xtics_visible && v.bottom <= 0 ? max(v.bottom*H/h + 12, 0.)
                                                    : 0;
    yUserScale = - (H - label_width) / h;
    yLogicalOrigin = v.top;
    shared.plot_y_scale = yUserScale;
}
 
void MainPlot::show_popup_menu (wxMouseEvent &event)
{
    wxMenu popup_menu; //("main plot menu");

    popup_menu.Append(ID_plot_popup_za, wxT("Zoom &All"));
    popup_menu.AppendSeparator();

    wxMenu *show_menu = new wxMenu;
    show_menu->AppendCheckItem (ID_plot_popup_sum, wxT("S&um"), wxT(""));
    show_menu->Check (ID_plot_popup_sum, sum_visible);
    //show_menu->AppendCheckItem (ID_plot_popup_groups, 
    //                            wxT("Grouped peaks"), wxT(""));
    //show_menu->Check (ID_plot_popup_groups, groups_visible);
    show_menu->AppendCheckItem (ID_plot_popup_peak, wxT("&Peaks"), wxT(""));
    show_menu->Check (ID_plot_popup_peak, peaks_visible);
    popup_menu.Append (wxNewId(), wxT("&Show"), show_menu);

    wxMenu *color_menu = new wxMenu;
    color_menu->Append (ID_plot_popup_c_background, wxT("&Background"));
    color_menu->Append (ID_plot_popup_c_inactive_data, wxT("&Inactive Data"));
    color_menu->Append (ID_plot_popup_c_sum, wxT("&Sum"));
    color_menu->AppendSeparator(); 
    color_menu->Append (ID_plot_popup_c_inv, wxT("&Invert colors")); 
    popup_menu.Append (wxNewId(), wxT("&Color"), color_menu);  

    wxMenu *size_menu = new wxMenu;
    size_menu->AppendCheckItem (ID_plot_popup_pt_size, wxT("&Line"), wxT(""));
    size_menu->Check (ID_plot_popup_pt_size, line_between_points);
    size_menu->AppendSeparator();
    for (int i = 1; i <= max_radius; i++) 
        size_menu->AppendRadioItem (ID_plot_popup_pt_size + i, 
                                    wxString::Format (wxT("&%d"), i), wxT(""));
    size_menu->Check (ID_plot_popup_pt_size + point_radius, true);
    popup_menu.Append (wxNewId(), wxT("Data point si&ze"), size_menu);

    popup_menu.Append (ID_plot_popup_axes, wxT("Configure &Axes..."));
    popup_menu.Append (ID_plot_popup_plabels, wxT("Configure Peak &Labels..."));

    PopupMenu (&popup_menu, event.GetX(), event.GetY());
}

void MainPlot::show_peak_menu (wxMouseEvent &event)
{
    if (over_peak == -1) return;
    wxMenu peak_menu; 
    peak_menu.Append(ID_peak_popup_info, wxT("Show &Info"));
    peak_menu.Append(ID_peak_popup_del, wxT("&Delete"));
    peak_menu.Append(ID_peak_popup_guess, wxT("&Guess parameters"));
    peak_menu.Enable(ID_peak_popup_guess, 
                     AL->get_function(over_peak)->is_peak());
    PopupMenu (&peak_menu, event.GetX(), event.GetY());
}

void MainPlot::PeakInfo()
{
    if (over_peak >= 0)
        exec_command("info+ " + AL->get_function(over_peak)->xname);
}

void MainPlot::OnPeakDelete(wxCommandEvent &WXUNUSED(event))
{
    if (over_peak >= 0)
        exec_command("delete " + AL->get_function(over_peak)->xname);
}

void MainPlot::OnPeakGuess(wxCommandEvent &WXUNUSED(event))
{
    if (over_peak >= 0) {
        Function const* p = AL->get_function(over_peak);
        if (p->is_peak()) {
            fp ctr = p->center();
            fp plusmin = max(fabs(p->fwhm()), p->area() / p->height());    
            exec_command(p->xname + " = guess " + p->type_name + " [" 
                             + S(ctr-plusmin) + ":" + S(ctr+plusmin) + "]"
                             + frame->get_in_dataset());
        }
    }
}

bool MainPlot::has_mod_keys(const wxMouseEvent& event)
{
    return (event.AltDown() || event.ControlDown() || event.ShiftDown());
}


// mouse usage
//
//  Simple Rules:
//   1. When one button is down, pressing other button cancels action 
//   2. Releasing / keeping down / pressing Ctrl/Alt/Shift keys, when you keep 
//     mouse button down makes no difference.
//   3. Ctrl, Alt and Shift buttons are equivalent. 
//  ----------------------------------
//  Usage:
//   Left/Right Button (no Ctrl)   -- mode dependent  
//   Ctrl + Left/Right Button      -- the same as Left/Right in normal mode 
//   Middle Button                 -- rectangle zoom 

void MainPlot::set_mouse_mode(MouseModeEnum m)
{
    if (pressed_mouse_button) cancel_mouse_press();
    MouseModeEnum old = mode;
    if (m != mmd_peak) basic_mode = m;
    mode = m;
    update_mouse_hints();
    if (old != mode && (old == mmd_bg || mode == mmd_bg 
                        || visible_peaktops(old) != visible_peaktops(mode)))
        Refresh(false);
}

void MainPlot::update_mouse_hints()
    // update mouse hint on status bar and cursor
{
    string left="", right="";
    switch (pressed_mouse_button) {
        case 1:
            left = "";       right = "cancel";
            break;
        case 2:
            left = "cancel"; right = "cancel";
            break;
        case 3:
            left = "cancel"; right = "";
            break;
        default:
            //button not pressed
            switch (mode) {
                case mmd_peak:
                    left = "move peak"; right = "peak menu";
                    SetCursor (wxCURSOR_CROSS);
                    break;
                case mmd_zoom: 
                    left = "rect zoom"; right = "plot menu";
                    SetCursor (wxCURSOR_ARROW);
                    break;
                case mmd_bg: 
                    left = "add point"; right = "del point";
                    SetCursor (wxCURSOR_ARROW);
                    break;
                case mmd_add: 
                    left = "draw-add";  right = "add-in-range";
                    SetCursor (wxCURSOR_ARROW);
                    break;
                case mmd_range: 
                    left = "activate";  right = "disactivate";
                    SetCursor (wxCURSOR_ARROW);
                    break;
                default: 
                    assert(0);
            }
    }
    frame->set_status_hint(left, right);
}

void MainPlot::OnMouseMove(wxMouseEvent &event)
{
    //display coords in status bar 
    int X = event.GetX();
    int Y = event.GetY();
    fp x = X2x(X);
    fp y = Y2y(Y);
    wxString str;
    str.Printf(wxT("%.3f  %d"), x, static_cast<int>(y + 0.5));
    frame->set_status_text(wx2s(str), sbf_coord);

    if (pressed_mouse_button == 0) {
        if (mode == mmd_range) {
            if (!ctrl)
                vert_line_following_cursor (mat_move, event.GetX());
            else
                vert_line_following_cursor (mat_cancel);
        }
        if (visible_peaktops(mode)) 
            look_for_peaktop (event);
        frame->draw_crosshair(X, Y);
    }
    else {
        vert_line_following_cursor (mat_move, event.GetX());
        move_peak(mat_move, event);
        peak_draft (mat_move, event);
        rect_zoom (mat_move, event);
    }
}

void MainPlot::look_for_peaktop (wxMouseEvent& event)
{
    // searching the closest peak-top and distance from it, d = dx + dy < 10 
    int focused_data = AL->get_active_ds_position();
    Sum const* sum = AL->get_sum(focused_data);
    vector<int> const& idx = sum->get_ff_idx();
    int min_dist = 10;
    int nearest = -1;
    if (shared.peaktops.size() != idx.size()) 
        Refresh(false);
    for (vector<wxPoint>::const_iterator i = shared.peaktops.begin(); 
         i != shared.peaktops.end(); i++) {
        int d = abs(event.GetX() - i->x) + abs(event.GetY() - i->y);
        if (d < min_dist) {
            min_dist = d;
            nearest = idx[i - shared.peaktops.begin()];
        }
    }
    if (over_peak == nearest) return;

    //if we are here, over_peak != nearest; changing cursor and statusbar text
    over_peak = nearest;
    if (nearest != -1) {
        Function const* f = AL->get_function(over_peak);
        string s = f->xname + " " + f->type_name + " ";
        vector<string> const& vn = f->type_var_names;
        for (int i = 0; i < size(vn); ++i)
            s += " " + vn[i] + "=" + S(f->get_var_value(i));
        frame->set_status_text(s);
        set_mouse_mode(mmd_peak);
    }
    else { //was over peak, but now is not 
        frame->set_status_text("");
        set_mouse_mode(basic_mode);
    }
}

void MainPlot::cancel_mouse_press()
{
    if (pressed_mouse_button) {
        rect_zoom (mat_cancel); 
        move_peak(mat_cancel);
        peak_draft (mat_cancel);
        vert_line_following_cursor (mat_cancel);
        mouse_press_X = mouse_press_Y = INVALID;
        pressed_mouse_button = 0;
        frame->set_status_text("");
        update_mouse_hints();
    }
}


void MainPlot::OnButtonDown (wxMouseEvent &event)
{
    if (pressed_mouse_button) {
        cancel_mouse_press();
        return;
    }

    frame->draw_crosshair(-1, -1);
    int button = event.GetButton();
    pressed_mouse_button = button;
    ctrl = has_mod_keys(event);
    mouse_press_X = event.GetX();
    mouse_press_Y = event.GetY();
    fp x = X2x (event.GetX());
    fp y = Y2y (event.GetY());
    if (button == 1 && (ctrl || mode == mmd_zoom) || button == 2) {
        rect_zoom (mat_start, event);
        SetCursor(wxCURSOR_MAGNIFIER);  
        frame->set_status_text("Select second corner to zoom...");
    }
    else if (button == 3 && (ctrl || mode == mmd_zoom)) {
        show_popup_menu (event);
        cancel_mouse_press();
    }
    else if (button == 1 && mode == mmd_peak) {
        frame->activate_function(over_peak);
        move_peak(mat_start, event);
        frame->set_status_text("Moving " + AL->get_function(over_peak)->xname 
                                + "...");
    }
    else if (button == 3 && mode == mmd_peak) {
        show_peak_menu(event);
        cancel_mouse_press();
    }
    else if (button == 1 && mode == mmd_bg) {
        add_background_point(x, y);
        Refresh(false);
    }
    else if (button == 3 && mode == mmd_bg) {
        rm_background_point(x);
        Refresh(false);
    }
    else if (button == 1 && mode == mmd_add) {
        peak_draft (mat_start, event);
        SetCursor(wxCURSOR_SIZING);
        frame->set_status_text("Add drawed peak...");
    }
    else if (button == 3 && mode == mmd_add) {
        vert_line_following_cursor(mat_start, mouse_press_X+1, mouse_press_X);
        SetCursor(wxCURSOR_SIZEWE);
        frame->set_status_text("Select range to add a peak in it..."); 
    }
    else if (button != 2 && mode == mmd_range) {
        vert_line_following_cursor(mat_start, mouse_press_X+1, mouse_press_X);
        SetCursor(wxCURSOR_SIZEWE);
        frame->set_status_text(button==1 ? "Select data range to activate..."
                                       : "Select data range to disactivate...");
    }
    update_mouse_hints();
}


void MainPlot::OnButtonUp (wxMouseEvent &event)
{
    int button = event.GetButton();
    if (button != pressed_mouse_button) {
        pressed_mouse_button = 0;
        return;
    }
    int dist_x = abs(event.GetX() - mouse_press_X);  
    int dist_y = abs(event.GetY() - mouse_press_Y);  
    // if Down and Up events are at the same position -> cancel
    if (button == 1 && (ctrl || mode == mmd_zoom) || button == 2) {
        rect_zoom(dist_x + dist_y < 10 ? mat_cancel: mat_stop, event);
        frame->set_status_text("");
    }
    else if (mode == mmd_peak && button == 1) {
        move_peak(dist_x + dist_y < 2 ? mat_cancel: mat_stop, event);
        frame->set_status_text("");
    }
    else if (mode == mmd_range && button != 2) {
        vert_line_following_cursor(mat_cancel);
        if (dist_x >= 5) { 
            fp xmin = X2x (min (event.GetX(), mouse_press_X));
            fp xmax = X2x (max (event.GetX(), mouse_press_X));
            string xmin_x_xmax = "(" + S(xmin) + "< x <" + S(xmax) + ")";
            string c = (button == 1 ? "A = a or " : "A = a and not ");
            exec_command(c + xmin_x_xmax + frame->get_in_one_or_all_datasets());
        }
        frame->set_status_text("");
    }
    else if (mode == mmd_add && button == 1) {
        frame->set_status_text("");
        peak_draft (dist_x + dist_y < 5 ? mat_cancel: mat_stop, event);
    }
    else if (mode == mmd_add && button == 3) {
        frame->set_status_text("");
        if (dist_x >= 5) { 
            fp x1 = X2x(mouse_press_X);
            fp x2 = X2x(event.GetX());
            exec_command ("guess " + frame->get_peak_type() 
                          + " [" + S(min(x1,x2)) + " : " + S(max(x1,x2)) + "]"
                          + frame->get_in_dataset());
        }
        vert_line_following_cursor(mat_cancel);
    }
    else {
        ;// nothing - action done in OnButtonDown()
    }
    pressed_mouse_button = 0;
    update_mouse_hints();
}


void MainPlot::OnKeyDown (wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_ESCAPE) {
        cancel_mouse_press(); 
    }
    else if (should_focus_input(event.GetKeyCode())) {
        cancel_mouse_press(); 
        frame->focus_input(event.GetKeyCode());
    }
    else
        event.Skip();
}


void MainPlot::move_peak (Mouse_act_enum ma, wxMouseEvent &event)
{
    static wxPoint prev;
    static int func_nr = -1;
    static vector<fp> p_values(4,0);
    static vector<bool> changable(4,false);
    static wxCursor old_cursor = wxNullCursor;

    if (over_peak == -1)
        return;
    Function const* p = AL->get_function(over_peak);

    if (ma != mat_start) {
        if (func_nr != over_peak) 
            return;
        draw_xor_peak(p, p_values); //clear old or redraw
    }

    if (ma == mat_redraw)
        ; //do nothing, already redrawn
    else if (ma == mat_start) {
        func_nr = over_peak;
        for (int i = 0; i < size(p_values); ++i) {
            if (i < p->get_vars_count()) {
                p_values[i] = p->get_var_value(i);
                Variable const* var = AL->get_variable(p->get_var_idx(i));
                changable[i] = var->is_simple();
            }
            else {
                changable[i] = false;
            }
        }
        draw_xor_peak(p, p_values); 
        prev.x = event.GetX(), prev.y = event.GetY();
        old_cursor = GetCursor();
        SetCursor(wxCURSOR_SIZENWSE);
    }
    else if (ma == mat_move) {
        bool mod_key =  has_mod_keys(event);
        string descr = mod_key ? "[shift]" : "(without [shift])";
        int n = mod_key ? 2 : 0;
        if (changable[n]) {
            fp dy = Y2y(event.GetY()) - Y2y(prev.y);
            p_values[n] += dy;
            descr += " Y:" + p->type_var_names[n] + "=" + S(p_values[n]);
        }
        if (changable[n+1]) {
            fp dx = X2x(event.GetX()) - X2x(prev.x);
            p_values[n+1] += dx;
            descr += " X:" + p->type_var_names[n+1] + "=" + S(p_values[n+1]);
        }

        frame->set_status_text(descr);
        prev.x = event.GetX(), prev.y = event.GetY();
        draw_xor_peak(p, p_values); 
    }
    else if (ma == mat_stop || ma == mat_cancel) {
        func_nr = -1;
        if (old_cursor.Ok()) {
            SetCursor(old_cursor);
            old_cursor = wxNullCursor;
        }
        if (ma == mat_stop) {
            vector<string> cmd;
            for (int i = 0; i < size(p_values); ++i) {
                if (changable[i] && p_values[i] != p->get_var_value(i))
                    cmd.push_back("$"+p->get_var_name(i)+"=~"+S(p_values[i]));
            }
            if (!cmd.empty())
                exec_command(join_vector(cmd, "; "));
        }
    }
}


void MainPlot::draw_xor_peak(Function const* func, vector<fp> const& p_values)
{
    wxClientDC dc(this);
    dc.SetLogicalFunction (wxINVERT);
    dc.SetPen(*wxBLACK_DASHED_PEN);

    int n = GetClientSize().GetWidth();
    if (n <= 0) 
        return;
    vector<fp> xx(n), yy(n, 0);
    for (int i = 0; i < n; ++i) 
        xx[i] = X2x(i);
    func->calculate_values_with_params(xx, yy, p_values);
    vector<int> YY(n);
    for (int i = 0; i < n; ++i) 
        YY[i] = y2Y(yy[i]);
    for (int i = 1; i < n; i++) 
        dc.DrawLine (i-1, YY[i-1], i, YY[i]); 
}

void MainPlot::peak_draft (Mouse_act_enum ma, wxMouseEvent &event)
{
    static wxPoint prev(INVALID, INVALID);
    if (ma != mat_start) {
        if (prev.x == INVALID) 
            return;
        //clear/redraw old peak-draft
        draw_peak_draft(mouse_press_X, abs(mouse_press_X - prev.x), prev.y);
    }
    switch (ma) {
        case mat_start:
        case mat_move:
            prev.x = event.GetX(), prev.y = event.GetY();
            draw_peak_draft(mouse_press_X, abs(mouse_press_X-prev.x), prev.y);
            break;
        case mat_stop: 
          {
            prev.x = prev.y = INVALID;
            fp height = Y2y(event.GetY());
            fp center = X2x(mouse_press_X);
            fp fwhm = fabs(shared.dX2dx(mouse_press_X - event.GetX()));
            fp area = height * 2*fwhm;
            string F = AL->get_ds_count() > 1 
                                      ?  frame->get_active_data_str()+".F" 
                                      : "F";
            exec_command(frame->get_peak_type()  
                         + "(height=~" + S(height) + ", center=~" + S(center) 
                         + ", fwhm=~" + S(fwhm) + ", area=~" + S(area) 
                         + ") -> " + F);
          }
          break;
        case mat_cancel:
            prev.x = prev.y = INVALID;
            break;
        case mat_redraw: //already redrawn
            break;
        default: assert(0);
    }
}

void MainPlot::draw_peak_draft(int Ctr, int Hwhm, int Y)
{
    if (Ctr == INVALID || Hwhm == INVALID || Y == INVALID)
        return;
    wxClientDC dc(this);
    dc.SetLogicalFunction (wxINVERT);
    dc.SetPen(*wxBLACK_DASHED_PEN);
    int Y0 = y2Y(0);
    dc.DrawLine (Ctr, Y0, Ctr, Y); //vertical line
    dc.DrawLine (Ctr - Hwhm, (Y+Y0)/2, Ctr + Hwhm, (Y+Y0)/2); //horizontal line
    dc.DrawLine (Ctr, Y, Ctr - 2 * Hwhm, Y0); //left slope
    dc.DrawLine (Ctr, Y, Ctr + 2 * Hwhm, Y0); //right slope
}

bool MainPlot::rect_zoom (Mouse_act_enum ma, wxMouseEvent &event) 
{
    static int X1 = INVALID, Y1 = INVALID, X2 = INVALID, Y2 = INVALID;

    if (ma == mat_start) {
        X1 = X2 = event.GetX(), Y1 = Y2 = event.GetY(); 
        draw_rect (X1, Y1, X2, Y2);
        CaptureMouse();
        return true;
    }
    else {
        if (X1 == INVALID || Y1 == INVALID) return false;
        draw_rect (X1, Y1, X2, Y2); //clear old rectangle
    }

    if (ma == mat_move) {
        X2 = event.GetX(), Y2 = event.GetY(); 
        draw_rect (X1, Y1, X2, Y2);
    }
    else if (ma == mat_stop) {
        X2 = event.GetX(), Y2 = event.GetY(); 
        fp x1 = X2x(X1);
        fp x2 = X2x(X2);
        fp y1 = Y2y(Y1);
        fp y2 = Y2y(Y2);
        if (abs(X1-X2) > 5 && abs(Y1-Y2) > 5) 
            frame->change_zoom("[ "+S(min(x1,x2))+" : "+S(max(x1,x2))+" ]"
                               "[ "+S(min(y1,y2))+" : "+S(max(y1,y2))+" ]"); 
    }

    if (ma == mat_cancel || ma == mat_stop) {
        X1 = Y1 = X2 = Y2 = INVALID;
        ReleaseMouse();
    }
    return true;
}

void MainPlot::draw_rect (int X1, int Y1, int X2, int Y2)
{
    if (X1 == INVALID || Y1 == INVALID || X2 == INVALID || Y2 == INVALID) 
        return;
    wxClientDC dc(this);
    dc.SetLogicalFunction (wxINVERT);
    dc.SetPen(*wxBLACK_DASHED_PEN);
    dc.SetBrush (*wxTRANSPARENT_BRUSH);
    int xmin = min (X1, X2);
    int width = max (X1, X2) - xmin;
    int ymin = min (Y1, Y2);
    int height = max (Y1, Y2) - ymin;
    dc.DrawRectangle (xmin, ymin, width, height);
}

void MainPlot::OnPopupShowXX (wxCommandEvent& event)
{
    switch (event.GetId()) {
        case ID_plot_popup_sum  :  sum_visible = !sum_visible;       break; 
        case ID_plot_popup_groups: groups_visible = !groups_visible; break; 
        case ID_plot_popup_peak :  peaks_visible = !peaks_visible;   break;  
        default: assert(0);
    }
    Refresh(false);
}

void MainPlot::OnPopupColor(wxCommandEvent& event)
{
    int n = event.GetId();
    wxBrush *brush = 0;
    wxPen *pen = 0;
    if (n == ID_plot_popup_c_background)
        brush = &backgroundBrush;
    else if (n == ID_plot_popup_c_inactive_data) {
        pen = &inactiveDataPen;
    }
    else if (n == ID_plot_popup_c_sum)
        pen = &sumPen;
    else 
        return;
    wxColour col = brush ? brush->GetColour() : pen->GetColour();
    if (change_color_dlg(col)) {
        if (brush) 
            brush->SetColour(col);
        if (pen) 
            pen->SetColour(col);
        if (n == ID_plot_popup_c_background)
            frame->update_data_pane();
        Refresh();
    }
}

void MainPlot::OnInvertColors (wxCommandEvent& WXUNUSED(event))
{
    backgroundBrush.SetColour (invert_colour (backgroundBrush.GetColour()));
    for (int i = 0; i < max_data_pens; i++)
        dataColour[i] = invert_colour(dataColour[i]);
    inactiveDataPen.SetColour (invert_colour(inactiveDataPen.GetColour()));
    sumPen.SetColour (invert_colour (sumPen.GetColour()));  
    xAxisPen.SetColour (invert_colour (xAxisPen.GetColour()));  
    for (int i = 0; i < max_group_pens; i++)
        groupPen[i].SetColour (invert_colour (groupPen[i].GetColour()));
    for (int i = 0; i < max_peak_pens; i++)
        peakPen[i].SetColour (invert_colour (peakPen[i].GetColour()));
    frame->update_data_pane();
    Refresh();
}

void MainPlot::OnPopupRadius (wxCommandEvent& event)
{
    int nr = event.GetId() - ID_plot_popup_pt_size;
    if (nr == 0)
        line_between_points = !line_between_points;
    else
        point_radius = nr;
    Refresh(false);
}

void MainPlot::OnConfigureAxes (wxCommandEvent& WXUNUSED(event))
{
    ConfigureAxesDlg dialog(frame, -1, this);
    dialog.ShowModal();
}

void MainPlot::OnConfigurePLabels (wxCommandEvent& WXUNUSED(event))
{
    ConfigurePLabelsDlg dialog(frame, -1, this);
    dialog.ShowModal();
}

void MainPlot::OnZoomAll (wxCommandEvent& WXUNUSED(event))
{
    frame->OnGViewAll(dummy_cmd_event);
}

//===============================================================
//                     ConfigureAxesDlg
//===============================================================

BEGIN_EVENT_TABLE(ConfigureAxesDlg, wxDialog)
    EVT_BUTTON(wxID_APPLY, ConfigureAxesDlg::OnApply)
    EVT_BUTTON(wxID_CLOSE, ConfigureAxesDlg::OnClose)
    EVT_BUTTON(ID_CAD_COLOR, ConfigureAxesDlg::OnChangeColor)
    EVT_BUTTON(ID_CAD_FONT, ConfigureAxesDlg::OnChangeFont)
END_EVENT_TABLE()

ConfigureAxesDlg::ConfigureAxesDlg(wxWindow* parent, wxWindowID id, 
                                   MainPlot* plot_)
    //explicit conversion of title to wxString() is neccessary
  : wxDialog(parent, id, wxString(wxT("Configure Axes"))), plot(plot_),
    axis_color(plot_->xAxisPen.GetColour())
{
    wxBoxSizer *top_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticBoxSizer *xsizer = new wxStaticBoxSizer(wxVERTICAL, this, 
                                                    wxT("X axis"));
    x_show_axis = new wxCheckBox(this, -1, wxT("show axis"));
    x_show_axis->SetValue(plot->x_axis_visible);
    xsizer->Add(x_show_axis, 0, wxALL, 5);
    x_show_tics = new wxCheckBox(this, -1, wxT("show tics"));
    x_show_tics->SetValue(plot->xtics_visible);
    xsizer->Add(x_show_tics, 0, wxALL, 5);
    wxBoxSizer *xmt_sizer = new wxBoxSizer(wxHORIZONTAL);
    xmt_sizer->Add(new wxStaticText(this, -1, wxT("max. number of tics")), 
                  0, wxALL|wxALIGN_CENTRE_VERTICAL, 5);
    x_max_tics = new wxSpinCtrl (this, -1, wxT("7"),
                                 wxDefaultPosition, wxSize(50, -1), 
                                 wxSP_ARROW_KEYS, 1, 30, 7);
    x_max_tics->SetValue(plot->x_max_tics);
    xmt_sizer->Add(x_max_tics, 0, wxALL, 5);
    xsizer->Add(xmt_sizer);
    wxBoxSizer *xts_sizer = new wxBoxSizer(wxHORIZONTAL);
    xts_sizer->Add(new wxStaticText(this, -1, wxT("length of tics")), 
                  0, wxALL|wxALIGN_CENTRE_VERTICAL, 5);
    x_tics_size = new wxSpinCtrl (this, -1, wxT("4"),
                                  wxDefaultPosition, wxSize(50, -1), 
                                  wxSP_ARROW_KEYS, -10, 20, 4);
    x_tics_size->SetValue(plot->x_tic_size);
    xts_sizer->Add(x_tics_size, 0, wxALL, 5);
    xsizer->Add(xts_sizer);
    x_reversed = new wxCheckBox(this, -1, wxT("reversed axis"));
    x_reversed->SetValue(plot->x_reversed);
    xsizer->Add(x_reversed, 0, wxALL, 5);
    sizer1->Add(xsizer, 0, wxALL, 5);

    wxStaticBoxSizer *ysizer = new wxStaticBoxSizer(wxVERTICAL, this, 
                                                    wxT("Y axis"));
    y_show_axis = new wxCheckBox(this, -1, wxT("show axis"));
    y_show_axis->SetValue(plot->y_axis_visible);
    ysizer->Add(y_show_axis, 0, wxALL, 5);
    y_show_tics = new wxCheckBox(this, -1, wxT("show tics"));
    y_show_tics->SetValue(plot->ytics_visible);
    ysizer->Add(y_show_tics, 0, wxALL, 5);
    wxBoxSizer *ymt_sizer = new wxBoxSizer(wxHORIZONTAL);
    ymt_sizer->Add(new wxStaticText(this, -1, wxT("max. number of tics")), 
                  0, wxALL|wxALIGN_CENTRE_VERTICAL, 5);
    y_max_tics = new wxSpinCtrl (this, -1, wxT("7"),
                                 wxDefaultPosition, wxSize(50, -1), 
                                 wxSP_ARROW_KEYS, 1, 30, 7);
    y_max_tics->SetValue(plot->y_max_tics);
    ymt_sizer->Add(y_max_tics, 0, wxALL, 5);
    ysizer->Add(ymt_sizer);
    wxBoxSizer *yts_sizer = new wxBoxSizer(wxHORIZONTAL);
    yts_sizer->Add(new wxStaticText(this, -1, wxT("length of tics")), 
                  0, wxALL|wxALIGN_CENTRE_VERTICAL, 5);
    y_tics_size = new wxSpinCtrl (this, -1, wxT("4"),
                                  wxDefaultPosition, wxSize(50, -1), 
                                  wxSP_ARROW_KEYS, 1, 20, 4);
    y_tics_size->SetValue(plot->y_tic_size);
    yts_sizer->Add(y_tics_size, 0, wxALL, 5);
    ysizer->Add(yts_sizer);
    //TODO y_reversed = new wxCheckBox(this, -1, wxT("reversed axis"));
    //y_reversed->SetValue(plot->y_reversed);
    //ysizer->Add(y_reversed, 0, wxALL, 5);
    sizer1->Add(ysizer, 0, wxALL, 5);

    top_sizer->Add(sizer1, 0);
    wxBoxSizer *common_sizer = new wxBoxSizer(wxHORIZONTAL);
    common_sizer->Add(new wxButton(this, ID_CAD_COLOR, 
                                   wxT("Change axes color...")),
                      0, wxALL, 5);
    common_sizer->Add(new wxButton(this, ID_CAD_FONT, 
                                   wxT("Change tics font...")),
                      0, wxALL, 5);
    top_sizer->Add(common_sizer, 0, wxALIGN_CENTER);
    top_sizer->Add (new wxStaticLine(this, -1), 0, wxEXPAND|wxLEFT|wxRIGHT, 5);

    wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
    button_sizer->Add(new wxButton(this, wxID_APPLY, wxT("&Apply")), 
                      0, wxALL, 5);
    button_sizer->Add(new wxButton(this, wxID_CLOSE, wxT("&Close")), 
                      0, wxALL, 5);
    top_sizer->Add(button_sizer, 0, wxALL|wxALIGN_CENTER, 0);
    SetSizerAndFit(top_sizer);
}

void ConfigureAxesDlg::OnApply (wxCommandEvent& WXUNUSED(event))
{
    bool scale_changed = false;
    plot->xAxisPen.SetColour(axis_color);
    plot->x_axis_visible = x_show_axis->GetValue();
    plot->xtics_visible = x_show_tics->GetValue();
    plot->x_max_tics = x_max_tics->GetValue();
    plot->x_tic_size = x_tics_size->GetValue();
    if (plot->x_reversed != x_reversed->GetValue()) {
        plot->x_reversed = x_reversed->GetValue();
        scale_changed = true;
    }
    plot->y_axis_visible = y_show_axis->GetValue();
    plot->ytics_visible = y_show_tics->GetValue();
    plot->y_max_tics = y_max_tics->GetValue();
    plot->y_tic_size = y_tics_size->GetValue();
    //TODO plot->y_reversed = y_reversed->GetValue();
    frame->refresh_plots(true, false, !scale_changed);
}

void ConfigureAxesDlg::OnChangeFont (wxCommandEvent& WXUNUSED(event)) 
{ 
    plot->change_tics_font(); 
}

//===============================================================
//                     ConfigurePLabelsDlg
//===============================================================

BEGIN_EVENT_TABLE(ConfigurePLabelsDlg, wxDialog)
    EVT_BUTTON(wxID_APPLY, ConfigurePLabelsDlg::OnApply)
    EVT_BUTTON(wxID_CLOSE, ConfigurePLabelsDlg::OnClose)
    EVT_BUTTON(ID_CPL_FONT, ConfigurePLabelsDlg::OnChangeLabelFont)
    EVT_CHECKBOX(ID_CPL_SHOW, ConfigurePLabelsDlg::OnCheckShowLabel)
    EVT_TEXT(ID_CPL_TEXT, ConfigurePLabelsDlg::OnChangeLabelText)
    EVT_RADIOBOX(ID_CPL_RADIO, ConfigurePLabelsDlg::OnRadioLabel)
END_EVENT_TABLE()

ConfigurePLabelsDlg::ConfigurePLabelsDlg(wxWindow* parent, wxWindowID id, 
                                         MainPlot* plot_)
    //explicit conversion of title to wxString() is neccessary
  : wxDialog(parent, id, wxString(wxT("Configure Peak Labels"))), plot(plot_),
    in_onradiolabel(false)
{
    wxBoxSizer *top_sizer = new wxBoxSizer(wxVERTICAL);
    show_plabels = new wxCheckBox(this, ID_CPL_SHOW, wxT("show peak labels"));
    top_sizer->Add(show_plabels, 0, wxALL, 5);
    wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
    vector<string> label_radio_choice;
    label_radio_choice.push_back("area");
    label_radio_choice.push_back("height");
    label_radio_choice.push_back("center");
    label_radio_choice.push_back("fwhm");
    label_radio_choice.push_back("name");
    label_radio_choice.push_back("custom");
    label_radio = new wxRadioBox(this, ID_CPL_RADIO, wxT("labels with:"),
                                 wxDefaultPosition, wxDefaultSize, 
                                 stl2wxArrayString(label_radio_choice),
                                 1, wxRA_SPECIFY_COLS);
    sizer1->Add(label_radio, 0, wxALL|wxEXPAND, 5);
    wxStaticBoxSizer *xsizer = new wxStaticBoxSizer(wxVERTICAL, this, 
                                            wxT("list of replaceable tokens"));
    xsizer->Add(new wxStaticText(this, -1, wxT("<area>   area of peak\n")
                                           wxT("<height> height of peak\n")
                                           wxT("<center> center of peak\n")
                                           wxT("<fwhm>   FWHM of peak\n")
                                       wxT("<ib>    integral breadth of peak\n")
                                           wxT("<name>   name of function\n")
                                           wxT("<br>     line break\n")),
                0, wxALL|wxEXPAND, 5);
    sizer1->Add(xsizer, 0, wxALL, 5);
    top_sizer->Add(sizer1, 0);
    label_text = new wxTextCtrl(this, ID_CPL_TEXT, wxT(""));
    top_sizer->Add(label_text, 0, wxALL|wxEXPAND, 5);

    vertical_rb = new wxRadioBox(this, ID_CPL_RADIO, wxT("label direction"),
                                 wxDefaultPosition, wxDefaultSize, 
                                 stl2wxArrayString(vector2(string("horizontal"),
                                                           string("vertical"))),
                                 2);
    top_sizer->Add(vertical_rb, 0, wxALL|wxEXPAND, 5);

    top_sizer->Add(new wxButton(this, ID_CPL_FONT, wxT("Change label font...")),
                   0, wxALL|wxALIGN_CENTER, 5);
    top_sizer->Add(new wxStaticText(this, -1, 
                                   wxT("Labels have the same colors as peaks")),
                   0, wxALL, 5);

    top_sizer->Add(new wxStaticLine(this, -1), 0, wxEXPAND|wxLEFT|wxRIGHT, 5);

    wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
    button_sizer->Add(new wxButton(this, wxID_APPLY, wxT("&Apply")), 
                      0, wxALL, 5);
    button_sizer->Add(new wxButton(this, wxID_CLOSE, wxT("&Close")), 
                      0, wxALL, 5);
    top_sizer->Add(button_sizer, 0, wxALL|wxALIGN_CENTER, 0);
    SetSizerAndFit(top_sizer);

    //set initial values
    show_plabels->SetValue(plot->plabels_visible); 
    label_text->SetValue(s2wx(plot->plabel_format));
    vertical_rb->SetSelection(plot->vertical_plabels ? 1 : 0);
    label_radio->SetStringSelection(wxT("custom"));
    label_text->Enable(plot->plabels_visible);
    label_radio->Enable(plot->plabels_visible);
    vertical_rb->Enable(plot->plabels_visible);
}

void ConfigurePLabelsDlg::OnChangeLabelText (wxCommandEvent& WXUNUSED(event))
{
    // don't change radio if this event is triggered by label_text->SetValue()
    // from radiobox event handler
    if (!in_onradiolabel)
        label_radio->SetStringSelection(wxT("custom"));
}

void ConfigurePLabelsDlg::OnCheckShowLabel (wxCommandEvent& event)
{
    bool checked = event.IsChecked();
    label_radio->Enable(checked);
    label_text->Enable(checked);
    vertical_rb->Enable(checked);
}

void ConfigurePLabelsDlg::OnRadioLabel (wxCommandEvent& WXUNUSED(event))
{
    in_onradiolabel = true;
    wxString s = label_radio->GetStringSelection();
    if (s != wxT("custom"))
        label_text->SetValue(wxT("<") + s + wxT(">"));
    in_onradiolabel = false;
}

void ConfigurePLabelsDlg::OnApply (wxCommandEvent& WXUNUSED(event))
{
    plot->plabels_visible = show_plabels->GetValue(); 
    plot->plabel_format = wx2s(label_text->GetValue());
    plot->vertical_plabels = vertical_rb->GetSelection() != 0;
    frame->refresh_plots(true, false, true);
}

void ConfigurePLabelsDlg::OnChangeLabelFont (wxCommandEvent& WXUNUSED(event)) 
{ 
    wxFontData data;
    data.SetInitialFont(plot->plabelFont);
    wxFontDialog dialog(frame, &data);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxFontData retData = dialog.GetFontData();
        plot->plabelFont = retData.GetChosenFont();
        plot->Refresh(false);
    }
}



//===============================================================
//           BgManager (for interactive background setting)
//===============================================================

/*
// outdated
void auto_background (int n, fp p1, bool is_proc1, fp p2, bool is_proc2)
{
    //FIXME: Do you know any good algorithm, that can extract background
    //       from data?
    if (n <= 0 || n >= size(p) / 2)
        return;
    int ps = p.size();
    for (int i = 0; i < n; i++) {
        int l = ps * i / n;
        int u = ps * (i + 1) / n;
        vector<fp> v (u - l);
        for (int k = l; k < u; k++)
            v[k - l] = p[k].orig_y;
        sort (v.begin(), v.end());
        int y_avg_beg = 0, y_avg_end = v.size();
        if (is_proc1) {
            p1 = min (max (p1, 0.), 100.);
            y_avg_beg = static_cast<int>(v.size() * p1 / 100); 
        }
        else {
            y_avg_beg = upper_bound (v.begin(), v.end(), v[0] + p1) - v.begin();
            if (y_avg_beg == size(v))
                y_avg_beg--;
        }
        if (is_proc2) {
            p2 = min (max (p2, 0.), 100.);
            y_avg_end = y_avg_beg + static_cast<int>(v.size() * p2 / 100);
        }
        else {
            fp end_val = v[y_avg_beg] + p2;
            y_avg_end = upper_bound (v.begin(), v.end(), end_val) - v.begin();
        }
        if (y_avg_beg < 0) 
            y_avg_beg = 0;
        if (y_avg_end > size(v))
            y_avg_end = v.size();
        if (y_avg_beg >= y_avg_end) {
            if (y_avg_beg >= size(v))
                y_avg_beg = v.size() - 1;;
            y_avg_end = y_avg_beg + 1;
        }
        int counter = 0;
        fp y = 0;
        for (int j = y_avg_beg; j < y_avg_end; j++){
            counter++;
            y += v[j];
        }
        y /= counter;
        add_background_point ((p[l].x + p[u - 1].x) / 2, y, bgc_bg);
    }
}
*/


void BgManager::add_background_point(fp x, fp y)
{
    rm_background_point(x);
    B_point t(x, y);
    bg_iterator l = lower_bound(bg.begin(), bg.end(), t);
    bg.insert (l, t);
    recompute_bgline();
}

void BgManager::rm_background_point (fp x)
{
    fp dx = x_calc.dX2dx(min_dist);
    bg_iterator l = lower_bound(bg.begin(), bg.end(), B_point(x-dx, 0));
    bg_iterator u = upper_bound (bg.begin(), bg.end(), B_point(x+dx, 0));
    if (u > l) {
        bg.erase(l, u);
        verbose (S(u - l) + " background points removed.");
        recompute_bgline();
    }
}

void BgManager::clear_background()
{
    int n = bg.size();
    bg.clear();
    recompute_bgline();
    verbose (S(n) + " background points deleted.");
}

void BgManager::strip_background()
{
    if (bg.empty())
        return;
    vector<fp> pars;
    for (bg_const_iterator i = bg.begin(); i != bg.end(); i++) {
        pars.push_back(i->x);
        pars.push_back(i->y);
    }
    clear_background();
    exec_command("Y = y - spline[" + join_vector(pars, ", ") + "](x)" 
                 + frame->get_in_one_or_all_datasets());
    verbose("Background stripped.");
}

void BgManager::recompute_bgline()
{
    int focused_data = AL->get_active_ds_position();
    const std::vector<Point>& p = AL->get_data(focused_data)->points();
    bgline.resize(p.size());
    if (spline_bg) 
        prepare_spline_interpolation(bg);
    for (int i = 0; i < size(p); i++) {
        bgline[i].x = p[i].x;
        bgline[i].y = spline_bg ? get_spline_interpolation(bg, p[i].x)
                                : get_linear_interpolation(bg, p[i].x);
    }
}


