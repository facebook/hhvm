# Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.
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

SET(TESTS
  allocator
  cell_calculator
  cell
  temptable-handler
  result
)

SET(ALL_TEMPTABLE_TESTS)
FOREACH(test ${TESTS})
  LIST(APPEND ALL_TEMPTABLE_TESTS ${test}-t.cc)
ENDFOREACH()

IF(WIN32)
  LIST(APPEND ALL_TEMPTABLE_TESTS ../../../sql/nt_servc.cc)
ENDIF()
MYSQL_ADD_EXECUTABLE(merge_temptable_tests-t ${ALL_TEMPTABLE_TESTS}
  ENABLE_EXPORTS
  EXCLUDE_ON_SOLARIS
  ADD_TEST merge_temptable_tests-t)

TARGET_LINK_LIBRARIES(merge_temptable_tests-t
  gunit_large
  server_unittest_library
  )

# Download TBB from https://www.threadingbuildingblocks.org
#SET(TBB_INCDIR /path/to/tbb43_20150611oss/include)
#SET(TBB_LIBDIR /path/to/tbb43_20150611oss/build/linux_intel64_gcc_cc4.9.1_libc2.12_kernel2.6.39_release)
#TARGET_LINK_LIBRARIES(merge_temptable_tests-t tbb -L${TBB_LIBDIR} -Wl,-rpath=${TBB_LIBDIR})
#INCLUDE_DIRECTORIES(${TBB_INCDIR})

TARGET_LINK_LIBRARIES(merge_temptable_tests-t perfschema)

# Necessary to make sure that we can use the jemalloc API calls.
GET_TARGET_PROPERTY(PREV_LINK_FLAGS merge_temptable_tests-t LINK_FLAGS)
IF(NOT PREV_LINK_FLAGS)
  SET(PREV_LINK_FLAGS)
ENDIF()
SET_TARGET_PROPERTIES(merge_temptable_tests-t PROPERTIES LINK_FLAGS
"${PREV_LINK_FLAGS} ${WITH_MYSQLD_LDFLAGS}")

ADD_DEPENDENCIES(merge_temptable_tests-t GenError)

FOREACH(test ${TESTS})
  SET(SRC_FILES ${test}-t.cc)
  IF(WIN32)
    LIST(APPEND SRC_FILES ../../../sql/nt_servc.cc)
  ENDIF()

  MYSQL_ADD_EXECUTABLE(${test}-t ${SRC_FILES}
    ENABLE_EXPORTS SKIP_INSTALL EXCLUDE_FROM_ALL)

  TARGET_LINK_LIBRARIES(${test}-t
    gunit_large
    server_unittest_library
    )

  # Necessary to make sure that we can use the jemalloc API calls.
  GET_TARGET_PROPERTY(PREV_LINK_FLAGS ${test}-t LINK_FLAGS)
  IF(NOT PREV_LINK_FLAGS)
    SET(PREV_LINK_FLAGS)
  ENDIF()
  SET_TARGET_PROPERTIES(${test}-t PROPERTIES LINK_FLAGS
  "${PREV_LINK_FLAGS} ${WITH_MYSQLD_LDFLAGS}")

  ADD_DEPENDENCIES(${test}-t GenError)

ENDFOREACH()

