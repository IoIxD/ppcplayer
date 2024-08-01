# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libxavs"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libxavs/src/libxavs-build"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libxavs"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libxavs/tmp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libxavs/src/libxavs-stamp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libxavs/src"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libxavs/src/libxavs-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libxavs/src/libxavs-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libxavs/src/libxavs-stamp${cfgdir}") # cfgdir has leading slash
endif()
