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
    add_application(os9player ${sources} ${data})
else()
    add_executable(os9player ${sources} ${data})
endif()

set_property(TARGET os9player PROPERTY CXX_STANDARD 20)

set_target_properties(os9player PROPERTIES COMPILE_FLAGS "-ffunction-sections -mcpu=601 -O3 -Wall -Wextra -Wno-unused-parameter")
set_target_properties(os9player PROPERTIES LINK_FLAGS "-Wl,-gc-sections")
target_link_libraries( os9player OpenGL)
 
message(CMAKE_C_COMPILER ": " ${CMAKE_C_COMPILER})
message(CMAKE_CXX_COMPILER ": " ${CMAKE_CXX_COMPILER})

# Retro68: compile ffmpeg and link against the OpenGL stub.
if(CMAKE_SYSTEM_NAME MATCHES Retro)

    file(GLOB_RECURSE sources   
        platform/apple/**/**/**/*.c platform/apple/**/**/**/*.h platform/apple/**/**/**/*.cpp platform/apple/**/**/**/*.hpp 
        platform/apple/**/**/*.c platform/apple/**/**/*.h platform/apple/**/**/*.cpp platform/apple/**/**/*.hpp 
        platform/apple/**/*.c platform/apple/**/*.h platform/apple/**/*.cpp platform/apple/**/*.hpp 
        platform/apple/*.c platform/apple/*.h platform/apple/*.cpp platform/apple/*.hpp
        platform/apple/*.r platform/apple/**/*.r platform/apple/**/**/*.r platform/apple/**/**/**/*.r platform/apple/**/**/**/**/*.r
    )
    include_directories(${CMAKE_SOURCE_DIR}/platform/apple)
    add_definitions(__RETRO68__)

    include_directories(${CMAKE_SOURCE_DIR}/vendored/FFmpeg)
    target_link_directories(os9player PUBLIC ${CMAKE_SOURCE_DIR}/lib/lib/)
    include(ExternalProject)
    ExternalProject_Add(
        libff
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/vendored/FFmpeg
        CONFIGURE_COMMAND ${CMAKE_SOURCE_DIR}/vendored/FFmpeg/configure --prefix=${CMAKE_SOURCE_DIR}/lib --arch=ppc32 --target-os=none --cross-prefix=powerpc-apple-macos- --enable-cross-compile --disable-pthreads --disable-altivec --enable-lasx
        PREFIX ${CMAKE_SOURCE_DIR}/vendored/FFmpeg
        BUILD_COMMAND make -j$(nproc)
        BUILD_IN_SOURCE 1
    )

    add_dependencies(os9player libff)

    target_link_libraries(os9player avcodec)
    target_link_libraries(os9player avdevice)
    target_link_libraries(os9player avfilter)
    target_link_libraries(os9player avformat)
    target_link_libraries(os9player avutil)
    target_link_libraries(os9player m)
    target_link_libraries(os9player avcodec)
    target_link_libraries(os9player swresample)
    target_link_libraries(os9player swscale)

    include_directories(${CMAKE_SOURCE_DIR}/vendored/include)
    link_directories(${CMAKE_SOURCE_DIR}/vendored/lib)
    
    target_link_libraries(os9player ${CMAKE_SOURCE_DIR}/vendored/lib/libsdl.a)
    target_link_libraries(os9player ${CMAKE_SOURCE_DIR}/vendored/lib/libgl.a)
    target_link_libraries(os9player ${CMAKE_SOURCE_DIR}/vendored/lib/libglmem.a)
    target_link_libraries(os9player ${CMAKE_SOURCE_DIR}/vendored/lib/libglutil.a)
    target_link_libraries(os9player ContextualMenu)
    target_link_libraries(os9player DrawSprocketLib)
else() # Linux: just get it via the package manager.
    find_package(OpenGL REQUIRED)
    find_package(GLUT REQUIRED)
    find_package(OpenAL REQUIRED)

    include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )
    include_directories( ${OPENAL_INCLUDE_DIR}  )
    link_directories( ${OPENAL_DIRECTORY})

    target_link_libraries(${PROJECT_NAME} OpenGL::GL GLUT::GLUT m OpenAL::OpenAL)

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

    target_link_libraries(${PROJECT_NAME}
        PkgConfig::LIBAV
    )

endif()
include_directories( "${RETRO68_TOOLCHAIN}universal/CIncludes/")
message(STATUS "Making for ${CMAKE_SYSTEM_NAME}")
if(CMAKE_SYSTEM_NAME MATCHES Retro68)
set_target_properties(os9player PROPERTIES LINK_FLAGS "-Wl,--mac-strip-macsbug")
endif()

# this copies all resource files in the build directory
# we need this, because we want to work with paths relative to the executable
file(COPY ${data} DESTINATION resources)