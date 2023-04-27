# Copyright (c) 2010, 2020, Oracle and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

IF(NOT GMOCK_FOUND)
  RETURN()
ENDIF()

# Some tests require Boost.
INCLUDE_DIRECTORIES(SYSTEM ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})

INCLUDE_DIRECTORIES(SYSTEM ${GMOCK_INCLUDE_DIRS})

MY_CHECK_CXX_COMPILER_FLAG("-fno-builtin-memcmp" HAVE_NO_BUILTIN_MEMCMP)

SET(DISABLE_PSI_DEFINITIONS
  "DISABLE_PSI_COND"
  "DISABLE_PSI_FILE"
  "DISABLE_PSI_METADATA"
  "DISABLE_PSI_MUTEX"
  "DISABLE_PSI_RWLOCK"
  "DISABLE_PSI_STAGE"
  )

# Set these in the cache to make them available globally
SET(DISABLE_PSI_DEFINITIONS ${DISABLE_PSI_DEFINITIONS} CACHE INTERNAL "")

# Set up valgrind options to use with ctest.
# Use `ctest (...) -D ExperimentalMemCheck` to run.
IF(LINUX OR FREEBSD)
  FIND_PROGRAM(MEMORYCHECK_COMMAND valgrind)
  SET(MEMORYCHECK_COMMAND_OPTIONS
    "--leak-check=full --errors-for-leak-kinds=definite,indirect --error-exitcode=42"
    CACHE INTERNAL "")
  SET(MEMORYCHECK_SUPPRESSIONS_FILE "${PROJECT_SOURCE_DIR}/mysql-test/valgrind.supp"
    CACHE INTERNAL "")
ENDIF()

MY_INCLUDE_SYSTEM_DIRECTORIES(ICU)

# main-wrapper libraries (with tap-compatible option).
ADD_LIBRARY(gunit_small STATIC
  benchmark.cc
  fake_costmodel.cc
  gunit_test_main.cc
  skip_trailing.cc
  strnxfrm.cc
  tap_event_listener.cc
  thread_utils.cc
  fake_table.cc
  fake_mdl.cc
)
SET_TARGET_PROPERTIES(gunit_small
  PROPERTIES COMPILE_DEFINITIONS "${DISABLE_PSI_DEFINITIONS}"
)

# Several "small" unit tests got 'missing symbol client_errors'
# When building with Sun Studio
ADD_LIBRARY(libmysql_errmsg STATIC
  ${CMAKE_SOURCE_DIR}/libmysql/errmsg.cc
)
SET_TARGET_PROPERTIES(libmysql_errmsg
  PROPERTIES COMPILE_DEFINITIONS "${DISABLE_PSI_DEFINITIONS}"
)

ADD_LIBRARY(gunit_large STATIC
  benchmark.cc
  gunit_test_main_server.cc
  tap_event_listener.cc
  test_utils.cc
  thread_utils.cc
)
ADD_DEPENDENCIES(gunit_small GenError)
ADD_DEPENDENCIES(gunit_large GenError)
TARGET_LINK_LIBRARIES(gunit_small
  mysys strings ${GTEST_LIBRARIES} libmysql_errmsg)
TARGET_LINK_LIBRARIES(gunit_large
  mysys strings ${GTEST_LIBRARIES})
MESSAGE(STATUS "GTEST_LIBRARIES:${GTEST_LIBRARIES}")

IF(WIN32)
  ## Create an object library of nt_servc.cc, so we don't have to
  ## compile it again for each target
  ADD_LIBRARY(my_nt_servc OBJECT ../../sql/nt_servc.cc)
ENDIF()

# Add some defines.
ADD_DEFINITIONS(-DMYSQL_SERVER -DEXTRA_CODE_FOR_UNIT_TESTING)
ADD_DEFINITIONS(-DERRMSG_DIR="${PROJECT_BINARY_DIR}/share")
ADD_DEFINITIONS(-DDATA_DIR="${CMAKE_CURRENT_BINARY_DIR}")

# Add tests (link them with gunit/gmock libraries)
SET(TESTS
  alignment
  bounds_checked_array
  bitmap
  charset_bug28956360
  byteorder
  calloc
  cost_estimate
  dbug
  decimal
  dynarray
  filesort_buffer
  filesort_compare
  float_compare
  inplace_vector
  integer_digits
  intrusive_list_iterator
  key
  like_range
  m_string
  mdl
  my_bitmap
  my_error
  my_fileutils
  my_murmur3
  my_thread
  mysys_base64
  mysys_lf
  mysys_my_b_vprintf
  mysys_my_getopt
  mysys_my_getpw
  mysys_my_loadpath
  mysys_my_malloc
  mysys_my_pwrite
  mysys_my_rdtsc
  mysys_my_symlink
  mysys_my_time
  mysys_my_write
  mysys_my_read
  mysys_pathfuncs
  nullable
  opt_recperkey
  partitioned_rwlock
  pattern_matcher
  prealloced_array
  priority_queue
  record_buffer
  stream_cipher
  sql_class_header
  sql_list
  sql_plist
  sql_string
  stl_alloc
  strings_skip_trailing
  strings_strnxfrm
  strings_utf8
  strings_valid_check
  strtoll
  thread_utils
  my_timer
  template_utils
  timespec
  my_alloc
  pump_object_filter
  val_int_compare
  varlen_sort
  collation_loader
  my_rcu_lock
  fast_int2str
  )

SET(ALL_SMALL_TESTS)
FOREACH(test ${TESTS})
  LIST(APPEND ALL_SMALL_TESTS ${test}-t.cc)
ENDFOREACH()

# Add tests (link them with gunit/gmock libraries and the server libraries)
SET(SERVER_TESTS
  character_set_deprecation
  copy_info
  create_field
  dd_cache
  dd_column_statistics
  dd_info_schema_native_func
  dd_properties
  dd_schema
  dd_sdi
  dd_string_type
  dd_table
  debug_sync
  explain_filename
  field
  get_diagnostics
  gis_algos
  gis_area
  gis_distance
  gis_geometries
  gis_is_simple
  gis_isvalid
  gis_relops
  gis_rtree_support
  gis_srs
  gis_wkb_parser
  gis_wkb_writer
  handler
  hash_join
  histograms
  initialize_password
  insert_delayed
  into_syntax
  item
  item_filter
  item_func_case
  item_func_now_local
  item_json_func
  item_func_regexp
  item_like
  item_timefunc
  join_syntax
  join_tab_sort
  json_binary
  json_dom
  json_path
  locking_clause_syntax
  locking_service
  log_timestamp
  log_throttle
  make_sortkey
  mdl_sync
  my_decimal
  mysqld_funcs
  opt_costconstants
  opt_costmodel
  opt_guessrecperkey
  opt_range
  opt_ref
  opt_trace
  protocol_classic
  regexp_engine
  regexp_facade
  security_context
  segfault
  select_lex_visitor
  sql_table
  subquery_syntax
  table_cache
  table_list
  table_factor_syntax
  tc_log_mmap
  temptable_allocator
  temptable_storage
  thd_manager
  union_syntax
  unique
  value_map
  wild_case_compare
  sha2_password
  decoy_user
)

IF(WIN32)
  LIST(APPEND SERVER_TESTS win_tests)
ENDIF()

# Suppress -Wformat-overflow= for my_safe_snprintf(" %s ", nullptr)
MY_CHECK_CXX_COMPILER_WARNING("format-overflow" HAS_WARN_FLAG)
IF(HAS_WARN_FLAG)
  ADD_COMPILE_FLAGS(segfault-t.cc COMPILE_FLAGS ${HAS_WARN_FLAG})
ENDIF()

# Warnings about missing PGO profile data are not useful for unit tests.
DISABLE_MISSING_PROFILE_WARNING()

SET(ALL_LARGE_TESTS)
FOREACH(test ${SERVER_TESTS})
  LIST(APPEND ALL_LARGE_TESTS ${test}-t.cc)
ENDFOREACH()

SET(SQL_GUNIT_LIB_SOURCE
  ${CMAKE_SOURCE_DIR}/sql/filesort_utils.cc
  ${CMAKE_SOURCE_DIR}/sql/mdl.cc
  ${CMAKE_SOURCE_DIR}/sql/stream_cipher.cc
  ${CMAKE_SOURCE_DIR}/sql/sql_list.cc
  ${CMAKE_SOURCE_DIR}/sql/stateless_allocator.cc
  ${CMAKE_SOURCE_DIR}/sql-common/sql_string.cc
  ${CMAKE_SOURCE_DIR}/sql/thr_malloc.cc
  )

ADD_LIBRARY(sqlgunitlib STATIC ${SQL_GUNIT_LIB_SOURCE})
ADD_DEPENDENCIES(sqlgunitlib GenError)
SET_TARGET_PROPERTIES(sqlgunitlib
  PROPERTIES COMPILE_DEFINITIONS "${DISABLE_PSI_DEFINITIONS}"
)

MYSQL_ADD_EXECUTABLE(merge_small_tests-t ${ALL_SMALL_TESTS}
  ENABLE_EXPORTS
  ADD_TEST merge_small_tests)
TARGET_LINK_LIBRARIES(merge_small_tests-t
  sqlgunitlib gunit_small)

IF(WIN32)
  LIST(APPEND ALL_LARGE_TESTS ../../sql/nt_servc.cc)
ENDIF()
LIST(APPEND ALL_LARGE_TESTS ../../storage/example/ha_example.cc)

MYSQL_ADD_EXECUTABLE(merge_large_tests-t ${ALL_LARGE_TESTS}
  ENABLE_EXPORTS
  EXCLUDE_ON_SOLARIS
  ADD_TEST merge_large_tests)

TARGET_LINK_LIBRARIES(merge_large_tests-t
  gunit_large
  server_unittest_library
  )

# Necessary to make sure that we can use the jemalloc API calls.
GET_TARGET_PROPERTY(PREV_LINK_FLAGS merge_large_tests-t LINK_FLAGS)
IF(NOT PREV_LINK_FLAGS)
  SET(PREV_LINK_FLAGS)
ENDIF()
SET_TARGET_PROPERTIES(merge_large_tests-t PROPERTIES LINK_FLAGS
"${PREV_LINK_FLAGS} ${WITH_MYSQLD_LDFLAGS}")

FOREACH(test ${TESTS})
  MYSQL_ADD_EXECUTABLE(${test}-t ${test}-t.cc
    ENABLE_EXPORTS SKIP_INSTALL EXCLUDE_FROM_ALL)
  ADD_COMPILE_DEFINITIONS(${test}-t.cc
    COMPILE_DEFINITIONS ${DISABLE_PSI_DEFINITIONS})
  TARGET_LINK_LIBRARIES(${test}-t sqlgunitlib gunit_small)
ENDFOREACH()

# See comments about __builtin_memcmp in the source file.
IF(HAVE_NO_BUILTIN_MEMCMP)
  ADD_COMPILE_FLAGS(
    filesort_compare-t.cc
    COMPILE_FLAGS "-fno-builtin-memcmp"
    )
ENDIF()

FOREACH(test ${SERVER_TESTS})
  SET(SRC_FILES ${test}-t.cc)
  IF(WIN32)
    LIST(APPEND SRC_FILES $<TARGET_OBJECTS:my_nt_servc>)
  ENDIF()
  IF(test MATCHES "table_cache")
    LIST(APPEND SRC_FILES ../../storage/example/ha_example.cc)
  ENDIF()
  MYSQL_ADD_EXECUTABLE(${test}-t ${SRC_FILES}
    ENABLE_EXPORTS SKIP_INSTALL EXCLUDE_FROM_ALL)

  # Necessary to make sure that we can use the jemalloc API calls.
  GET_TARGET_PROPERTY(PREV_LINK_FLAGS ${test}-t LINK_FLAGS)
  IF(NOT PREV_LINK_FLAGS)
    SET(PREV_LINK_FLAGS)
  ENDIF()
  SET_TARGET_PROPERTIES(${test}-t PROPERTIES LINK_FLAGS
  "${PREV_LINK_FLAGS} ${WITH_MYSQLD_LDFLAGS}")

  TARGET_LINK_LIBRARIES(${test}-t
    gunit_large
    server_unittest_library
    )
ENDFOREACH()

ADD_SUBDIRECTORY(ddl_rewriter)
ADD_SUBDIRECTORY(innodb)
ADD_SUBDIRECTORY(keyring)
ADD_SUBDIRECTORY(components/mysql_server)
ADD_SUBDIRECTORY(xplugin)
ADD_SUBDIRECTORY(group_replication)
ADD_SUBDIRECTORY(libmysqlgcs)
ADD_SUBDIRECTORY(temptable)
ADD_SUBDIRECTORY(binlogevents)
