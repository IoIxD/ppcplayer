# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libmp3lame"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libmp3lame/src/libmp3lame-build"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libmp3lame"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libmp3lame/tmp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libmp3lame/src/libmp3lame-stamp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libmp3lame/src"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libmp3lame/src/libmp3lame-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libmp3lame/src/libmp3lame-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libmp3lame/src/libmp3lame-stamp${cfgdir}") # cfgdir has leading slash
endif()
