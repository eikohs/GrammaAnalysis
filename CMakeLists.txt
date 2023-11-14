cmake_minimum_required(VERSION 2.8)

project(SyntaxAnalysis)

set(CMAKE_CXX_STANDARD 11) # 指定cpp版本

if(WIN32)
set(CMAKE_EXE_LINKER_FLAGS "-static") # windows下生成可运行的exe文件
ENDIF ()

if(UNIX)
add_compile_options(-std=c++11) # Linux, 设置c++版本
endif()


# headers
include_directories(${PROJECT_SOURCE_DIR}/include)

# 查找源文件目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)

# link
add_executable(gramma ${DIR_SRCS})