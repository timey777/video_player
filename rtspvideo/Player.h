// Player.h
#pragma once
#include "rtspvideo_export.h"
#include <functional>
#include <memory>
#include <atomic>
#include <vector>
#include "MediaDecoder.h"
#include "PCMPlayer.h"

class RTSPVIDEO_API Player {
public:
    using VideoCallback = std::function<void(const uint8_t* data, int width, int height)>;
    using StatusCallback = std::function<void(int)>; // 1 playing, 2 stopped, 3 error

    Player();
    ~Player();

    // 返回是否成功开始打开流（异步解码仍可能失败，status 回调会通知）
    bool Start(const std::string& url);
    void Stop();

    void SetVideoCallback(VideoCallback cb) { m_videoCb = std::move(cb); }
    void SetStatusCallback(StatusCallback cb) { m_statusCb = std::move(cb); }

private:
    MediaDecoder* m_decoder = nullptr;
    std::shared_ptr<PCMPlayer> m_player;
    VideoCallback m_videoCb;
    StatusCallback m_statusCb;
    std::atomic<bool> m_running{ false };
};
