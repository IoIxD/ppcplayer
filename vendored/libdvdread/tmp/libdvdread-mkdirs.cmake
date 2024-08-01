# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdread"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdread/src/libdvdread-build"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdread"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdread/tmp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdread/src/libdvdread-stamp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdread/src"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdread/src/libdvdread-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdread/src/libdvdread-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libdvdread/src/libdvdread-stamp${cfgdir}") # cfgdir has leading slash
endif()
