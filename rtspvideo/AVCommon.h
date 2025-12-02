#ifndef __AVCOMMON_H__
#define __AVCOMMON_H__

#include <string>
#include <array>
#include <memory>
#include <stdexcept>
#include "rtspvideo_export.h"

// 前向声明 FFmpeg 类型，避免在头文件中包含 heavy headers
struct AVFrame;
struct AVPacket;

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
