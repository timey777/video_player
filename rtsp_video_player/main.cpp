#include <wx/wx.h>
#include <MainPanel.h>
#include <vld.h>
// 主框架
class MainFrame : public wxFrame {
public:
    MainFrame() : wxFrame(nullptr, wxID_ANY, "RTSP Player", wxDefaultPosition, wxSize(800, 600)) {
        new MainPanel(this);
    }
};

class MyApp : public wxApp {
public:
    bool OnInit() override {
        std::set_terminate(TerminateHandler);
        if (!wxApp::OnInit()) return false;
        MainFrame* frame = new MainFrame();
        frame->Show();
        return true;
    }
    static void TerminateHandler();
};
void MyApp::TerminateHandler() {
    wxLogError("程序即将异常终止！");
    std::abort();
}
wxIMPLEMENT_APP_CONSOLE(MyApp);
