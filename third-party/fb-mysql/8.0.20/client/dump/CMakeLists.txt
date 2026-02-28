# Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.
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

# Prevent Boost from including external precompiled Boost libraries,
# and turn off unused functionality.
IF(USING_LOCAL_BOOST)
  ADD_DEFINITIONS(
    -DBOOST_ALL_NO_LIB
    -DBOOST_SYSTEM_NO_DEPRECATED)
ENDIF()

INCLUDE_DIRECTORIES(SYSTEM ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})

SET(MYSQLPUMP_LIB_SOURCES
  abstract_chain_element.cc
  abstract_connection_provider.cc
  abstract_crawler.cc
  abstract_data_formatter_wrapper.cc
  abstract_data_object.cc
  abstract_database_dump_task.cc
  abstract_dump_task.cc
  abstract_mysql_chain_element_extension.cc
  abstract_object_reader_wrapper.cc
  abstract_output_writer_wrapper.cc
  abstract_plain_sql_object.cc
  abstract_plain_sql_object_dump_task.cc
  abstract_progress_reporter.cc
  abstract_progress_watcher.cc
  abstract_simple_dump_task.cc
  abstract_table_dump_task.cc
  chain_data.cc
  column_statistic.cc
  composite_message_handler.cc
  compression_lz4_writer.cc
  compression_zlib_writer.cc
  database.cc
  database_end_dump_task.cc
  database_start_dump_task.cc
  dump_end_dump_task.cc
  dump_start_dump_task.cc
  event_scheduler_event.cc
  field.cc
  file_writer.cc
  i_chain_element.cc
  i_dump_task.cc
  i_progress_reporter.cc
  item_processing_data.cc
  mysql_chain_element_options.cc
  mysql_crawler.cc
  mysql_field.cc
  mysql_function.cc
  mysql_object_reader.cc
  mysql_object_reader_options.cc
  mysqldump_tool_chain_maker.cc
  mysqldump_tool_chain_maker_options.cc
  object_filter.cc
  object_queue.cc
  pattern_matcher.cc
  privilege.cc
  row.cc
  row_group_dump_task.cc
  simple_id_generator.cc
  single_transaction_connection_provider.cc
  sql_formatter.cc
  sql_formatter_options.cc
  standard_progress_watcher.cc
  standard_writer.cc
  stored_procedure.cc
  table.cc
  table_deferred_indexes_dump_task.cc
  table_definition_dump_task.cc
  table_rows_dump_task.cc
  tables_definition_ready_dump_task.cc
  thread.cc
  thread_group.cc
  thread_specific_connection_provider.cc
  trigger.cc
  view.cc
)
ADD_LIBRARY(mysqlpump_lib STATIC ${MYSQLPUMP_LIB_SOURCES})
TARGET_LINK_LIBRARIES(mysqlpump_lib
   client_base ${LZ4_LIBRARY})

MYSQL_ADD_EXECUTABLE(mysqlpump  program.cc)

TARGET_LINK_LIBRARIES(mysqlpump mysqlpump_lib)

SET_TARGET_PROPERTIES(mysqlpump PROPERTIES HAS_CXX TRUE)
