# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libass"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libass/src/libass-build"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libass"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libass/tmp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libass/src/libass-stamp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libass/src"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libass/src/libass-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libass/src/libass-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libass/src/libass-stamp${cfgdir}") # cfgdir has leading slash
endif()
