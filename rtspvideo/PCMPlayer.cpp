#include "PCMPlayer.h"

PCMPlayer::PCMPlayer(int ch, int sr, int bps, int bufSamps, int bufCount)
    : sampleRate(sr)
{
    device = alcOpenDevice(nullptr);
    context = alcCreateContext(device, nullptr);
    alcMakeContextCurrent(context);

    format = (bps == 16 ? (ch == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16)
        : (ch == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8));

    alGenSources(1, &source);
    buffers.resize(bufCount);
    alGenBuffers(bufCount, buffers.data());
    for (auto b : buffers) freeBuffers.push(b);
}

PCMPlayer::~PCMPlayer() {
    stop();
    alDeleteSources(1, &source);
    alDeleteBuffers(buffers.size(), buffers.data());
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

bool PCMPlayer::start() {
    if (threadRunning) return false;
    threadRunning = true;
    audioThread = std::thread(&PCMPlayer::audioLoop, this);
    return true;
}

void PCMPlayer::stop() {
    threadRunning = false;
    if (audioThread.joinable()) audioThread.join();

    alSourceStop(source);
    ALint queued;
    alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
    while (queued--) {
        ALuint b;
        alSourceUnqueueBuffers(source, 1, &b);
    }

    std::lock_guard<std::mutex> lock(mtx);
    while (!freeBuffers.empty()) freeBuffers.pop();
    for (auto b : buffers) freeBuffers.push(b);
}

bool PCMPlayer::enqueue(const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(mtx);
    if (freeBuffers.empty()) return false;
    ALuint buf = freeBuffers.front(); freeBuffers.pop();
    alBufferData(buf, format, data, (ALsizei)size, sampleRate);
    alSourceQueueBuffers(source, 1, &buf);
    return true;
}

void PCMPlayer::update() {
    ALint processed = 0;
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
    while (processed-- > 0) {
        ALuint buf;
        alSourceUnqueueBuffers(source, 1, &buf);
        std::lock_guard<std::mutex> lock(mtx);
        freeBuffers.push(buf);
    }
}

void PCMPlayer::audioLoop() {
    while (threadRunning) {

        ALint queued = 0;
        alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
        if (queued > 0) {
            ALint state;
            alGetSourcei(source, AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING)
                alSourcePlay(source);
        }
        else {
            // 队列空时保持停止状态，不重启
        }
        update();  // 回收 processed buffers

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
