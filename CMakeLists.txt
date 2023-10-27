cmake_minimum_required(VERSION 2.8)

project(GrammaAnalysis)
set(CMAKE_CXX_STANDARD 11) # 指定cpp版本

# headers
include_directories(${PROJECT_SOURCE_DIR}/include)

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)

# link
add_executable(gramma ${DIR_SRCS})