
cmake_minimum_required(VERSION 3.4.1)

#引入子目录下的CMakeLists.txt
add_subdirectory(librtmp)

#导入头文件的路径
include_directories(librtmp libx264/include libfaac/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libx264/libs/${CMAKE_ANDROID_ARCH_ABI}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libfaac/libs/${CMAKE_ANDROID_ARCH_ABI}")

add_library(
        native-lib
        SHARED
        native-lib.cpp
        VideoChannel.cpp
        AudioChannel.cpp
)


target_link_libraries(
        native-lib
        rtmp
        faac
        x264
        log
)