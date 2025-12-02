// Player.cpp
#include "Player.h"
#include <iostream>
#include <cstring>
extern "C" {
#include <libavutil/frame.h>
}

Player::Player()
    : m_decoder(new MediaDecoder())
{
    const int channels = 2;
    const int sampleRate = 48000;
    const int bits = 16;
    const int bufSeconds = 1;
    const int bufferSamples = sampleRate * bufSeconds;
    m_player = std::make_shared<PCMPlayer>(channels, sampleRate, bits, bufferSamples, 8);
}

Player::~Player() {
    Stop();
    if (m_decoder) delete m_decoder;
    m_decoder = nullptr;
}

bool Player::Start(const std::string& url) {
    if (m_running) return false;

    if (!m_player->start()) {
        std::cerr << "PCMPlayer failed to start" << std::endl;
    }

    m_decoder->SetVideoCallback([this](AVFrame* frame, int w, int h) {
        if (!frame) return;
        // 假设回传的 frame->data[0] 已经是 RGB24 数据
        size_t bytes = (size_t)w * (size_t)h * 3;
        if (m_videoCb) {
            m_videoCb(frame->data[0], w, h);
        }
    });

    m_decoder->SetAudioCallback([this](uint8_t* pcm, int nbSamples, int channels, int sampleRate) {
        if (!pcm) return;
        // 假设 bitsPerSample == 16
        size_t bytes = (size_t)nbSamples * (size_t)channels * 2;
        m_player->enqueue(pcm, bytes);
    });

    m_decoder->SetErrorCallback([this](int error) {
        m_running = false;
        if (m_statusCb) m_statusCb(error == 0 ? 2 : 3);
    });

    m_decoder->OpenAsync(url, [this](bool ok) {
        if (!ok) {
            if (m_statusCb) m_statusCb(3);
            m_decoder->Close();
            m_running = false;
            return;
        }
        m_running = true;
        if (m_statusCb) m_statusCb(1);
        m_decoder->StartDecoding();
    });

    return true;
}

void Player::Stop() {
    if (!m_running) return;
    m_decoder->Close();
    m_player->stop();
    m_running = false;
    if (m_statusCb) m_statusCb(2);
}
