// MainPanel.cpp
#include "MainPanel.h"

MainPanel::MainPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	urlInput = new wxTextCtrl(this, wxID_ANY, "rtsp://127.0.0.1:8554/hls/test", wxDefaultPosition, wxSize(400, -1));
	playBtn = new wxButton(this, wxID_ANY, "Play");
	stopBtn = new wxButton(this, wxID_ANY, "Stop");
	statusLabel = new wxStaticText(this, wxID_ANY, "Status: Stopped");
	videoCanvas = new VideoCanvas(this);

	auto* topSizer = new wxBoxSizer(wxHORIZONTAL);
	topSizer->Add(new wxStaticText(this, wxID_ANY, "URL:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	topSizer->Add(urlInput, 1, wxEXPAND | wxALL, 5);
	topSizer->Add(playBtn, 0, wxALL, 5);
	topSizer->Add(stopBtn, 0, wxALL, 5);
	topSizer->Add(statusLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	auto* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(topSizer, 0, wxEXPAND);
	mainSizer->Add(videoCanvas, 1, wxEXPAND | wxALL, 5);
	SetSizer(mainSizer);

	playBtn->Bind(wxEVT_BUTTON, &MainPanel::OnPlay, this);
	stopBtn->Bind(wxEVT_BUTTON, &MainPanel::OnStop, this);
}

void MainPanel::OnPlay(wxCommandEvent&) {
	statusLabel->SetLabel("Status: Opening...");
	videoCanvas->StartStream(urlInput->GetValue().ToStdString(),
		[this](int ok) {
			wxTheApp->CallAfter([this, ok]() {
				if (ok == 1)
				{
					statusLabel->SetLabel("Status: Playing");
				}
				else if (ok == 2)
				{
					statusLabel->SetLabel("Status: Stopped");
				}
				else
				{
					statusLabel->SetLabel("Status: Error");
				}
				});
		});
}

void MainPanel::OnStop(wxCommandEvent&) {
	videoCanvas->StopStream();
	statusLabel->SetLabel("Status: Stopped");
}
