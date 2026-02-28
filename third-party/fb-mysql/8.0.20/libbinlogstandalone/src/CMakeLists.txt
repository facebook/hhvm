# Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.
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

SET(REPLICATION_SOURCES
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/binlog_event.cpp
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/codecs/binary.cpp
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/codecs/factory.cpp
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/compression/base.cpp
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/compression/factory.cpp
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/compression/iterator.cpp
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/compression/zstd.cpp
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/compression/none.cpp
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/control_events.cpp
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/event_reader.cpp
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/load_data_events.cpp
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/rows_event.cpp
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/statement_events.cpp
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/uuid.cpp
  )

# Configure for building static library
ADD_LIBRARY(binlogstandalone_static STATIC ${REPLICATION_SOURCES})

TARGET_LINK_LIBRARIES(binlogstandalone_static ${ZSTD_LIBRARY})

SET_TARGET_PROPERTIES(binlogstandalone_static
  PROPERTIES OUTPUT_NAME "binlogstandalone"
  )

SET_TARGET_PROPERTIES(binlogstandalone_static
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/libbinlogstandalone/lib
  )
