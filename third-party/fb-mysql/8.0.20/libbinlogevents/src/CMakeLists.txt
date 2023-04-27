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
  binary_log_funcs.cpp
  binlog_event.cpp
  codecs/binary.cpp
  codecs/factory.cpp
  compression/base.cpp
  compression/factory.cpp
  compression/iterator.cpp
  compression/none.cpp
  compression/zstd.cpp
  control_events.cpp
  event_reader.cpp
  load_data_events.cpp
  rows_event.cpp
  statement_events.cpp
  trx_boundary_parser.cpp
  uuid.cpp
  )

# Configure for building static library
ADD_LIBRARY(binlogevents_static STATIC ${REPLICATION_SOURCES})

IF(STANDALONE_BINLOG)
  TARGET_LINK_LIBRARIES(binlogevents_static  ${ZSTD_LIBRARY})
ELSE()
  TARGET_LINK_LIBRARIES(binlogevents_static ${ZSTD_LIBRARY} mysys)
  ADD_DEPENDENCIES(binlogevents_static GenError)
ENDIF()

SET_TARGET_PROPERTIES(binlogevents_static
  PROPERTIES OUTPUT_NAME "binlogevents"
  )

SET_TARGET_PROPERTIES(binlogevents_static
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/libbinlogevents/lib
  )
