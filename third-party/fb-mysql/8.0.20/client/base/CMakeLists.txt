# Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.
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

ADD_LIBRARY(client_base STATIC
  bool_option.cc
  char_array_option.cc
  disabled_option.cc
  i_option.cc
  password_option.cc
  simple_option.cc
  string_option.cc

  abstract_options_provider.cc
  composite_options_provider.cc
  debug_options.cc
  help_options.cc
  mysql_connection_options.cc
  ssl_options.cc

  abstract_program.cc
  abstract_connection_program.cc

  message_data.cc
  mysql_query_runner.cc
  show_variable_query_extractor.cc

  ${CMAKE_SOURCE_DIR}/sql-common/sql_string.cc
)

TARGET_LINK_LIBRARIES(client_base mysqlclient)
