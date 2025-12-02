#pragma once
#include "rtspvideo_export.h"
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>
#include <vector>

// 轻量定义 OpenAL 基本类型，避免在头文件中包含 heavy header
using ALuint = unsigned int;
using ALenum = int;

class RTSPVIDEO_API PCMPlayer {
public:
    PCMPlayer(int channels, int sampleRate, int bitsPerSample, int bufferSamples, int bufferCount = 4);
    ~PCMPlayer();
    bool start();
    void stop();
    bool enqueue(const void* data, size_t size);
private:
    void audioLoop();
    void update();

    void* device = nullptr; // actually ALCdevice*
    void* context = nullptr; // actually ALCcontext*
    ALuint source = 0;
    std::vector<ALuint> buffers;
    std::queue<ALuint> freeBuffers;
    std::mutex mtx;
    int sampleRate;
    ALenum format;
    std::thread audioThread;
    std::atomic<bool> threadRunning{ false };
};
