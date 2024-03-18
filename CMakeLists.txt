cmake_minimum_required(VERSION 3.10)

# 项目名称
project(ServerProject)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 指定源文件目录
aux_source_directory(src SRC_LIST) # 如果不使用aux_source_directory，也可以直接指定文件名


# 或者手动添加单个源文件
# set(SRC_LIST src/Server.cpp)

# 添加头文件目录到包含路径
include_directories(include)

# 添加可执行文件目标
add_executable(Server ${SRC_LIST})

set(FAC_LIB src/HttpMessageFactory.cpp)

add_library(FAC_LIBS ${FAC_LIB})
# 确保include目录对于Server目标可见
target_include_directories(Server PRIVATE include)

# 如果需要链接库，添加类似下面的语句
# target_link_libraries(Server ${REQUIRED_LIBRARIES})


# 允许用户选择是否构建测试
option(BUILD_TESTING "Build the unit tests." ON)

# 添加子目录，同时考虑是否要构建测试
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# 如果构建测试，则添加对GoogleTest的查找和安装（如果还未找到）
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})
endif()