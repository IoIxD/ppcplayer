# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libxevd"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libxevd/src/libxevd-build"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libxevd"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libxevd/tmp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libxevd/src/libxevd-stamp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libxevd/src"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libxevd/src/libxevd-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libxevd/src/libxevd-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libxevd/src/libxevd-stamp${cfgdir}") # cfgdir has leading slash
endif()
