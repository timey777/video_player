// 简单控制台示例：使用 Player 类播放 RTSP 流（不做绘制）
#include <iostream>
#include <thread>
#include <chrono>
#include "Player.h"

int main(int argc, char** argv) {
    std::string url = "rtsp://192.168.1.105:8554/test";
    if (argc > 1) url = argv[1];

    Player player;

    player.SetStatusCallback([](int status) {
        // status: 1 playing, 2 stopped, 3 error
        if (status == 1) std::cout << "Status: Playing\n";
        else if (status == 2) std::cout << "Status: Stopped\n";
        else std::cout << "Status: Error\n";
    });

    player.SetVideoCallback([](const uint8_t* data, int w, int h) {
        // 这里只输出帧信息；上层可以用 OpenGL/SDL 绘制
        static int cnt = 0;
        ++cnt;
        if (cnt % 30 == 0) std::cout << "Received video frame: " << w << "x" << h << "\n";
    });

    if (!player.Start(url)) {
        std::cerr << "Failed to start player" << std::endl;
        return 1;
    }

    std::cout << "Started player for: " << url << "\n";
    std::cout << "Press Enter to stop..." << std::endl;
    std::string dummy;
    std::getline(std::cin, dummy);

    player.Stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Player stopped. Exiting." << std::endl;
    return 0;
}
