#pragma once
#include "rtspvideo_export.h"
#include "AVCommon.h"

// 前向声明 swr/AV 类型，避免在头文件中包含 libswresample 等 heavy headers
struct SwrContext;
struct AVChannelLayout;
enum AVSampleFormat : int;

class RTSPVIDEO_API AudioResampler
{
public:
    AudioResampler& operator=(const AudioResampler&) = delete;
    AudioResampler(const AudioResampler&) = delete;
    AudioResampler();
    ~AudioResampler();
    bool Init(int in_samplerate, AVChannelLayout in_layout, AVSampleFormat in_format, int out_samplerate, AVChannelLayout out_layout, AVSampleFormat out_format);

    int Convert(avcommon::AVFramePtr in_frame, uint8_t** converted_input_samples);
private:
    AVChannelLayout out_layout_;
    AVSampleFormat out_format_;
    void Destroy();
    SwrContext* swrCtx = nullptr;
};