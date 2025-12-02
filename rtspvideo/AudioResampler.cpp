#include "AudioResampler.h"
#include <iostream>

AudioResampler::AudioResampler()
{
}

AudioResampler::~AudioResampler()
{
    Destroy();
}

bool AudioResampler::Init(int in_samplerate, AVChannelLayout in_layout, AVSampleFormat in_format, int out_samplerate, AVChannelLayout out_layout, AVSampleFormat out_format)
{
    if (swrCtx)
    {
        Destroy();
    }
    out_layout_ = out_layout;
    out_format_ = out_format;
    int ret = swr_alloc_set_opts2(
        &swrCtx,                     // 输出参数：SwrContext**
        &out_layout,           // 输出声道布局
        out_format,           // 输出格式：16-bit PCM
        out_samplerate,                       // 输出采样率
        &in_layout,            // 输入声道布局
        in_format,        // 输入格式
        in_samplerate,    // 输入采样率
        0,                           // 日志偏移
        nullptr                      // 日志上下文
    );
    if (ret < 0 || !swrCtx) {
        std::cerr << "Error: Failed to init resampler" << std::endl;
        return false;
    }
    if (swr_init(swrCtx) < 0) {
        std::cerr << "Error: Failed to init swr" << std::endl;
        return false;
    }
    return true;
}

int AudioResampler::Convert(avcommon::AVFramePtr in_frame, uint8_t** converted_input_samples)
{
    if (swrCtx == nullptr) {
        return -1;
    }
    int out_samples_count = swr_get_out_samples(swrCtx, in_frame->nb_samples);
    av_samples_alloc(converted_input_samples, NULL, out_layout_.nb_channels, out_samples_count, out_format_, 0);

    int error;

    if ((error = swr_convert(swrCtx,
        converted_input_samples, out_samples_count, (const uint8_t**)in_frame->data, in_frame->nb_samples)) < 0) {
        std::cerr << "Could not convert input samples (error '%s')\n" << avcommon::av_error_to_string(error);
        return error;
    }

    return error;
}

void AudioResampler::Destroy()
{
    if (swrCtx)
    {
        swr_free(&swrCtx);
    }
}
