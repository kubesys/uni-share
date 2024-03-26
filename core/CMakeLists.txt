cmake_minimum_required(VERSION 3.10)
project(MyLibrary VERSION 1.0.0)

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# 启用 fPIC 选项以便生成位置无关代码
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(LIBRARIES
    #cuda
    #cndev
    nvidia-ml
    pthread
)

# 包含的源文件目录
include_directories(
        container
        controller
        interceptor
        monitor
        /usr/local/neuware/include
        /usr/local/cuda/include
)

# 添加库搜索路径
link_directories(
    /usr/local/cuda/targets/x86_64-linux/lib
    /usr/local/neuware/lib64

)
# 定义要编译成库的源文件
file(GLOB_RECURSE SOURCES "controller/*.c" "interceptor/*.c" "monitor/*.c" "container/*.c")

# 指定生成动态链接库
add_library(mylibrary SHARED ${SOURCES})

# 链接外部库
target_link_libraries(mylibrary ${LIBRARIES})
