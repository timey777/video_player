#include "AVCommon.h"
using namespace avcommon;
// AVFramePtr
void avcommon::AVFrameDeleter(AVFrame* frame) {
	av_frame_free(&frame);
}

// 创建并返回一个受管理的 AVFrame
avcommon::AVFramePtr avcommon::make_shared_avframe() {
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		throw std::runtime_error("Failed to allocate AVFrame");
	}
	return avcommon::AVFramePtr(frame, AVFrameDeleter);
}

// AVPacketPtr
void avcommon::AVPacketDeleter(AVPacket* packet) {
	av_packet_free(&packet);
}

// 创建并返回一个受管理的 AVPacket
avcommon::AVPacketPtr avcommon::make_shared_avpacket() {
	AVPacket* packet = av_packet_alloc();
	if (!packet) {
		throw std::runtime_error("Failed to allocate AVPacket");
	}
	return avcommon::AVPacketPtr(packet, AVPacketDeleter);
}
std::string avcommon::av_error_to_string(int errnum) {
	std::array<char, AV_ERROR_MAX_STRING_SIZE> buf{};
	av_make_error_string(buf.data(), buf.size(), errnum);
	return std::string(buf.data());
}
