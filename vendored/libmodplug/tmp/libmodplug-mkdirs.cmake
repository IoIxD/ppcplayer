# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libmodplug"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libmodplug/src/libmodplug-build"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libmodplug"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libmodplug/tmp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libmodplug/src/libmodplug-stamp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libmodplug/src"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libmodplug/src/libmodplug-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libmodplug/src/libmodplug-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libmodplug/src/libmodplug-stamp${cfgdir}") # cfgdir has leading slash
endif()
