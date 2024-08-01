# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdnav"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdnav/src/libdvdnav-build"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdnav"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdnav/tmp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdnav/src/libdvdnav-stamp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdnav/src"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdnav/src/libdvdnav-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdnav/src/libdvdnav-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdnav/src/libdvdnav-stamp${cfgdir}") # cfgdir has leading slash
endif()
