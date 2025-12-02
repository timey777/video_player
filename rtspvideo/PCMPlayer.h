#pragma once
#include "rtspvideo_export.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>
#include <vector>

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

    ALCdevice* device = nullptr;
    ALCcontext* context = nullptr;
    ALuint source = 0;
    std::vector<ALuint> buffers;
    std::queue<ALuint> freeBuffers;
    std::mutex mtx;
    int sampleRate;
    ALenum format;
    std::thread audioThread;
    std::atomic<bool> threadRunning{ false };
};
