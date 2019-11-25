# TODO:
# 1. pkgconfig file not yet built.
# 2. installation not tested.
# 3. Problems generating html pages.
# 4. create usable libtool .la file

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(libzip C)

INCLUDE(CheckFunctionExists)
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckTypeSize)
INCLUDE(CheckCSourceRuns)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckStructHasMember)

SET(PACKAGE "libzip")
SET(PACKAGE_NAME ${PACKAGE})
SET(PACKAGE_VERSION_MAJOR "1")
SET(PACKAGE_VERSION_MINOR "1")
SET(PACKAGE_VERSION_PATCH "3")
#SET(VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}")
SET(VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}")
SET(PACKAGE_VERSION ${VERSION})
SET(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")

# Checks

CHECK_FUNCTION_EXISTS(_close HAVE__CLOSE)
CHECK_FUNCTION_EXISTS(_dup HAVE__DUP)
CHECK_FUNCTION_EXISTS(_fdopen HAVE__FDOPEN)
CHECK_FUNCTION_EXISTS(_fileno HAVE__FILENO)
CHECK_FUNCTION_EXISTS(_open HAVE__OPEN)
CHECK_FUNCTION_EXISTS(_setmode HAVE__SETMODE)
CHECK_FUNCTION_EXISTS(_snprintf HAVE__SNPRINTF)
CHECK_FUNCTION_EXISTS(_strdup HAVE__STRDUP)
CHECK_FUNCTION_EXISTS(_stricmp HAVE__STRICMP)
CHECK_FUNCTION_EXISTS(fileno HAVE_FILENO)
CHECK_FUNCTION_EXISTS(fseeko HAVE_FSEEKO)
CHECK_FUNCTION_EXISTS(ftello HAVE_FTELLO)
CHECK_FUNCTION_EXISTS(getprogname HAVE_GETPROGNAME)
CHECK_FUNCTION_EXISTS(open HAVE_OPEN)
CHECK_FUNCTION_EXISTS(mkstemp HAVE_MKSTEMP)
CHECK_FUNCTION_EXISTS(setmode HAVE_SETMODE)
CHECK_FUNCTION_EXISTS(snprintf HAVE_SNPRINTF)
CHECK_FUNCTION_EXISTS(strcasecmp HAVE_STRCASECMP)
CHECK_FUNCTION_EXISTS(strdup HAVE_STRDUP)
CHECK_FUNCTION_EXISTS(stricmp HAVE_STRICMP)

CHECK_INCLUDE_FILES(fts.h HAVE_FTS_H)
CHECK_INCLUDE_FILES(stdbool.h HAVE_STDBOOL_H)
CHECK_INCLUDE_FILES(strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)

CHECK_INCLUDE_FILES(inttypes.h HAVE_INTTYPES_H_LIBZIP)
CHECK_INCLUDE_FILES(stdint.h HAVE_STDINT_H_LIBZIP)
CHECK_INCLUDE_FILES(sys/types.h HAVE_SYS_TYPES_H_LIBZIP)

# TODO: fix test
# this test does not find __progname even when it exists
#CHECK_SYMBOL_EXISTS(__progname stdlib.h HAVE___PROGNAME)

CHECK_TYPE_SIZE(__int8 __INT8_LIBZIP)
CHECK_TYPE_SIZE(int8_t INT8_T_LIBZIP)
CHECK_TYPE_SIZE(uint8_t UINT8_T_LIBZIP)
CHECK_TYPE_SIZE(__int16 __INT16_LIBZIP)
CHECK_TYPE_SIZE(int16_t INT16_T_LIBZIP)
CHECK_TYPE_SIZE(uint16_t UINT16_T_LIBZIP)
CHECK_TYPE_SIZE(__int32 __INT32_LIBZIP)
CHECK_TYPE_SIZE(int32_t INT32_T_LIBZIP)
CHECK_TYPE_SIZE(uint32_t UINT32_T_LIBZIP)
CHECK_TYPE_SIZE(__int64 __INT64_LIBZIP)
CHECK_TYPE_SIZE(int64_t INT64_T_LIBZIP)
CHECK_TYPE_SIZE(uint64_t UINT64_T_LIBZIP)
CHECK_TYPE_SIZE("short" SHORT_LIBZIP)
CHECK_TYPE_SIZE("int" INT_LIBZIP)
CHECK_TYPE_SIZE("long" LONG_LIBZIP)
CHECK_TYPE_SIZE("long long" LONG_LONG_LIBZIP)
CHECK_TYPE_SIZE("off_t" SIZEOF_OFF_T)
CHECK_TYPE_SIZE("size_t" SIZE_T_LIBZIP)
CHECK_TYPE_SIZE("ssize_t" SSIZE_T_LIBZIP)

FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
set(CMAKE_REQUIRED_INCLUDES ${ZLIB_INCLUDE_DIR})
IF(ZLIB_VERSION_STRING VERSION_LESS "1.1.2")
  MESSAGE(FATAL_ERROR "-- ZLIB version too old, please install at least v1.1.2")
ENDIF(ZLIB_VERSION_STRING VERSION_LESS "1.1.2")

IF(MSVC)
ADD_DEFINITIONS("-D_CRT_SECURE_NO_WARNINGS")
ENDIF(MSVC)

ADD_DEFINITIONS("-DHAVE_CONFIG_H")
ADD_DEFINITIONS("-DZIP_STATIC")
ADD_DEFINITIONS(-UPACKAGE -UPACKAGE_VERSION)

# TODO: pkgconfig file

# write out config file
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/cmake-config.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/config.h)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/cmake-zipconf.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/lib/zipconf.h)

SET(LIBZIP_SOURCES
  src/lib/zip_add.c
  src/lib/zip_add_dir.c
  src/lib/zip_add_entry.c
  src/lib/zip_buffer.c
  src/lib/zip_close.c
  src/lib/zip_delete.c
  src/lib/zip_dir_add.c
  src/lib/zip_dirent.c
  src/lib/zip_discard.c
  src/lib/zip_entry.c
  src/lib/zip_err_str.c
  src/lib/zip_error.c
  src/lib/zip_error_clear.c
  src/lib/zip_error_get.c
  src/lib/zip_error_get_sys_type.c
  src/lib/zip_error_strerror.c
  src/lib/zip_error_to_str.c
  src/lib/zip_extra_field.c
  src/lib/zip_extra_field_api.c
  src/lib/zip_fclose.c
  src/lib/zip_fdopen.c
  src/lib/zip_file_add.c
  src/lib/zip_file_error_clear.c
  src/lib/zip_file_error_get.c
  src/lib/zip_file_get_comment.c
  src/lib/zip_file_get_external_attributes.c
  src/lib/zip_file_get_offset.c
  src/lib/zip_file_rename.c
  src/lib/zip_file_replace.c
  src/lib/zip_file_set_comment.c
  src/lib/zip_file_set_external_attributes.c
  src/lib/zip_file_set_mtime.c
  src/lib/zip_file_strerror.c
  src/lib/zip_filerange_crc.c
  src/lib/zip_fopen.c
  src/lib/zip_fopen_encrypted.c
  src/lib/zip_fopen_index.c
  src/lib/zip_fopen_index_encrypted.c
  src/lib/zip_fread.c
  src/lib/zip_get_archive_comment.c
  src/lib/zip_get_archive_flag.c
  src/lib/zip_get_compression_implementation.c
  src/lib/zip_get_encryption_implementation.c
  src/lib/zip_get_file_comment.c
  src/lib/zip_get_name.c
  src/lib/zip_get_num_entries.c
  src/lib/zip_get_num_files.c
  src/lib/zip_hash.c
  src/lib/zip_io_util.c
  src/lib/zip_memdup.c
  src/lib/zip_name_locate.c
  src/lib/zip_new.c
  src/lib/zip_open.c
  src/lib/zip_rename.c
  src/lib/zip_replace.c
  src/lib/zip_set_archive_comment.c
  src/lib/zip_set_archive_flag.c
  src/lib/zip_set_default_password.c
  src/lib/zip_set_file_comment.c
  src/lib/zip_set_file_compression.c
  src/lib/zip_set_name.c
  src/lib/zip_source_begin_write.c
  src/lib/zip_source_buffer.c
  src/lib/zip_source_call.c
  src/lib/zip_source_close.c
  src/lib/zip_source_commit_write.c
  src/lib/zip_source_crc.c
  src/lib/zip_source_deflate.c
  src/lib/zip_source_error.c
  src/lib/zip_source_filep.c
  src/lib/zip_source_free.c
  src/lib/zip_source_function.c
  src/lib/zip_source_is_deleted.c
  src/lib/zip_source_layered.c
  src/lib/zip_source_open.c
  src/lib/zip_source_pkware.c
  src/lib/zip_source_read.c
  src/lib/zip_source_remove.c
  src/lib/zip_source_rollback_write.c
  src/lib/zip_source_seek.c
  src/lib/zip_source_seek_write.c
  src/lib/zip_source_stat.c
  src/lib/zip_source_supports.c
  src/lib/zip_source_tell.c
  src/lib/zip_source_tell_write.c
  src/lib/zip_source_window.c
  src/lib/zip_source_write.c
  src/lib/zip_source_zip.c
  src/lib/zip_source_zip_new.c
  src/lib/zip_stat.c
  src/lib/zip_stat_index.c
  src/lib/zip_stat_init.c
  src/lib/zip_strerror.c
  src/lib/zip_string.c
  src/lib/zip_unchange.c
  src/lib/zip_unchange_all.c
  src/lib/zip_unchange_archive.c
  src/lib/zip_unchange_data.c
  src/lib/zip_utf-8.c
)

IF(WIN32)
  SET(LIBZIP_OPSYS_FILES
    src/lib/zip_source_win32a.c
    src/lib/zip_source_win32handle.c
    src/lib/zip_source_win32utf8.c
    src/lib/zip_source_win32w.c
  )
ELSE(WIN32)
  SET(LIBZIP_OPSYS_FILES
    src/lib/zip_source_file.c
  )
ENDIF(WIN32)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/lib/
  ${CMAKE_CURRENT_BINARY_DIR}/src/..)

CHECK_FUNCTION_EXISTS(mkstemp HAVE_MKSTEMP)
IF(NOT HAVE_MKSTEMP)
  SET(LIBZIP_EXTRA_FILES src/lib/mkstemp.c)
ENDIF(NOT HAVE_MKSTEMP)

ADD_LIBRARY(zip_static STATIC ${LIBZIP_SOURCES} ${LIBZIP_EXTRA_FILES} ${LIBZIP_OPSYS_FILES})
TARGET_LINK_LIBRARIES(zip_static ${ZLIB_LIBRARY})
TARGET_COMPILE_DEFINITIONS(zip_static PUBLIC "ZIP_EXTERN=")
# To pick up zipconf.h
TARGET_INCLUDE_DIRECTORIES(zip_static PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src/lib)
