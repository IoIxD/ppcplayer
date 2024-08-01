# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdc1394"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdc1394/src/libdc1394-build"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdc1394"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdc1394/tmp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdc1394/src/libdc1394-stamp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdc1394/src"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdc1394/src/libdc1394-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libdc1394/src/libdc1394-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libdc1394/src/libdc1394-stamp${cfgdir}") # cfgdir has leading slash
endif()
