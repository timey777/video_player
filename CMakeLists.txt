cmake_minimum_required (VERSION 3.8)
set(PROJECT_NAME video_player)
project (${PROJECT_NAME})
set (${PROJECT_NAME}_VERSION_MAJOR 1)
set (${PROJECT_NAME}_VERSION_MINOR 0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set (3RD_DIR ${PROJECT_SOURCE_DIR}/thirdparty)
set (CMAKE_MODULE_PATH ${3RD_DIR})

function(source_group_by_dir)
    if(MSVC)
        set(source_files ${ARGV0})
        set(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR})
		if(${ARGC} EQUAL 2)
			set(sgbd_cur_dir ${ARGV1})
		endif()
        foreach(sgbd_file ${${source_files}})
            string(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
            string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
            string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
            string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
            if(sgbd_nogroup)
                set(sgbd_group_name "\\")
            endif(sgbd_nogroup)
            source_group(${sgbd_group_name} FILES ${sgbd_file})
        endforeach(sgbd_file)
    endif()
endfunction()

function(link_directory_by_cl dir)
	foreach(arg IN LISTS ARGN)
		IF(CMAKE_CL_64)
			link_directories(${dir}/${arg}/lib/Win64)
		ELSE(CMAKE_CL_64)
			link_directories(${dir}/${arg}/lib/Win32)
		ENDIF(CMAKE_CL_64)
	endforeach()
endfunction()

if (CMAKE_BUILD_TYPE)
	string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
endif()

########################################
# wxWidgets
# 设置wxWidgets选项
set(wxUSE_RIBBON        OFF CACHE BOOL "" FORCE)
set(wxUSE_RIBBON_NATIVE OFF CACHE BOOL "" FORCE)
set(wxUSE_PROPGRID OFF)
set(wxUSE_DEBUGREPORT OFF) 
set(wxBUILD_QA OFF) 
set(wxUSE_STC OFF)  # 禁用 Scintilla 组件
set(wxBUILD_SHARED OFF)
set(wxBUILD_TESTS OFF)
set (WXWIDGETS_DIR ${3RD_DIR}/wxWidgets)
if(WIN32)
add_subdirectory (${WXWIDGETS_DIR})
else()
find_package(wxWidgets)
endif()

########################################
add_subdirectory(rtsp_video_player)

