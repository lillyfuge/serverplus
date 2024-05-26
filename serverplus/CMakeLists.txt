cmake_minimum_required(VERSION 3.10)

# 项目名称
project(EchoExample)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 分别设置echoserver和echoclient的源文件
#set(SOURCE_ECHOSERVER examples/echoserver.cpp)
set(SOURCE_ECHOCLIENT examples/echoclient.cpp)
set(SOURCE_COECHOSERVER examples/co_echoserver.cpp)

# 添加可执行文件目标echoserver
#add_executable(echoserver ${SOURCE_ECHOSERVER})

# 添加可执行文件目标echoclient
add_executable(echoclient ${SOURCE_ECHOCLIENT})


add_executable(co_echoserver ${SOURCE_COECHOSERVER})

# 设置输出路径使得可执行文件直接生成在CMakeLists.txt所在目录
#set_target_properties(echoserver PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(echoclient PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(co_echoserver PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")