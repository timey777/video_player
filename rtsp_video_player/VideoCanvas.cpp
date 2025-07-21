#include "VideoCanvas.h"
#include <wx/app.h>
#include <GL/gl.h>

wxBEGIN_EVENT_TABLE(VideoCanvas, wxGLCanvas)
EVT_PAINT(VideoCanvas::OnPaint)
EVT_SIZE(VideoCanvas::OnResize)
wxEND_EVENT_TABLE()

VideoCanvas::VideoCanvas(wxWindow* parent)
	: wxGLCanvas(parent, wxID_ANY, nullptr),
	glContext(this), m_decoder(new MediaDecoder())
{
	m_weakThis = this;
	const int channels = 2;
	const int sampleRate = 48000;
	const int bits = 16;
	const int bufSeconds = 1;
	const int bufferSamples = sampleRate * bufSeconds;
	const int bufSize = bufferSamples * channels * bits / 8;
	m_player = std::make_shared<PCMPlayer>(channels, sampleRate, bits, bufferSamples, 8);
}

VideoCanvas::~VideoCanvas() {
	m_weakThis = nullptr;
	delete m_decoder;
}

void VideoCanvas::StartStream(const std::string& url, std::function<void(int)> onStatus) {
	if (running) return;
	statusCallback = onStatus;
	if (!m_player->start()) {
		std::cerr << "e\n";
	}
	m_decoder->SetVideoCallback([this](AVFrame* frame, int w, int h) {
		std::vector<uint8_t> buffer(frame->data[0], frame->data[0] + w * h * 3);
		{
			std::lock_guard<std::mutex> lk(frameMutex);
			frameBuffer.swap(buffer);
			frameWidth = w;
			frameHeight = h;
			newFrameReceived = true;
		}
		wxTheApp->CallAfter([weakThis = m_weakThis]() {
			if (weakThis) {
				weakThis->Refresh(false);  // 安全调用
			}});
		});

	m_decoder->SetAudioCallback([this](uint8_t* pcm, int nbSamples, int channels, int sampleRate) {
		/*while (!*/m_player->enqueue(pcm, nbSamples);/*) {*/
			//std::this_thread::sleep_for(std::chrono::milliseconds(5));
		//}
		wxTheApp->CallAfter([weakThis = m_weakThis]() {
			if (weakThis) {
				weakThis->Refresh(false);  // 安全调用
			}});
		});
	m_decoder->SetErrorCallback([this](int error) {
		// 解码线程结束（流断开或 StopStream 被调用）
		running = false;
		wxTheApp->CallAfter([weakThis = m_weakThis, error]() {
			if (weakThis && weakThis->statusCallback)  weakThis->statusCallback(error == 0 ? 2 : 3);
			});
		});
	try
	{
		// 异步打开流
		m_decoder->OpenAsync(url, [this](bool ok) {
			wxTheApp->CallAfter([this, weakThis = m_weakThis, ok]() {
				if (weakThis) {
					if (weakThis->statusCallback)  weakThis->statusCallback(ok ? 1 : 3);
					if (!ok) {
						m_decoder->Close();
						running = false;
						return;
					}

					running = true;
					textureInitialized = false;

					m_decoder->StartDecoding();
				}
				});
			});
	}
	catch (const std::exception& e)
	{
		std::cout << "" << std::endl;
	}
}

void VideoCanvas::StopStream() {
	if (!running) return;
	m_decoder->Close();
	m_player->stop();
	running = false;
}

void VideoCanvas::OnPaint(wxPaintEvent&) {
	wxPaintDC dc(this);
	SetCurrent(glContext);

	int w, h;
	GetSize(&w, &h);
	glViewport(0, 0, w, h);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	std::lock_guard<std::mutex> lk(frameMutex);
	if (newFrameReceived) {
		if (!textureInitialized) {
			glGenTextures(1, &textureId);
			textureInitialized = true;
		}
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameWidth, frameHeight, 0,
			GL_RGB, GL_UNSIGNED_BYTE, frameBuffer.data());
		newFrameReceived = false;
	}

	if (textureInitialized) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 1); glVertex2f(-1, -1);
		glTexCoord2f(1, 1); glVertex2f(1, -1);
		glTexCoord2f(1, 0); glVertex2f(1, 1);
		glTexCoord2f(0, 0); glVertex2f(-1, 1);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}

	SwapBuffers();
}

void VideoCanvas::OnResize(wxSizeEvent&) {
	Refresh(false);
}
