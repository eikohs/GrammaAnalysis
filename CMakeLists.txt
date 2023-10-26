cmake_minimum_required(2.8)

project(GrammaAnalysis)
add_definitions(-std=c99)

# headers
include_directories(${PROJECT_SOURCE_DIR}/include)
# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

# link
add_executable(GrammaAnalysis ${DIR_SRCS})