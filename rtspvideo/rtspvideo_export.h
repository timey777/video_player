#pragma once

// 导出宏定义（与 CMake 中的 RTSPVIDEO_EXPORTS 一致）
#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef RTSPVIDEO_EXPORTS
    #define RTSPVIDEO_API __declspec(dllexport)
  #else
    #define RTSPVIDEO_API __declspec(dllimport)
  #endif
#else
  #define RTSPVIDEO_API
#endif
