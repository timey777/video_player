// 简单控制台示例：使用 Player 类播放 RTSP 流（不做绘制）
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
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
    // SDL rendering path
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    std::mutex sdl_mtx;
    int tex_w = 0, tex_h = 0;

    player.SetVideoCallback([&](const uint8_t* data, int w, int h) {
        std::lock_guard<std::mutex> lk(sdl_mtx);
        if (!window) {
            window = SDL_CreateWindow("RTSP Player", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
            if (!window) {
                std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
                return;
            }
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (!renderer) {
                std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
                return;
            }
        }

        if (!texture || tex_w != w || tex_h != h) {
            if (texture) SDL_DestroyTexture(texture);
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, w, h);
            tex_w = w; tex_h = h;
            SDL_SetWindowSize(window, w, h);
        }

        if (texture) {
            int pitch = w * 3;
            SDL_UpdateTexture(texture, nullptr, data, pitch);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }
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
    });

    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (inputThread.joinable()) inputThread.join();

    player.Stop();

    {
        std::lock_guard<std::mutex> lk(sdl_mtx);
        if (texture) { SDL_DestroyTexture(texture); texture = nullptr; }
        if (renderer) { SDL_DestroyRenderer(renderer); renderer = nullptr; }
        if (window) { SDL_DestroyWindow(window); window = nullptr; }
    }
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
