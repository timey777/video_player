// 简单控制台示例：使用 Player 类播放 RTSP 流（不做绘制）
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <deque>
#include <vector>
#include <condition_variable>
#include <cstring>
#include "Player.h"

#ifdef USE_SDL
#include <SDL.h>
#endif

int main(int argc, char** argv) {
    std::string url = "rtsp://192.168.1.105:8554/test";
    if (argc > 1) url = argv[1];

    Player player;

    player.SetStatusCallback([](int status) {
        if (status == 1) std::cout << "Status: Playing\n";
        else if (status == 2) std::cout << "Status: Stopped\n";
        else std::cout << "Status: Error\n";
    });

#ifdef USE_SDL
    // SDL rendering path (main-thread rendering). Decoder callback will only enqueue frames.
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;

    struct Frame {
        std::vector<uint8_t> data;
        int w;
        int h;
    };

    std::deque<Frame> frameQueue;
    std::mutex queueMtx;
    std::condition_variable queueCv;
    const size_t MAX_QUEUE = 3; // drop old frames when lagging

    // Decoder thread -> enqueue a copy of the RGB24 frame
    player.SetVideoCallback([&](const uint8_t* data, int w, int h) {
        if (!data || w <= 0 || h <= 0) return;
        Frame f;
        try {
            f.w = w; f.h = h;
            f.data.resize((size_t)w * (size_t)h * 3);
            std::memcpy(f.data.data(), data, f.data.size());
        } catch (...) {
            return;
        }
        {
            std::lock_guard<std::mutex> lk(queueMtx);
            frameQueue.push_back(std::move(f));
            while (frameQueue.size() > MAX_QUEUE) frameQueue.pop_front();
        }
        queueCv.notify_one();
    });

#else
    // Fallback: simple console output
    player.SetVideoCallback([](const uint8_t* data, int w, int h) {
        static int cnt = 0;
        ++cnt;
        if (cnt % 30 == 0) std::cout << "Received video frame: " << w << "x" << h << "\n";
    });
#endif

    if (!player.Start(url)) {
        std::cerr << "Failed to start player" << std::endl;
        return 1;
    }

    std::cout << "Started player for: " << url << "\n";

#ifdef USE_SDL
    // Main loop: handle SDL events until window closed or user presses Enter
    bool quit = false;
    std::thread inputThread([&]() {
        std::string dummy;
        std::getline(std::cin, dummy);
        quit = true;
        queueCv.notify_one();
    });

    // Rendering loop runs in main thread: wait for frames from decoder and render them
    while (!quit) {
        // Wait for a frame or timeout to handle events
        std::unique_lock<std::mutex> lk(queueMtx);
        if (frameQueue.empty()) {
            queueCv.wait_for(lk, std::chrono::milliseconds(50));
        }

        // Handle SDL events regardless
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
        }

        if (frameQueue.empty()) {
            if (quit) break;
            continue;
        }

        // Pop one frame to render
        Frame f = std::move(frameQueue.front());
        frameQueue.pop_front();
        lk.unlock();

        if (!window) {
            window = SDL_CreateWindow("RTSP Player", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, f.w, f.h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
            if (!window) {
                std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
                break;
            }
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (!renderer) {
                std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
                break;
            }
        }

        if (!texture) {
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, f.w, f.h);
            SDL_SetWindowSize(window, f.w, f.h);
        }

        if (texture) {
            int pitch = f.w * 3;
            SDL_UpdateTexture(texture, nullptr, f.data.data(), pitch);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }
    }

    if (inputThread.joinable()) inputThread.join();

    player.Stop();

    if (texture) { SDL_DestroyTexture(texture); texture = nullptr; }
    if (renderer) { SDL_DestroyRenderer(renderer); renderer = nullptr; }
    if (window) { SDL_DestroyWindow(window); window = nullptr; }
    SDL_Quit();

#else
    std::cout << "Press Enter to stop..." << std::endl;
    std::string dummy;
    std::getline(std::cin, dummy);
    player.Stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
#endif

    std::cout << "Player stopped. Exiting." << std::endl;
    return 0;
}
