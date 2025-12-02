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

# rtspvideo 库目录
set (RTSP_DIR ${ROOT_DIR}/rtspvideo)

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

# 包含 rtspvideo 的头，并链接其库（使用 DLL / 静态由 rtspvideo CMake 决定）
target_include_directories(${PROJECT_NAME} PRIVATE ${RTSP_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE rtspvideo)
#sdl2
# Try to find SDL2 and link for video playback if available
# If SDL2 sources exist under thirdparty, add as subdirectory to guarantee availability
# Guard against double-adding SDL2 (some top-level CMake may have already added it)
if (EXISTS "${3RD_DIR}/SDL2/CMakeLists.txt")
	if (NOT TARGET sdl-build-options)
		add_subdirectory(${3RD_DIR}/SDL2 ${BUILD_DIR}/SDL2)
	endif()
endif()

# Prefer CMake's FindPackage, but fall back to variables or targets from subdirectory
find_package(SDL2 QUIET)
if (SDL2_FOUND)
	message(STATUS "Found SDL2 via find_package: ${SDL2_INCLUDE_DIRS}")
	target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
	target_compile_definitions(${PROJECT_NAME} PRIVATE USE_SDL)
elseif(TARGET SDL2 OR TARGET SDL2-static OR TARGET SDL2main)
	message(STATUS "Found SDL2 as a built target; linking against available SDL target")
	if (TARGET SDL2)
		target_link_libraries(${PROJECT_NAME} PRIVATE SDL2)
	elseif (TARGET SDL2-static)
		target_link_libraries(${PROJECT_NAME} PRIVATE SDL2-static)
		if (TARGET SDL2main)
			target_link_libraries(${PROJECT_NAME} PRIVATE SDL2main)
		endif()
	elseif (TARGET SDL2main)
		target_link_libraries(${PROJECT_NAME} PRIVATE SDL2main)
	endif()
	target_compile_definitions(${PROJECT_NAME} PRIVATE USE_SDL)
else()
	message(STATUS "SDL2 not found; building without SDL video output")
endif()