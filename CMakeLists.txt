cmake_minimum_required(VERSION 3.0)

project(ppcplayer LANGUAGES C CXX)




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
    src/*.r src/**/*.r src/**/**/*.r src/**/**/**/*.r src/**/**/**/**/*.r
)
file(GLOB_RECURSE data resources/*)
# you can use set(sources src/main.cpp) etc if you don't want to
# use globbing to find files automatically

###############################################################################
## target definitions #########################################################
###############################################################################

# add the data to the target, so it becomes visible in some IDE

if(CMAKE_SYSTEM_NAME MATCHES Retro)
    add_application(ppcplayer ${sources} ${data})
else()
    add_executable(ppcplayer ${sources} ${data})
endif()

set_property(TARGET ppcplayer PROPERTY CXX_STANDARD 20)


message(CMAKE_C_COMPILER ": " ${CMAKE_C_COMPILER})
message(CMAKE_CXX_COMPILER ": " ${CMAKE_CXX_COMPILER})

include(ExternalProject)



# Retro68: compile ffmpeg and link against the OpenGL stub.
if(CMAKE_SYSTEM_NAME MATCHES Retro)
    set_target_properties(ppcplayer PROPERTIES COMPILE_FLAGS "-ffunction-sections -g -mcpu=601 -O3 -Wall -Wextra -Wno-unused-parameter")
    set_target_properties(ppcplayer PROPERTIES LINK_FLAGS "-Wl,-gc-sections")

    file(GLOB_RECURSE sources   
        platform/apple/**/**/**/*.c platform/apple/**/**/**/*.h platform/apple/**/**/**/*.cpp platform/apple/**/**/**/*.hpp 
        platform/apple/**/**/*.c platform/apple/**/**/*.h platform/apple/**/**/*.cpp platform/apple/**/**/*.hpp 
        platform/apple/**/*.c platform/apple/**/*.h platform/apple/**/*.cpp platform/apple/**/*.hpp 
        platform/apple/*.c platform/apple/*.h platform/apple/*.cpp platform/apple/*.hpp
        platform/apple/*.r platform/apple/**/*.r platform/apple/**/**/*.r platform/apple/**/**/**/*.r platform/apple/**/**/**/**/*.r
    )
    
    include_directories(${CMAKE_SOURCE_DIR}/platform/apple)
    add_compile_definitions(__RETRO68__)
    add_compile_definitions(__VENDORED_PORTAUDIO__)

    include_directories(${CMAKE_SOURCE_DIR}/vendored/FFmpeg)
    target_link_directories(ppcplayer PUBLIC ${CMAKE_SOURCE_DIR}/lib/lib/)
    ExternalProject_Add(
        libff
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/vendored/FFmpeg
        CONFIGURE_COMMAND ${CMAKE_SOURCE_DIR}/vendored/FFmpeg/configure --prefix=${CMAKE_SOURCE_DIR}/lib --arch=ppc32 --target-os=none --cross-prefix=powerpc-apple-macos- --enable-cross-compile --disable-pthreads --disable-altivec --enable-lasx
        PREFIX ${CMAKE_SOURCE_DIR}/vendored/FFmpeg
        BUILD_COMMAND make -j$(nproc)
        BUILD_IN_SOURCE 1
    )

    add_dependencies(ppcplayer libff)
    link_directories(${CMAKE_SOURCE_DIR}/vendored/lib)

    target_link_libraries( ppcplayer OpenGL)
    target_link_libraries(ppcplayer avcodec)
    target_link_libraries(ppcplayer avdevice)
    target_link_libraries(ppcplayer avfilter)
    target_link_libraries(ppcplayer avformat)
    target_link_libraries(ppcplayer avutil)
    target_link_libraries(ppcplayer m)
    target_link_libraries(ppcplayer avcodec)
    target_link_libraries(ppcplayer swresample)
    target_link_libraries(ppcplayer swscale)

    include_directories(${CMAKE_SOURCE_DIR}/vendored/include)
    
    
    target_link_libraries(ppcplayer ${CMAKE_SOURCE_DIR}/vendored/lib/libgl.a)
    target_link_libraries(ppcplayer ${CMAKE_SOURCE_DIR}/vendored/lib/libglmem.a)
    target_link_libraries(ppcplayer ${CMAKE_SOURCE_DIR}/vendored/lib/libglutil.a)
    target_link_libraries(ppcplayer ContextualMenu)
    target_link_libraries(ppcplayer DrawSprocketLib)
    target_link_libraries(ppcplayer SoundLib)
else() # Linux: just get it via the package manager.
    find_package(OpenGL REQUIRED)
    find_package(GLUT REQUIRED)

    include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenGL::GL GLUT::GLUT m)

    ExternalProject_Add(freealut
        GIT_REPOSITORY "https://github.com/vancegroup/freealut"
        INSTALL_COMMAND cmake -E echo "Skipping install step."
    )
    ExternalProject_Add(pa
        PREFIX=${CMAKE_SOURCE_DIR}/lib
        GIT_REPOSITORY "https://github.com/PortAudio/portaudio"
        INSTALL_COMMAND cmake -E echo "Skipping install step."
        EXCLUDE_FROM_ALL
    )

    add_dependencies(ppcplayer freealut)
    add_dependencies(ppcplayer pa)
    target_link_libraries(${PROJECT_NAME} PUBLIC portaudio)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
        libavdevice
        libavfilter
        libavformat
        libavcodec
        libswresample
        libswscale
        libavutil
    )

    target_link_libraries(${PROJECT_NAME} PUBLIC
        PkgConfig::LIBAV
    )
endif()



include_directories( "${RETRO68_TOOLCHAIN}universal/CIncludes/")
message(STATUS "Making for ${CMAKE_SYSTEM_NAME}")


# this copies all resource files in the build directory
# we need this, because we want to work with paths relative to the executable
file(COPY ${data} DESTINATION resources)