cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0048 NEW)
project(libbluray VERSION 0.9.3 LANGUAGES C)


find_package(freetype 2.6 REQUIRED NO_MODULE)
find_package(libxml2 2.9 REQUIRED NO_MODULE)


set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(MSVC)
  set(CMAKE_DEBUG_POSTFIX "d")
endif()


set(SRCS    
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/headers_fixup.h
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/bluray-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/libbluray.def
  ${PROJECT_SOURCE_DIR}/file/dirs.h
  ${PROJECT_SOURCE_DIR}/file/dirs_win32.c
  ${PROJECT_SOURCE_DIR}/file/dir_win32.c
  ${PROJECT_SOURCE_DIR}/file/dl.h
  ${PROJECT_SOURCE_DIR}/file/dl_win32.c
  ${PROJECT_SOURCE_DIR}/file/file.c
  ${PROJECT_SOURCE_DIR}/file/file.h
  ${PROJECT_SOURCE_DIR}/file/filesystem.c
  ${PROJECT_SOURCE_DIR}/file/filesystem.h
  ${PROJECT_SOURCE_DIR}/file/file_win32.c
  ${PROJECT_SOURCE_DIR}/file/mount.c
  ${PROJECT_SOURCE_DIR}/file/mount.h
  ${PROJECT_SOURCE_DIR}/libbluray/bluray.c
  ${PROJECT_SOURCE_DIR}/libbluray/bluray.h
  ${PROJECT_SOURCE_DIR}/libbluray/bluray_internal.h
  ${PROJECT_SOURCE_DIR}/libbluray/keys.h
  ${PROJECT_SOURCE_DIR}/libbluray/player_settings.h
  ${PROJECT_SOURCE_DIR}/libbluray/register.c
  ${PROJECT_SOURCE_DIR}/libbluray/register.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/bdid_parse.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/bdid_parse.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/bdparse.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/clpi_data.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/clpi_parse.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/clpi_parse.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/extdata_parse.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/extdata_parse.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/index_parse.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/index_parse.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/meta_data.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/meta_parse.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/meta_parse.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/mpls_parse.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/mpls_parse.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/navigation.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/navigation.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/sound_parse.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/sound_parse.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdnav/uo_mask_table.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/graphics_controller.c
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/graphics_controller.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/graphics_processor.c
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/graphics_processor.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/hdmv_pids.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/ig.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/ig_decode.c
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/ig_decode.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/m2ts_demux.c
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/m2ts_demux.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/m2ts_filter.c
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/m2ts_filter.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/overlay.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/pes_buffer.c
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/pes_buffer.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/pg.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/pg_decode.c
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/pg_decode.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/rle.c
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/rle.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/textst.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/textst_decode.c
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/textst_decode.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/textst_render.c
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/textst_render.h
  ${PROJECT_SOURCE_DIR}/libbluray/disc/aacs.c
  ${PROJECT_SOURCE_DIR}/libbluray/disc/aacs.h
  ${PROJECT_SOURCE_DIR}/libbluray/disc/bdplus.c
  ${PROJECT_SOURCE_DIR}/libbluray/disc/bdplus.h
  ${PROJECT_SOURCE_DIR}/libbluray/disc/dec.c
  ${PROJECT_SOURCE_DIR}/libbluray/disc/dec.h
  ${PROJECT_SOURCE_DIR}/libbluray/disc/disc.c
  ${PROJECT_SOURCE_DIR}/libbluray/disc/disc.h
  ${PROJECT_SOURCE_DIR}/libbluray/disc/enc_info.h
  ${PROJECT_SOURCE_DIR}/libbluray/hdmv/hdmv_insn.h
  ${PROJECT_SOURCE_DIR}/libbluray/hdmv/hdmv_vm.c
  ${PROJECT_SOURCE_DIR}/libbluray/hdmv/hdmv_vm.h
  ${PROJECT_SOURCE_DIR}/libbluray/hdmv/mobj_data.h
  ${PROJECT_SOURCE_DIR}/libbluray/hdmv/mobj_parse.c
  ${PROJECT_SOURCE_DIR}/libbluray/hdmv/mobj_parse.h
  ${PROJECT_SOURCE_DIR}/libbluray/hdmv/mobj_print.c
  ${PROJECT_SOURCE_DIR}/libbluray/hdmv/mobj_print.h
  ${PROJECT_SOURCE_DIR}/util/array.c
  ${PROJECT_SOURCE_DIR}/util/array.h
  ${PROJECT_SOURCE_DIR}/util/attributes.h
  ${PROJECT_SOURCE_DIR}/util/bits.c
  ${PROJECT_SOURCE_DIR}/util/bits.h
  ${PROJECT_SOURCE_DIR}/util/event_queue.c
  ${PROJECT_SOURCE_DIR}/util/event_queue.h
  ${PROJECT_SOURCE_DIR}/util/logging.c
  ${PROJECT_SOURCE_DIR}/util/logging.h
  ${PROJECT_SOURCE_DIR}/util/log_control.h
  ${PROJECT_SOURCE_DIR}/util/macro.h
  ${PROJECT_SOURCE_DIR}/util/mutex.c
  ${PROJECT_SOURCE_DIR}/util/mutex.h
  ${PROJECT_SOURCE_DIR}/util/refcnt.c
  ${PROJECT_SOURCE_DIR}/util/refcnt.h
  ${PROJECT_SOURCE_DIR}/util/strutl.c
  ${PROJECT_SOURCE_DIR}/util/strutl.h
  ${PROJECT_SOURCE_DIR}/util/time.c
  ${PROJECT_SOURCE_DIR}/util/time.h
)

set(bdj_SRCS 
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/bdj.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/bdj.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/bdjo_data.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/bdjo_parse.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/bdjo_parse.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/native/bdjo.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/native/bdjo.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/native/java_awt_BDFontMetrics.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/native/java_awt_BDFontMetrics.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/native/java_awt_BDGraphics.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/native/java_awt_BDGraphics.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/native/org_videolan_Libbluray.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/native/org_videolan_Libbluray.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/native/org_videolan_Logger.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/native/org_videolan_Logger.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/native/register_native.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/native/register_native.h
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/native/util.c
  ${PROJECT_SOURCE_DIR}/libbluray/bdj/native/util.h
)


# BD-J

find_package(JNI)
if (JNI_FOUND)
  include_directories("${JAVA_INCLUDE_PATH}")
  include_directories("${JAVA_INCLUDE_PATH2}")
  set(HAVE_JNI_H 1)
endif()

find_package(Java COMPONENTS Development)
if (JAVA_FOUND)
  get_filename_component(_JDK_HOME "${Java_JAVA_EXECUTABLE}" PATH)
  get_filename_component(_JDK_HOME "${_JDK_HOME}/.." ABSOLUTE)
  message("JDK_HOME=${_JDK_HOME}")
endif()

if (JNI_FOUND AND JAVA_FOUND)
  set(USING_BDJAVA 1)
 
  ADD_LIBRARY(libbluray SHARED ${SRCS} ${bdj_SRCS})
  target_link_libraries(libbluray PRIVATE freetype::freetype)
  SET_TARGET_PROPERTIES(libbluray PROPERTIES LINK_FLAGS "/DEF:\"libbluray.def\"")
  
else()

ADD_LIBRARY(libbluray SHARED ${SRCS})
SET_TARGET_PROPERTIES(libbluray PROPERTIES LINK_FLAGS "/DEF:\"libbluray.def\"")

endif()

target_link_libraries(libbluray PRIVATE libxml2::libxml2)
target_compile_definitions(libbluray
  PRIVATE
  HAVE_CONFIG_H
  _CRT_SECURE_NO_WARNINGS)
target_include_directories(libbluray
  PRIVATE
  $<BUILD_INTERFACE:.;cmake;src;src/libbluray;${CMAKE_CURRENT_BINARY_DIR}>
  INTERFACE
  $<INSTALL_INTERFACE:include/libbluray>
)

if (EXISTS $ENV{ANT_HOME})
set(ANT_CMD $ENV{ANT_HOME}/bin/ant)
set(ANT_BUILD ${PROJECT_SOURCE_DIR}/libbluray/bdj/build.xml)
  # STRING(REGEX REPLACE "/" "\\\\" ANT_BUILD "${PROJECT_SOURCE_DIR}/libbluray/bdj/build.xml")
  message("ANT command: ${ANT_CMD} -f ${ANT_BUILD}")
  add_custom_target (ANT
     COMMENT "compiling libbluray.jar"
     COMMAND 
        ${ANT_CMD} -f "${ANT_BUILD}"
                  -Ddist="${CMAKE_CURRENT_BINARY_DIR}"
                  -Dsrc_awt=:java-j2se
                  -Dversion="j2se-0.9.3"
  )
  add_dependencies(libbluray ANT)
else()
  message("ANT path not set, can't compile libbluray.jar")
endif()

set(HAVE_FT2 1)
set(HAVE_LIBXML2 1)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cm ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/libbluray-config-version.cmake
  VERSION ${libbluray_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(TARGETS libbluray EXPORT libbluray
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib)


install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/bluray-version.h
  ${PROJECT_SOURCE_DIR}/libbluray/bluray.h
  ${PROJECT_SOURCE_DIR}/libbluray/player_settings.h
  ${PROJECT_SOURCE_DIR}/libbluray/keys.h
  ${PROJECT_SOURCE_DIR}/libbluray/decoders/overlay.h
  ${PROJECT_SOURCE_DIR}/file/filesystem.h
  ${PROJECT_SOURCE_DIR}/util/log_control.h
  DESTINATION include/libbluray)

install(FILES
  $<TARGET_PDB_FILE:libbluray>
  DESTINATION lib)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/libbluray-j2se-0.9.3.jar
  DESTINATION bin)

install(EXPORT libbluray
  FILE
    libbluray.cmake
  NAMESPACE
    libbluray::
  DESTINATION
    lib/cmake/libbluray
)
install(
  FILES
    cmake/libbluray-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libbluray-config-version.cmake
  DESTINATION
    lib/cmake/libbluray
)