#pragma once
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <functional>
#include "MediaDecoder.h"
#include <PCMPlayer.h>

class VideoCanvas : public wxGLCanvas {
public:
    VideoCanvas(wxWindow* parent);
    ~VideoCanvas();

    void StartStream(const std::string& url, std::function<void(int)> onStatus);
    void StopStream();

private:
    wxWeakRef<VideoCanvas> m_weakThis;
    wxGLContext glContext;
    MediaDecoder* m_decoder;
    std::shared_ptr<PCMPlayer> m_player;

    std::vector<uint8_t> frameBuffer;
    int frameWidth = 0, frameHeight = 0;
    std::mutex frameMutex;
    std::atomic<bool> newFrameReceived{ false };
    std::atomic<bool> running{ false };

    GLuint textureId = 0;
    bool textureInitialized = false;

    std::function<void(int)> statusCallback;

    void OnPaint(wxPaintEvent&);
    void OnResize(wxSizeEvent&);

    wxDECLARE_EVENT_TABLE();
};
