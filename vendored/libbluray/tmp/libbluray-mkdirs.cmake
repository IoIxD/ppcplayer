# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libbluray"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libbluray/src/libbluray-build"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libbluray"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libbluray/tmp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libbluray/src/libbluray-stamp"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libbluray/src"
  "/home/gavin/Projects/os9ffmpegplayer/vendored/libbluray/src/libbluray-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libbluray/src/libbluray-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gavin/Projects/os9ffmpegplayer/vendored/libbluray/src/libbluray-stamp${cfgdir}") # cfgdir has leading slash
endif()
