# version required
cmake_minimum_required(VERSION 3.20)
# project(MyIPC)

project(example)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 减小生成可执行文件的体积
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11 -Os -Wall" )
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=gnu++11 -Os" )
  
#启用调试
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11 -g")

# 编译命令:
# cmake -DCMAKE_C_COMPILER=mips-linux-uclibc-gnu-gcc -DCMAKE_CXX_COMPILER=mips-linux-uclibc-gnu-g++ ..

# set the include files 
set(PRO_CODE_INC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        )
include_directories(${PRO_CODE_INC})
  

# set the source files 
file(GLOB_RECURSE PRO_CODE_SRC  
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ) 


add_definitions(-DNO_STD_LIB -DNO_OPENSSL )


### build exe example
add_executable(${PROJECT_NAME} ${PRO_CODE_SRC})

# #---------------- ADD LIBS ---------------#

# set(LIVE555_LIB
#         ${CMAKE_CURRENT_SOURCE_DIR}/../liveMedia/libliveMedia.a
#         ${CMAKE_CURRENT_SOURCE_DIR}/../groupsock/libgroupsock.a
#         ${CMAKE_CURRENT_SOURCE_DIR}/../BasicUsageEnvironment/libBasicUsageEnvironment.a
#         ${CMAKE_CURRENT_SOURCE_DIR}/../UsageEnvironment/libUsageEnvironment.a
#         CACHE INTERNAL "Test Str")

# # target_include_directories(${PROJECT_NAME} PUBLIC ${LIVE555_INCLUDE})
# target_link_libraries(${PROJECT_NAME} ${LIVE555_LIB})

set(LIVE555_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../release/live555-mips-uclibc)

set(LIVE555_INCLUDE ${LIVE555_ROOT_PATH}/include
                    ${LIVE555_ROOT_PATH}/include/BasicUsageEnvironment
                    ${LIVE555_ROOT_PATH}/include/groupsock
                    ${LIVE555_ROOT_PATH}/include/liveMedia
                    ${LIVE555_ROOT_PATH}/include/UsageEnvironment CACHE INTERNAL "Test Str")
set(LIVE555_LIB
        ${LIVE555_ROOT_PATH}/lib/libliveMedia.a
        ${LIVE555_ROOT_PATH}/lib/libgroupsock.a
        ${LIVE555_ROOT_PATH}/lib/libBasicUsageEnvironment.a
        ${LIVE555_ROOT_PATH}/lib/libUsageEnvironment.a
        CACHE INTERNAL "Test Str")

target_include_directories(${PROJECT_NAME} PUBLIC ${LIVE555_INCLUDE})
target_link_libraries(${PROJECT_NAME} ${LIVE555_LIB})

# #---------------- ADD LIBS ---------------#


target_link_libraries(${PROJECT_NAME} pthread rt dl atomic m)
