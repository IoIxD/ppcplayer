# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/gavin/Projects/ppcplayer/vendored/FFmpeg"
  "/home/gavin/Projects/ppcplayer/vendored/FFmpeg/src/libff-build"
  "/home/gavin/Projects/ppcplayer/vendored/FFmpeg"
  "/home/gavin/Projects/ppcplayer/vendored/FFmpeg/tmp"
  "/home/gavin/Projects/ppcplayer/vendored/FFmpeg/src/libff-stamp"
  "/home/gavin/Projects/ppcplayer/vendored/FFmpeg/src"
  "/home/gavin/Projects/ppcplayer/vendored/FFmpeg/src/libff-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/gavin/Projects/ppcplayer/vendored/FFmpeg/src/libff-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/gavin/Projects/ppcplayer/vendored/FFmpeg/src/libff-stamp${cfgdir}") # cfgdir has leading slash
endif()
