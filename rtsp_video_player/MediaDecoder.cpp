#include "MediaDecoder.h"
#include <iostream>

MediaDecoder::MediaDecoder() { avformat_network_init(); }
MediaDecoder::~MediaDecoder() { Close(); avformat_network_deinit(); }

void MediaDecoder::OpenAsync(const std::string& url, std::function<void(bool)> onOpen) {
	std::thread([=]() { bool ok = Open(url); if (onOpen) onOpen(ok); }).detach();
}

bool MediaDecoder::Open(const std::string& url) {
	AVDictionary* opts = nullptr;
	av_dict_set(&opts, "timeout", "3000000", 0);
	av_dict_set(&opts, "rw_timeout", "3000000", 0);

	if (avformat_open_input(&m_fmtCtx, url.c_str(), nullptr, &opts) != 0) return false;
	if (avformat_find_stream_info(m_fmtCtx, nullptr) < 0) return false;

	for (unsigned i = 0; i < m_fmtCtx->nb_streams; i++) {
		auto p = m_fmtCtx->streams[i]->codecpar;
		if (p->codec_type == AVMEDIA_TYPE_VIDEO) m_vidIdx = i;
		else if (p->codec_type == AVMEDIA_TYPE_AUDIO) m_audIdx = i;
	}
	if (m_vidIdx < 0 && m_audIdx < 0) return false;

	if (m_vidIdx >= 0) {
		auto p = m_fmtCtx->streams[m_vidIdx]->codecpar;
		auto c = avcodec_find_decoder(p->codec_id);
		m_vCtx = avcodec_alloc_context3(c);
		avcodec_parameters_to_context(m_vCtx, p);
		avcodec_open2(m_vCtx, c, nullptr);
		m_sws = sws_getContext(
			m_vCtx->width, m_vCtx->height, m_vCtx->pix_fmt,
			m_vCtx->width, m_vCtx->height, AV_PIX_FMT_RGB24,
			SWS_BILINEAR, nullptr, nullptr, nullptr);
	}

	if (m_audIdx >= 0) {
		// 初始化解码器
		auto codecParams = m_fmtCtx->streams[m_audIdx]->codecpar;
		auto codec = avcodec_find_decoder(codecParams->codec_id);
		m_aCtx = avcodec_alloc_context3(codec);
		avcodec_parameters_to_context(m_aCtx, codecParams);
		if (avcodec_open2(m_aCtx, codec, nullptr) < 0) {
			std::cerr << "Error: Failed to open codec" << std::endl;
			return false;
		}

		SwrContext* swr = nullptr;
		AVChannelLayout outChannelLayout = AV_CHANNEL_LAYOUT_STEREO;
		AVChannelLayout inChannelLayout;
		av_channel_layout_copy(&inChannelLayout, &codecParams->ch_layout);
		int ret = swr_alloc_set_opts2(
			&swr,                     // 输出参数：SwrContext**
			&outChannelLayout,           // 输出声道布局
			AV_SAMPLE_FMT_S16,           // 输出格式：16-bit PCM
			44100,                       // 输出采样率
			&inChannelLayout,            // 输入声道布局
			m_aCtx->sample_fmt,        // 输入格式
			codecParams->sample_rate,    // 输入采样率
			0,                           // 日志偏移
			nullptr                      // 日志上下文
		);
		if (ret < 0 || !swr || swr_init(swr) < 0) {
			std::cerr << "swr_alloc_set_opts2 failed " << ret << "\n";
			if (swr) swr_free(&swr);
			return false;
		}
		m_swr = swr;
	}
	return true;
}

void MediaDecoder::StartDecoding() {
	if (m_running) return;
	m_running = true;
	m_thread = std::thread(&MediaDecoder::DecodingThread, this);
}

void MediaDecoder::StopDecoding() {
	m_running = false;
	if (m_thread.joinable()) m_thread.join();
}

void MediaDecoder::Close() {
	StopDecoding();
	if (m_swr) {
		swr_free(&m_swr);
		m_swr = nullptr;
	}
	if (m_sws) {
		sws_freeContext(m_sws);
		m_sws = nullptr;
	}
	if (m_vCtx) {
		avcodec_free_context(&m_vCtx);
		m_vCtx = nullptr;
	}
	if (m_aCtx) {
		avcodec_free_context(&m_aCtx);
		m_aCtx = nullptr;
	}
	if (m_fmtCtx) {
		avformat_close_input(&m_fmtCtx);
		m_fmtCtx = nullptr;
	}
}

void MediaDecoder::DecodingThread() {
	AVPacket* pkt = av_packet_alloc();
	AVFrame* vf = av_frame_alloc();
	AVFrame* af = av_frame_alloc();
	AVFrame* rgb = nullptr;
	uint8_t* buf = nullptr;
	if (m_vCtx) {
		rgb = av_frame_alloc();
		int sz = av_image_get_buffer_size(AV_PIX_FMT_RGB24, m_vCtx->width, m_vCtx->height, 1);
		buf = (uint8_t*)av_malloc(sz);
		av_image_fill_arrays(rgb->data, rgb->linesize, buf, AV_PIX_FMT_RGB24, m_vCtx->width, m_vCtx->height, 1);
	}

	while (m_running && av_read_frame(m_fmtCtx, pkt) >= 0) {
		if (pkt->stream_index == m_vidIdx) {
			if (avcodec_send_packet(m_vCtx, pkt) == 0)
				while (avcodec_receive_frame(m_vCtx, vf) == 0) {
					sws_scale(m_sws, vf->data, vf->linesize, 0, vf->height, rgb->data, rgb->linesize);
					if (m_videoCb) m_videoCb(rgb, m_vCtx->width, m_vCtx->height);
				}
		}
		else if (pkt->stream_index == m_audIdx) {
			if (avcodec_send_packet(m_aCtx, pkt) == 0)
				while (avcodec_receive_frame(m_aCtx, af) == 0) {
					int outSamples = swr_get_out_samples(m_swr, af->nb_samples);
					uint8_t* outBuf[1] = { nullptr };
					av_samples_alloc(outBuf, nullptr, m_aCtx->ch_layout.nb_channels, outSamples, AV_SAMPLE_FMT_S16, 0);
					int conv = swr_convert(m_swr, outBuf, outSamples, (const uint8_t**)af->data, af->nb_samples);
					if (conv > 0 && m_audioCb)
						m_audioCb(outBuf[0], conv * 2 * sizeof(short), m_aCtx->ch_layout.nb_channels, m_aCtx->sample_rate);

					if (outBuf[0]) av_freep(&outBuf[0]);
				}
		}
		av_packet_unref(pkt);
	}

	av_free(buf);
	av_frame_free(&rgb);
	av_frame_free(&vf);
	av_frame_free(&af);
	av_packet_free(&pkt);
	if (m_errorCb) m_errorCb(0);
}
