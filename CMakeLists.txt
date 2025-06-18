cmake_minimum_required(VERSION 3.10)

project(MonitorWall)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

file(GLOB SOURCES "src/*.cpp")
file(GLOB SOURCESCC "src/*.cc")
file(GLOB CLIENTSRC "test/*.cpp")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

# 查找 Protobuf 库
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
# 查找大华的库
set(LIBRARY_SEARCH_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
find_library(AVNETSDK avnetsdk PATHS ${LIBRARY_SEARCH_PATH} REQUIRED)
find_library(CRYPTO crypto PATHS ${LIBRARY_SEARCH_PATH})
find_library(DHCONFIGSDK dhconfigsdk PATHS ${LIBRARY_SEARCH_PATH})
find_library(DHNETSDK dhnetsdk PATHS ${LIBRARY_SEARCH_PATH})
find_library(SSL ssl PATHS ${LIBRARY_SEARCH_PATH})
find_library(STERAMCONVERTOR StreamConvertor PATHS ${LIBRARY_SEARCH_PATH})
# 添加线程支持
find_package(Threads REQUIRED) 

# 添加 server 可执行程序
add_executable(MonitorWallGRPCServer ${SOURCES} ${SOURCESCC})
# 添加对 protobuf 和 gRPC 库的链接
target_link_libraries(MonitorWallGRPCServer PRIVATE 
    gRPC::grpc++  # CMake 提供的目标库名称
    gRPC::grpc++_reflection  # CMake 提供的目标库名称
    protobuf::libprotobuf  # CMake 提供的目标库名称
    Threads::Threads
    ${AVNETSDK}
    ${CRYPTO}
    ${DHCONFIGSDK}
    ${DHNETSDK}
    ${SSL}
    ${STERAMCONVERTOR}
)

# 添加 client 可执行程序
add_executable(MonitorWallGRPCClient ${CLIENTSRC} ${SOURCESCC})
# 添加对 protobuf 和 gRPC 库的链接
target_link_libraries(MonitorWallGRPCClient PRIVATE
  gRPC::grpc++
  gRPC::grpc++_reflection
  protobuf::libprotobuf
  Threads::Threads
)