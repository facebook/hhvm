# Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
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

SET(MYSQL_SERVER_COMPONENT_SOURCES
  audit_api_message_service_imp.cc
  backup_lock_service.cc
  clone_protocol_service.cc
  component_status_var_service.cc
  component_sys_var_service.cc
  dynamic_loader_path_filter.cc
  host_application_signal_imp.cc
  keyring_iterator_service_imp.cc
  log_builtins.cc
  log_builtins_filter.cc
  mysql_cond_service.cc
  mysql_connection_attributes_iterator_imp.cc
  mysql_current_thread_reader_imp.cc
  mysql_mutex_service.cc
  mysql_psi_system_service.cc
  mysql_runtime_error.cc
  mysql_rwlock_service.cc
  mysql_server_runnable_imp.cc
  mysql_string_service.cc
  ongoing_transaction_query_service.cc
  page_track_service.cc
  persistent_dynamic_loader.cc
  plugin_registry_service.cc
  security_context_imp.cc
  server_component.cc
  system_variable_source_imp.cc
  udf_metadata_imp.cc
  )

# This static library is used to build mysqld binary and in some unit test cases
ADD_LIBRARY(mysql_server_component_services STATIC
  ${MYSQL_SERVER_COMPONENT_SOURCES})
TARGET_COMPILE_DEFINITIONS(mysql_server_component_services
  PRIVATE WITH_MYSQL_COMPONENTS_TEST_DRIVER)
ADD_DEPENDENCIES(mysql_server_component_services GenError)
