# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libcodec2"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libcodec2/src/libcodec2-build"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libcodec2"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libcodec2/tmp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libcodec2/src/libcodec2-stamp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libcodec2/src"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libcodec2/src/libcodec2-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libcodec2/src/libcodec2-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libcodec2/src/libcodec2-stamp${cfgdir}") # cfgdir has leading slash
endif()
