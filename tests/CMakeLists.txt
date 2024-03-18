# tests/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 引入上级CMakeLists.txt中的项目名称与版本设置
set(PROJECT_NAME ServerProject)
project(${PROJECT_NAME}Tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加对主项目的依赖，包括它的头文件和库
include_directories(${PROJECT_SOURCE_DIR}/include)
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCLISTS)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()


add_executable(ThreadPool_test ThreadPool_test.cpp)
add_executable(Factory_test HttpMessageFactory_test.cpp)

target_link_libraries(ThreadPool_test PRIVATE gtest)
target_link_libraries(Factory_test PRIVATE gtest PRIVATE FAC_LIBS)

include(GoogleTest)
gtest_discover_tests(ThreadPool_test Factory_test)
