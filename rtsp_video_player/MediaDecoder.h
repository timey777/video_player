#pragma once
#include <thread>
#include <atomic>
#include <string>
#include <functional>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavutil/channel_layout.h>
}

class MediaDecoder {
public:
    MediaDecoder();
    ~MediaDecoder();

    void OpenAsync(const std::string& url, std::function<void(bool)> onOpen);
    bool Open(const std::string& url);
    void Close();

    void StartDecoding();
    void StopDecoding();

    void SetVideoCallback(std::function<void(AVFrame*, int, int)> cb) { m_videoCb = cb; }
    void SetAudioCallback(std::function<void(uint8_t*, int, int, int)> cb) { m_audioCb = cb; }
    void SetErrorCallback(std::function<void(int)> cb) { m_errorCb = cb; }

private:
    void DecodingThread();

    AVFormatContext* m_fmtCtx = nullptr;
    AVCodecContext* m_vCtx = nullptr;
    AVCodecContext* m_aCtx = nullptr;
    SwsContext* m_sws = nullptr;
    SwrContext* m_swr = nullptr;

    int m_vidIdx = -1, m_audIdx = -1;
    std::thread m_thread;
    std::atomic<bool> m_running{ false };

    std::function<void(AVFrame*, int, int)> m_videoCb;
    std::function<void(uint8_t*, int, int, int)> m_audioCb;
    std::function<void(int)> m_errorCb;
};
