
cmake_minimum_required(VERSION 3.4.1)

file(GLOB rtmp_src *.c)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_CRYPTO") 依然编译错误
#如果纯C的环境就要使用 CMAKE_C_FLAGS
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_CRYPTO")

add_library(
        rtmp
        STATIC
        ${rtmp_src})

