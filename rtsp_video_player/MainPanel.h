// MainPanel.h
#pragma once
#include <wx/wx.h>
#include "VideoCanvas.h"

class MainPanel : public wxPanel {
public:
    MainPanel(wxWindow* parent);
private:
    wxTextCtrl* urlInput;
    wxButton* playBtn;
    wxButton* stopBtn;
    wxStaticText* statusLabel;
    VideoCanvas* videoCanvas;

    void OnPlay(wxCommandEvent&);
    void OnStop(wxCommandEvent&);
};
