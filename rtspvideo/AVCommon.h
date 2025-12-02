#ifndef __AVCOMMON_H__
#define __AVCOMMON_H__

#include <string>
#include <array>
#include <memory>
#include <stdexcept>
extern "C" {
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
}

#include "rtspvideo_export.h"

namespace avcommon
{
    using AVFramePtr = std::shared_ptr<AVFrame>;
    using AVPacketPtr = std::shared_ptr<AVPacket>;
    // AVFramePtr
    RTSPVIDEO_API void AVFrameDeleter(AVFrame* frame);

    // 创建并返回一个受管理的 AVFrame
    RTSPVIDEO_API AVFramePtr make_shared_avframe();

    // AVPacketPtr
    RTSPVIDEO_API void AVPacketDeleter(AVPacket* packet);

    // 创建并返回一个受管理的 AVPacket
    RTSPVIDEO_API std::shared_ptr<AVPacket> make_shared_avpacket();
    RTSPVIDEO_API std::string av_error_to_string(int errnum);
}
#endif // !__AVCOMMON_H__
