cmake_minimum_required(VERSION 3.5.0)

project (ababisme)

if (TRUE)
#set(CMAKE_SYSTEM_NAME, Linux)
#set(CMAKE_SYSTEM_PROCESSOR aarch64)

add_compile_definitions(USE_SDL=1)
add_compile_definitions(GLIBCXX_USE_CXX11_ABI=0)

endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/projects/cmake")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
 # set(CMAKE_BUILD_TYPE "Debug")
endif()

#set(CMAKE_CXX_COMPILER "$ENV{CROSS}g++" CACHE PATH "" FORCE)
#set(CMAKE_C_COMPILER "$ENV{CROSS}gcc" CACHE PATH "" FORCE)
#set(CMAKE_SYSROOT "/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)

add_compile_options(-Wno-unused-parameter -Wno-missing-field-initializers
  -Wno-sign-compare -Wno-parentheses -Wno-unused-variable -Wno-narrowing
)

# add_compile_options(-g -O2 -W -Wall -Wextra)
add_compile_options(-g)
add_compile_options(
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-reorder>
  $<$<COMPILE_LANGUAGE:CXX>:-std=c++20>
)

include_directories(src)
include_directories(${SDL2_INCLUDE_DIR} ${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS})

set(SRC_ROOT "${CMAKE_SOURCE_DIR}/src")

file(GLOB SOURCES_FILES
  "${SRC_ROOT}/*.cpp"
  "${SRC_ROOT}/game/*.cpp"
  "${SRC_ROOT}/gfx/*.cpp"
 # "${SRC_ROOT}/gfx/backends/*.cpp"
  "${SRC_ROOT}/gfx/views/*.cpp"
  "${SRC_ROOT}/io/*.cpp"
  "${SRC_ROOT}/io/zip/miniz.cpp"
  "${SRC_ROOT}/utils/format/*.cpp"
  "${SRC_ROOT}/test/*.cpp"
)

set(SOURCES ${SOURCES_FILES})

add_executable(ababisme ${SOURCES})

target_link_libraries(ababisme ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES})
