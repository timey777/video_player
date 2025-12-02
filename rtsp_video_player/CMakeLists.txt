# cmake最低版本号要求
cmake_minimum_required (VERSION 3.8)
set(PROJECT_NAME rtsp_video_player)
project (${PROJECT_NAME})
set (${PROJECT_NAME}_VERSION_MAJOR 1)
set (${PROJECT_NAME}_VERSION_MINOR 0)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set (ROOT_DIR ${CMAKE_SOURCE_DIR})
set (3RD_DIR ${ROOT_DIR}/thirdparty)
set (BUILD_DIR ${ROOT_DIR}/build)
set (CMAKE_MODULE_PATH ${3RD_DIR})

message("CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")
message("CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
# vld windows平台使用
set (VLD_DIR ${3RD_DIR}/vld)
include_directories (${VLD_DIR}/include)
link_directory_by_cl(${3RD_DIR} vld)

# 设置MSVC编译编码
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")

# 添加src目录引用
include_directories(${PROJECT_SOURCE_DIR})
# 递归列出所有源文件
file (GLOB_RECURSE SOURCE_FILES RELATIVE ${PROJECT_SOURCE_DIR} *.cpp *.h)
source_group_by_dir(SOURCE_FILES)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE ${CPP_DEFS})
target_link_libraries(${PROJECT_NAME} PRIVATE common OpenAL)
target_link_libraries(${PROJECT_NAME} PRIVATE alure2_s)

target_include_directories (${PROJECT_NAME} PRIVATE ${OPENAL_SOFT_DIR}/include)
target_include_directories (${PROJECT_NAME} PRIVATE ${ALURE_DIR}/include)


# ========================
# FFmpeg 基础配置
# ========================
set(FFMPEG_ROOT "${3RD_DIR}/ffmpeg")
message(STATUS "FFmpeg路径: ${FFMPEG_ROOT}")

# 头文件目录
find_path(FFMPEG_INCLUDE_DIR
    libavcodec/avcodec.h
    PATHS ${FFMPEG_ROOT}/include
    REQUIRED
    NO_DEFAULT_PATH
)
message(STATUS "FFmpeg头文件目录: ${FFMPEG_INCLUDE_DIR}")

# ========================
# 项目配置
# ========================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${FFMPEG_INCLUDE_DIR}
)
target_link_directories(${PROJECT_NAME} PRIVATE
    ${FFMPEG_ROOT}/lib
)
# 直接链接FFmpeg库（按依赖顺序）
target_link_libraries(${PROJECT_NAME} PRIVATE
    avformat
    avcodec
    swscale
    swresample
    avutil  # 基础库必须放在最后
)

# ========================
# Windows特定处理
# ========================
if(WIN32)
    # 自动复制必要的DLL
    set(FFMPEG_DLLS
        avcodec-61.dll
        avformat-61.dll
        swscale-8.dll
        avutil-59.dll
        swresample-5.dll
    )
    
    foreach(DLL ${FFMPEG_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${FFMPEG_ROOT}/bin/${DLL}"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "复制: ${DLL}"
        )
    endforeach()
endif()


# Windows 特定设置
if(WIN32)
    # 设置 Unicode 编译选项
    target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE UNICODE)
endif()