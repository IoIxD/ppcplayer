# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libopenmpt"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libopenmpt/src/libopenmpt-build"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libopenmpt"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libopenmpt/tmp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libopenmpt/src/libopenmpt-stamp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libopenmpt/src"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libopenmpt/src/libopenmpt-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libopenmpt/src/libopenmpt-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libopenmpt/src/libopenmpt-stamp${cfgdir}") # cfgdir has leading slash
endif()
