CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(libzip C)

INCLUDE(CheckFunctionExists)
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckTypeSize)
INCLUDE(CheckCSourceRuns)
INCLUDE(CheckCSourceCompiles)

# Checks

CHECK_FUNCTION_EXISTS(_close HAVE__CLOSE)
CHECK_FUNCTION_EXISTS(_dup HAVE__DUP)
CHECK_FUNCTION_EXISTS(_fdopen HAVE__FDOPEN)
CHECK_FUNCTION_EXISTS(_fileno HAVE__FILENO)
CHECK_FUNCTION_EXISTS(_open HAVE__OPEN)
CHECK_FUNCTION_EXISTS(_snprintf HAVE__SNPRINTF)
CHECK_FUNCTION_EXISTS(_strdup HAVE__STRDUP)
CHECK_FUNCTION_EXISTS(_stricmp HAVE__STRICMP)
CHECK_FUNCTION_EXISTS(fseeko HAVE_FSEEKO)
CHECK_FUNCTION_EXISTS(ftello HAVE_FTELLO)
CHECK_FUNCTION_EXISTS(open HAVE_OPEN)
CHECK_FUNCTION_EXISTS(mkstemp HAVE_MKSTEMP)
CHECK_FUNCTION_EXISTS(snprintf HAVE_SNPRINTF)
CHECK_FUNCTION_EXISTS(strcasecmp HAVE_STRCASECMP)
CHECK_FUNCTION_EXISTS(strdup HAVE_STRDUP)
CHECK_SYMBOL_EXISTS(MoveFileExA windows.h HAVE_MOVEFILEEXA)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)

CHECK_INCLUDE_FILES(sys/types.h HAVE_SYS_TYPES_H_LIBZIP)
CHECK_INCLUDE_FILES(inttypes.h HAVE_INTTYPES_H_LIBZIP)
CHECK_INCLUDE_FILES(stdint.h HAVE_STDINT_H_LIBZIP)
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

# write out config file
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake-zipconf.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/zipconf.h)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake-config.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

SET(LIBZIP_SOURCES
  zip_add.c
  zip_add_dir.c
  zip_add_entry.c
  zip_close.c
  zip_delete.c
  zip_dir_add.c
  zip_dirent.c
  zip_discard.c
  zip_entry.c
  zip_error.c
  zip_error_clear.c
  zip_error_get.c
  zip_error_get_sys_type.c
  zip_error_strerror.c
  zip_error_to_str.c
  zip_err_str.c
  zip_extra_field_api.c
  zip_extra_field.c
  zip_fclose.c
  zip_fdopen.c
  zip_file_add.c
  zip_file_error_clear.c
  zip_file_error_get.c
  zip_file_get_comment.c
  zip_file_get_external_attributes.c
  zip_file_get_offset.c
  zip_filerange_crc.c
  zip_file_rename.c
  zip_file_replace.c
  zip_file_set_comment.c
  zip_file_set_external_attributes.c
  zip_file_strerror.c
  zip_fopen.c
  zip_fopen_encrypted.c
  zip_fopen_index.c
  zip_fopen_index_encrypted.c
  zip_fread.c
  zip_get_archive_comment.c
  zip_get_archive_flag.c
  zip_get_compression_implementation.c
  zip_get_encryption_implementation.c
  zip_get_file_comment.c
  zip_get_name.c
  zip_get_num_entries.c
  zip_get_num_files.c
  zip_memdup.c
  zip_name_locate.c
  zip_new.c
  zip_open.c
  zip_rename.c
  zip_replace.c
  zip_set_archive_comment.c
  zip_set_archive_flag.c
  zip_set_default_password.c
  zip_set_file_comment.c
  zip_set_file_compression.c
  zip_set_name.c
  zip_source_buffer.c
  zip_source_close.c
  zip_source_crc.c
  zip_source_deflate.c
  zip_source_error.c
  zip_source_file.c
  zip_source_filep.c
  zip_source_free.c
  zip_source_function.c
  zip_source_layered.c
  zip_source_open.c
  zip_source_pkware.c
  zip_source_pop.c
  zip_source_read.c
  zip_source_stat.c
  zip_source_window.c
  zip_source_zip.c
  zip_source_zip_new.c
  zip_stat.c
  zip_stat_index.c
  zip_stat_init.c
  zip_strerror.c
  zip_string.c
  zip_unchange_all.c
  zip_unchange_archive.c
  zip_unchange.c
  zip_unchange_data.c
  zip_utf-8.c
)

CHECK_FUNCTION_EXISTS(mkstemp HAVE_MKSTEMP)
IF(NOT HAVE_MKSTEMP)
  SET(LIBZIP_EXTRA_FILES mkstemp.c)
ENDIF(NOT HAVE_MKSTEMP)

ADD_LIBRARY(zip_static STATIC ${LIBZIP_SOURCES} ${LIBZIP_EXTRA_FILES})
SET_TARGET_PROPERTIES(zip_static PROPERTIES OUTPUT_NAME zip)
TARGET_LINK_LIBRARIES(zip_static ${ZLIB_LIBRARY})
INSTALL(TARGETS zip_static DESTINATION lib)

ADD_LIBRARY(zip_shared SHARED ${LIBZIP_SOURCES} ${LIBZIP_EXTRA_FILES})
SET_TARGET_PROPERTIES(zip_shared PROPERTIES OUTPUT_NAME zip)
TARGET_LINK_LIBRARIES(zip_shared ${ZLIB_LIBRARY})
INSTALL(TARGETS zip_shared DESTINATION lib)

INSTALL(FILES zipconf.h zip.h DESTINATION include)
