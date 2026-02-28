/* Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <mysql/components/component_implementation.h>
#include <mysql/components/minimal_chassis.h>
#include <mysql/components/my_service.h>
#include <mysql/components/services/dynamic_loader_scheme_file.h>
#include <mysql/components/services/mysql_cond_service.h>
#include <mysql/components/services/mysql_mutex_service.h>
#include <mysql/components/services/mysql_psi_system_service.h>
#include <mysql/components/services/mysql_runtime_error_service.h>
#include <mysql/components/services/mysql_rwlock_service.h>

// pfs services
#include "storage/perfschema/pfs.h"
#include "storage/perfschema/pfs_plugin_table.h"

#include <stddef.h>
#include <new>
#include <stdexcept>  // std::exception subclasses

#include "audit_api_message_service_imp.h"
#include "component_status_var_service_imp.h"
#include "component_sys_var_service_imp.h"
#include "dynamic_loader_path_filter_imp.h"
#include "host_application_signal_imp.h"
#include "keyring_iterator_service_imp.h"
#include "log_builtins_filter_imp.h"
#include "log_builtins_imp.h"
#include "my_inttypes.h"
#include "mysql_backup_lock_imp.h"
#include "mysql_clone_protocol_imp.h"
#include "mysql_connection_attributes_iterator_imp.h"
#include "mysql_current_thread_reader_imp.h"
#include "mysql_ongoing_transaction_query_imp.h"
#include "mysql_page_track_imp.h"
#include "mysql_runtime_error_imp.h"
#include "mysql_server_runnable_imp.h"
#include "mysql_string_service_imp.h"
#include "mysqld_error.h"
#include "persistent_dynamic_loader_imp.h"
#include "security_context_imp.h"
#include "sql/auth/dynamic_privileges_impl.h"
#include "sql/log.h"
#include "sql/mysqld.h"  // srv_registry
#include "sql/server_component/mysql_admin_session_imp.h"
#include "sql/udf_registration_imp.h"
#include "storage/perfschema/pfs_services.h"
#include "system_variable_source_imp.h"
#include "udf_metadata_imp.h"

// Must come after sql/log.h.
#include "mysql/components/services/log_builtins.h"

/* Implementation located in the mysql_server component. */
extern SERVICE_TYPE(mysql_cond_v1)
    SERVICE_IMPLEMENTATION(mysql_server, mysql_cond_v1);
extern SERVICE_TYPE(mysql_mutex_v1)
    SERVICE_IMPLEMENTATION(mysql_server, mysql_mutex_v1);
extern SERVICE_TYPE(mysql_rwlock_v1)
    SERVICE_IMPLEMENTATION(mysql_server, mysql_rwlock_v1);
extern SERVICE_TYPE(mysql_psi_system_v1)
    SERVICE_IMPLEMENTATION(mysql_server, mysql_psi_system_v1);

extern REQUIRES_SERVICE_PLACEHOLDER(mysql_rwlock_v1);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_psi_system_v1);

BEGIN_SERVICE_IMPLEMENTATION(mysql_server_path_filter,
                             dynamic_loader_scheme_file)
mysql_dynamic_loader_scheme_file_path_filter_imp::load,
    mysql_dynamic_loader_scheme_file_path_filter_imp::unload
    END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, persistent_dynamic_loader)
mysql_persistent_dynamic_loader_imp::load,
    mysql_persistent_dynamic_loader_imp::unload END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, dynamic_privilege_register)
dynamic_privilege_services_impl::register_privilege,
    dynamic_privilege_services_impl::unregister_privilege
    END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, global_grants_check)
dynamic_privilege_services_impl::has_global_grant END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_string_factory)
mysql_string_imp::create,
    mysql_string_imp::destroy END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_string_case)
mysql_string_imp::tolower,
    mysql_string_imp::toupper END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_string_converter)
mysql_string_imp::convert_from_buffer,
    mysql_string_imp::convert_to_buffer END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_string_character_access)
mysql_string_imp::get_char,
    mysql_string_imp::get_char_length END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_string_byte_access)
mysql_string_imp::get_byte,
    mysql_string_imp::get_byte_length END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_string_iterator)
mysql_string_imp::iterator_create, mysql_string_imp::iterator_get_next,
    mysql_string_imp::iterator_destroy END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_string_ctype)
mysql_string_imp::is_upper, mysql_string_imp::is_lower,
    mysql_string_imp::is_digit END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, log_builtins)
log_builtins_imp::wellknown_by_type, log_builtins_imp::wellknown_by_name,
    log_builtins_imp::wellknown_get_type, log_builtins_imp::wellknown_get_name,

    log_builtins_imp::item_inconsistent, log_builtins_imp::item_generic_type,
    log_builtins_imp::item_string_class, log_builtins_imp::item_numeric_class,

    log_builtins_imp::item_set_int, log_builtins_imp::item_set_float,
    log_builtins_imp::item_set_lexstring, log_builtins_imp::item_set_cstring,

    log_builtins_imp::item_set_with_key, log_builtins_imp::item_set,

    log_builtins_imp::line_item_set_with_key, log_builtins_imp::line_item_set,

    log_builtins_imp::line_init, log_builtins_imp::line_exit,
    log_builtins_imp::line_item_count,

    log_builtins_imp::line_item_types_seen,

    log_builtins_imp::line_item_iter_acquire,
    log_builtins_imp::line_item_iter_release,
    log_builtins_imp::line_item_iter_first,
    log_builtins_imp::line_item_iter_next,
    log_builtins_imp::line_item_iter_current,

    log_builtins_imp::line_submit,

    log_builtins_imp::message,

    log_builtins_imp::sanitize,

    log_builtins_imp::errmsg_by_errcode, log_builtins_imp::errcode_by_errsymbol,

    log_builtins_imp::label_from_prio,

    log_builtins_imp::open_errstream, log_builtins_imp::write_errstream,
    log_builtins_imp::dedicated_errstream,
    log_builtins_imp::close_errstream END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, log_builtins_filter)
log_builtins_filter_imp::filter_ruleset_new,
    log_builtins_filter_imp::filter_ruleset_lock,
    log_builtins_filter_imp::filter_ruleset_unlock,
    log_builtins_filter_imp::filter_ruleset_drop,
    log_builtins_filter_imp::filter_ruleset_free,
    log_builtins_filter_imp::filter_ruleset_move,
    log_builtins_filter_imp::filter_rule_init,
    log_builtins_filter_imp::filter_run END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, log_builtins_filter_debug)
log_builtins_filter_debug_imp::filter_debug_ruleset_get
END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, log_builtins_string)
log_builtins_string_imp::malloc, log_builtins_string_imp::strndup,
    log_builtins_string_imp::free,

    log_builtins_string_imp::length, log_builtins_string_imp::find_first,
    log_builtins_string_imp::find_last,

    log_builtins_string_imp::compare,

    log_builtins_string_imp::substitutev,
    log_builtins_string_imp::substitute END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, log_builtins_tmp)
log_builtins_tmp_imp::notify_client END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, log_builtins_syseventlog)
log_builtins_syseventlog_imp::open, log_builtins_syseventlog_imp::write,
    log_builtins_syseventlog_imp::close END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, udf_registration)
mysql_udf_registration_imp::udf_register,
    mysql_udf_registration_imp::udf_unregister END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, udf_registration_aggregate)
mysql_udf_registration_imp::udf_register_aggregate,
    mysql_udf_registration_imp::udf_unregister END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_udf_metadata)
mysql_udf_metadata_imp::argument_get, mysql_udf_metadata_imp::result_get,
    mysql_udf_metadata_imp::argument_set,
    mysql_udf_metadata_imp::result_set END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, component_sys_variable_register)
mysql_component_sys_variable_imp::register_variable,
    mysql_component_sys_variable_imp::get_variable END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_connection_attributes_iterator)
mysql_connection_attributes_iterator_imp::init,
    mysql_connection_attributes_iterator_imp::deinit,
    mysql_connection_attributes_iterator_imp::get END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, component_sys_variable_unregister)
mysql_component_sys_variable_imp::unregister_variable,
    END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, status_variable_registration)
mysql_status_variable_registration_imp::register_variable,
    mysql_status_variable_registration_imp::unregister_variable
    END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, system_variable_source)
mysql_system_variable_source_imp::get END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_backup_lock)
mysql_acquire_backup_lock_nsec,
    mysql_release_backup_lock END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, clone_protocol)
mysql_clone_start_statement, mysql_clone_finish_statement,
    mysql_clone_get_charsets, mysql_clone_validate_charsets,
    mysql_clone_get_configs, mysql_clone_validate_configs, mysql_clone_connect,
    mysql_clone_send_command, mysql_clone_get_response, mysql_clone_kill,
    mysql_clone_disconnect, mysql_clone_get_error, mysql_clone_get_command,
    mysql_clone_send_response,
    mysql_clone_send_error END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_thd_security_context)
mysql_security_context_imp::get,
    mysql_security_context_imp::set END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_security_context_factory)
mysql_security_context_imp::create, mysql_security_context_imp::destroy,
    mysql_security_context_imp::copy END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server,
                             mysql_account_database_security_context_lookup)
mysql_security_context_imp::lookup END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_security_context_options)
mysql_security_context_imp::get,
    mysql_security_context_imp::set END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_ongoing_transactions_query)
mysql_ongoing_transactions_query_imp::get_ongoing_server_transactions
END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, host_application_signal)
mysql_component_host_application_signal_imp::signal
END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_audit_api_message)
mysql_audit_api_message_imp::emit END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_page_track)
Page_track_implementation::start, Page_track_implementation::stop,
    Page_track_implementation::purge, Page_track_implementation::get_page_ids,
    Page_track_implementation::get_num_page_ids,
    Page_track_implementation::get_status END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_runtime_error)
mysql_server_runtime_error_imp::emit END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_current_thread_reader)
mysql_component_mysql_current_thread_reader_imp::get
END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_keyring_iterator)
mysql_keyring_iterator_imp::init, mysql_keyring_iterator_imp::deinit,
    mysql_keyring_iterator_imp::get END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_admin_session)
mysql_component_mysql_admin_session_imp::open END_SERVICE_IMPLEMENTATION();

BEGIN_SERVICE_IMPLEMENTATION(mysql_server, mysql_server_runnable)
mysql_server_runnable_imp::run END_SERVICE_IMPLEMENTATION();

BEGIN_COMPONENT_PROVIDES(mysql_server)
PROVIDES_SERVICE(mysql_server_path_filter, dynamic_loader_scheme_file),
    PROVIDES_SERVICE(mysql_server, persistent_dynamic_loader),
    PROVIDES_SERVICE(mysql_server, dynamic_privilege_register),
    PROVIDES_SERVICE(mysql_server, global_grants_check),
    PROVIDES_SERVICE(mysql_server, mysql_string_factory),
    PROVIDES_SERVICE(mysql_server, mysql_string_case),
    PROVIDES_SERVICE(mysql_server, mysql_string_converter),
    PROVIDES_SERVICE(mysql_server, mysql_string_character_access),
    PROVIDES_SERVICE(mysql_server, mysql_string_byte_access),
    PROVIDES_SERVICE(mysql_server, mysql_string_iterator),
    PROVIDES_SERVICE(mysql_server, mysql_string_ctype),
    PROVIDES_SERVICE(mysql_server, log_builtins),
    PROVIDES_SERVICE(mysql_server, log_builtins_filter),
    PROVIDES_SERVICE(mysql_server, log_builtins_filter_debug),
    PROVIDES_SERVICE(mysql_server, log_builtins_string),
    PROVIDES_SERVICE(mysql_server, log_builtins_tmp),
    PROVIDES_SERVICE(mysql_server, log_builtins_syseventlog),
    PROVIDES_SERVICE(mysql_server, udf_registration),
    PROVIDES_SERVICE(mysql_server, udf_registration_aggregate),
    PROVIDES_SERVICE(mysql_server, mysql_udf_metadata),
    PROVIDES_SERVICE(mysql_server, component_sys_variable_register),
    PROVIDES_SERVICE(mysql_server, component_sys_variable_unregister),
    PROVIDES_SERVICE(mysql_server, mysql_cond_v1),
    PROVIDES_SERVICE(mysql_server, mysql_mutex_v1),
    PROVIDES_SERVICE(mysql_server, mysql_rwlock_v1),
    PROVIDES_SERVICE(mysql_server, status_variable_registration),
    PROVIDES_SERVICE(mysql_server, system_variable_source),
    PROVIDES_SERVICE(mysql_server, mysql_backup_lock),
    PROVIDES_SERVICE(mysql_server, clone_protocol),
    PROVIDES_SERVICE(mysql_server, mysql_thd_security_context),
    PROVIDES_SERVICE(mysql_server, mysql_security_context_factory),
    PROVIDES_SERVICE(mysql_server,
                     mysql_account_database_security_context_lookup),
    PROVIDES_SERVICE(mysql_server, mysql_security_context_options),
    PROVIDES_SERVICE(mysql_server, mysql_ongoing_transactions_query),
    PROVIDES_SERVICE(mysql_server, host_application_signal),
    PROVIDES_SERVICE(mysql_server, mysql_audit_api_message),
    PROVIDES_SERVICE(mysql_server, mysql_page_track),
    PROVIDES_SERVICE(mysql_server, mysql_runtime_error),
    PROVIDES_SERVICE(mysql_server, mysql_current_thread_reader),
    PROVIDES_SERVICE(mysql_server, mysql_keyring_iterator),
    PROVIDES_SERVICE(mysql_server, mysql_admin_session),
    PROVIDES_SERVICE(mysql_server, mysql_connection_attributes_iterator),
    PROVIDES_SERVICE(mysql_server, mysql_server_runnable),
    PROVIDES_SERVICE(mysql_server, mysql_psi_system_v1),
    PROVIDES_SERVICE(performance_schema, psi_cond_v1),
    PROVIDES_SERVICE(performance_schema, psi_error_v1),
    PROVIDES_SERVICE(performance_schema, psi_file_v2),
    PROVIDES_SERVICE(performance_schema, psi_idle_v1),
    PROVIDES_SERVICE(performance_schema, psi_mdl_v1),
    PROVIDES_SERVICE(performance_schema, psi_memory_v1),
    PROVIDES_SERVICE(performance_schema, psi_mutex_v1),
    /* Obsolete: PROVIDES_SERVICE(performance_schema, psi_rwlock_v1), */
    PROVIDES_SERVICE(performance_schema, psi_rwlock_v2),
    PROVIDES_SERVICE(performance_schema, psi_socket_v1),
    PROVIDES_SERVICE(performance_schema, psi_stage_v1),
    /* Deprecated, use psi_statement_v2. */
    PROVIDES_SERVICE(performance_schema, psi_statement_v1),
    PROVIDES_SERVICE(performance_schema, psi_statement_v2),
    PROVIDES_SERVICE(performance_schema, psi_system_v1),
    PROVIDES_SERVICE(performance_schema, psi_table_v1),
    /* Obsolete: PROVIDES_SERVICE(performance_schema, psi_thread_v1), */
    /* Obsolete: PROVIDES_SERVICE(performance_schema, psi_thread_v2), */
    PROVIDES_SERVICE(performance_schema, psi_thread_v3),
    PROVIDES_SERVICE(performance_schema, psi_transaction_v1),
    /* Deprecated, use pfs_plugin_table_v1. */
    PROVIDES_SERVICE(performance_schema, pfs_plugin_table),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_table_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_tiny_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_small_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_medium_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_integer_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_bigint_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_decimal_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_float_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_double_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_string_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_blob_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_enum_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_date_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_time_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_datetime_v1),
    /* Deprecated, use pfs_plugin_column_timestamp_v2. */
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_timestamp_v1),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_timestamp_v2),
    PROVIDES_SERVICE(performance_schema, pfs_plugin_column_year_v1),
    END_COMPONENT_PROVIDES();

static BEGIN_COMPONENT_REQUIRES(mysql_server) END_COMPONENT_REQUIRES();

#ifndef WITH_MYSQL_COMPONENTS_TEST_DRIVER
/*
  These symbols are present in minimal chassis library. We defined again
  for minimal chassis test driver, because we are not supposed to link
  the minchassis to component_mysql_server
  On windows we see these symbol issue, On other OSs we are seeing ODR
  violation error(i.e ASAN error). Hence we added WIN define.
*/
#ifdef _WIN32
REQUIRES_SERVICE_PLACEHOLDER(mysql_rwlock_v1);
REQUIRES_SERVICE_PLACEHOLDER(mysql_psi_system_v1);
REQUIRES_SERVICE_PLACEHOLDER(mysql_runtime_error);
void mysql_components_handle_std_exception(const char *) {}
#endif
#endif

static mysql_service_status_t mysql_server_init() {
#ifndef WITH_MYSQL_COMPONENTS_TEST_DRIVER
  /*
    Changing minimal_chassis service implementations to mysql_server service
    implementations
  */
  mysql_service_mysql_rwlock_v1 = &imp_mysql_server_mysql_rwlock_v1;
  mysql_service_mysql_psi_system_v1 = &imp_mysql_server_mysql_psi_system_v1;
  mysql_service_mysql_runtime_error = &imp_mysql_server_mysql_runtime_error;
#endif
  return false;
}

static mysql_service_status_t mysql_server_deinit() { return false; }

/*
  This wrapper function is created to have the conditional compilation for
  mysqld server code(i.e sql_main) and the component_mysql_server used for
  minimal chassis test driver.
*/
bool initialize_minimal_chassis(SERVICE_TYPE_NO_CONST(registry) * *registry) {
#ifndef WITH_MYSQL_COMPONENTS_TEST_DRIVER
  /*
    In case of minimal chassis test driver we just need to update the
    registry with the mysql_service_registry reference. Because we already
    initialized the minimal chassis in the test binary.
  */
  *registry =
      const_cast<SERVICE_TYPE_NO_CONST(registry) *>(mysql_service_registry);
#else
  /* Normal server code path. Hence we need to initialize minimal chassis */
  if (minimal_chassis_init(registry, &COMPONENT_REF(mysql_server))) {
    return true;
  }
#endif
  return false;
}

bool deinitialize_minimal_chassis(SERVICE_TYPE_NO_CONST(registry) *
                                  registry MY_ATTRIBUTE((unused))) {
#ifdef WITH_MYSQL_COMPONENTS_TEST_DRIVER
  /* Normal server code path. Hence we need to deinitialize minimal chassis */
  if (minimal_chassis_deinit(registry, &COMPONENT_REF(mysql_server))) {
    return true;
  }
#endif
  return false;
}

BEGIN_COMPONENT_METADATA(mysql_server)
METADATA("mysql.author", "Oracle Corporation"),
    METADATA("mysql.license", "GPL"), END_COMPONENT_METADATA();

DECLARE_COMPONENT(mysql_server, "mysql:core")
mysql_server_init, mysql_server_deinit END_DECLARE_COMPONENT();

/* Below library header code is needed when component_mysql_server.so is
   created. And it is not needed when the code is part of mysqld executable.
   Hence WITH_MYSQL_COMPONENTS_TEST_DRIVER is used to handle the
   conditional compilation. */
#ifndef WITH_MYSQL_COMPONENTS_TEST_DRIVER
/* components contained in this library.
   for now assume that each library will have exactly one component. */
DECLARE_LIBRARY_COMPONENTS &COMPONENT_REF(mysql_server)
    END_DECLARE_LIBRARY_COMPONENTS
#endif
