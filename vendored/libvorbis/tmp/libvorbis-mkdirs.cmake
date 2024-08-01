# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libvorbis"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libvorbis/src/libvorbis-build"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libvorbis"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libvorbis/tmp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libvorbis/src/libvorbis-stamp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libvorbis/src"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libvorbis/src/libvorbis-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libvorbis/src/libvorbis-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libvorbis/src/libvorbis-stamp${cfgdir}") # cfgdir has leading slash
endif()
