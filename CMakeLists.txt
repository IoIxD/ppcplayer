cmake_minimum_required(VERSION 3.0)

project(os9player LANGUAGES C CXX)




###############################################################################
## file globbing ##############################################################
###############################################################################

# these instructions search the directory tree when CMake is
# invoked and put all files that match the pattern in the variables
# `sources` and `data`
file(GLOB_RECURSE sources   
src/**/**/**/*.c src/**/**/**/*.h src/**/**/**/*.cpp src/**/**/**/*.hpp 
src/**/**/*.c src/**/**/*.h src/**/**/*.cpp src/**/**/*.hpp 
src/**/*.c src/**/*.h src/**/*.cpp src/**/*.hpp 
src/*.c src/*.h src/*.cpp src/*.hpp

)
file(GLOB_RECURSE data resources/*)
# you can use set(sources src/main.cpp) etc if you don't want to
# use globbing to find files automatically

###############################################################################
## target definitions #########################################################
###############################################################################

# add the data to the target, so it becomes visible in some IDE

if(CMAKE_SYSTEM_NAME MATCHES Retro)
add_application(os9player ${sources} ${data} CONSOLE)
else()
add_executable(os9player ${sources} ${data})
endif()

set_property(TARGET os9player PROPERTY CXX_STANDARD 20)

if(PLATFORM MATCHES retro68)
add_compile_definitions(FOR_68K)
endif()

if(PLATFORM MATCHES retroppc)
add_compile_definitions(FOR_PPC)
set_target_properties(os9player PROPERTIES COMPILE_FLAGS "-ffunction-sections -mcpu=601 -O3 -Wall -Wextra -Wno-unused-parameter")
set_target_properties(os9player PROPERTIES LINK_FLAGS "-Wl,-gc-sections")
target_link_libraries( os9player PowerMgrLib)
target_link_libraries( os9player NameRegistryLib)
target_link_libraries( os9player OpenGL)
endif()

include(ExternalProject)
 
message(CMAKE_C_COMPILER ": " ${CMAKE_C_COMPILER})
message(CMAKE_CXX_COMPILER ": " ${CMAKE_CXX_COMPILER})

include_directories(${CMAKE_SOURCE_DIR}/vendored/FFmpeg)
#include_directories(${CMAKE_SOURCE_DIR}/vendored/libass)
include(ExternalProject)

macro(add_vendored __lib__ __extra_args__)  
    ExternalProject_Add(
        lib${__lib__}
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/vendored/lib${__lib__}
        CONFIGURE_COMMAND libtoolize && aclocal && autoupdate && automake --add-missing && autoreconf -fvi && ${CMAKE_SOURCE_DIR}/vendored/lib${__lib__}/configure --prefix=${CMAKE_SOURCE_DIR}/lib ${__extra_args__} 
        PREFIX ${CMAKE_SOURCE_DIR}/vendored/lib${__lib__}
        BUILD_COMMAND make -j$(nproc)
        BUILD_IN_SOURCE 1
    )
    include_directories(${CMAKE_SOURCE_DIR}/vendored/lib${__lib__}/src)
    include_directories(${CMAKE_SOURCE_DIR}/vendored/lib${__lib__}/include)
    target_link_libraries(os9player ${__lib__} STATIC)
endmacro()

#add_subdirectory(vendored/gme)
#include_directories(BEFORE ${CMAKE_SOURCE_DIR}/vendored/gme)
# add_vendored(modplug "")
# add_vendored(mp3lame "")
#target_link_libraries(os9player gme)

#target_link_libraries(os9player xavs)
#add_vendored(vorbis true)
#add_vendored(x264 true)
#add_vendored(x265 true)
#add_vendored(cdio true)

ExternalProject_Add(
    libff
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/vendored/FFmpeg
    CONFIGURE_COMMAND ${CMAKE_SOURCE_DIR}/vendored/FFmpeg/configure --prefix=${CMAKE_SOURCE_DIR}/lib --arch=ppc32 --target-os=none --cross-prefix=powerpc-apple-macos- --enable-cross-compile --disable-pthreads --disable-altivec --enable-lasx
    PREFIX ${CMAKE_SOURCE_DIR}/vendored/FFmpeg
    BUILD_COMMAND make -j$(nproc)
    BUILD_IN_SOURCE 1
)

add_dependencies(os9player libff)
target_link_directories(os9player PUBLIC ${CMAKE_SOURCE_DIR}/lib/lib/)
target_link_libraries(os9player avcodec)
target_link_libraries(os9player avdevice)
target_link_libraries(os9player avfilter)
target_link_libraries(os9player avformat)
target_link_libraries(os9player avutil)
target_link_libraries(os9player m)
target_link_libraries(os9player avcodec)
target_link_libraries(os9player swresample)
target_link_libraries(os9player swscale)


include_directories( "${RETRO68_TOOLCHAIN}universal/CIncludes/")
message(STATUS "Making for ${CMAKE_SYSTEM_NAME}")
if(CMAKE_SYSTEM_NAME MATCHES Retro68)
set_target_properties(os9player PROPERTIES LINK_FLAGS "-Wl,--mac-strip-macsbug")
endif()

# this copies all resource files in the build directory
# we need this, because we want to work with paths relative to the executable
file(COPY ${data} DESTINATION resources)