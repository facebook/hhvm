# Copyright (c) 2006, 2020, Oracle and/or its affiliates. All rights reserved.
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

INCLUDE_DIRECTORIES(SYSTEM ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})

MY_INCLUDE_SYSTEM_DIRECTORIES(ICU)

IF(MYSQL_MAINTAINER_MODE)
  IF(MY_COMPILER_IS_GNU AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
    ADD_COMPILE_OPTIONS("-Wshadow=local")
  ELSEIF(MY_COMPILER_IS_CLANG)
    ADD_COMPILE_OPTIONS("-Wshadow-uncaptured-local")
  ENDIF()
ENDIF()

# Array indexing with pland_idx == int8 gives warnings.
IF(SOLARIS AND MY_COMPILER_IS_GNU)
  ADD_COMPILE_OPTIONS("-Wno-char-subscripts")
ENDIF()

SET(CONF_SOURCES
  ${CMAKE_CURRENT_BINARY_DIR}/sql_builtin.cc
)
SET(GEN_SOURCES
  ${CMAKE_CURRENT_BINARY_DIR}/sql_yacc.h
  ${CMAKE_CURRENT_BINARY_DIR}/sql_yacc.cc
  ${CMAKE_CURRENT_BINARY_DIR}/sql_hints.yy.h
  ${CMAKE_CURRENT_BINARY_DIR}/sql_hints.yy.cc
  ${CMAKE_CURRENT_BINARY_DIR}/lex_hash.h
  ${CMAKE_CURRENT_BINARY_DIR}/mysql_githash.h
)
SET(GEN_DIGEST_SOURCES
  ${CMAKE_CURRENT_BINARY_DIR}/lex_token.h
)
SET(GEN_KEYWORD_LIST_SOURCES
  ${CMAKE_CURRENT_BINARY_DIR}/keyword_list.h
)

SET_SOURCE_FILES_PROPERTIES(
  ${GEN_SOURCES}
  ${GEN_DIGEST_SOURCES}
  ${GEN_KEYWORD_LIST_SOURCES}
  ${CONF_SOURCES}
  PROPERTIES GENERATED 1)

# Gen_lex_token
# Make sure sql_yacc.h is generated before compiling gen_lex_token
MYSQL_ADD_EXECUTABLE(gen_lex_token
  gen_lex_token.cc
  DEPENDENCIES GenServerSource
  SKIP_INSTALL
  )

MY_CHECK_CXX_COMPILER_WARNING("-Wmissing-profile" HAS_MISSING_PROFILE)
IF(HAS_MISSING_PROFILE)
  ADD_COMPILE_FLAGS(gen_lex_token.cc COMPILE_FLAGS ${HAS_MISSING_PROFILE})
ENDIF()

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lex_token.h
  COMMAND gen_lex_token > lex_token.h
  DEPENDS gen_lex_token
)

# gen_keyword_list
# Make sure sql_yacc.h is generated before compiling gen_keyword_list
MYSQL_ADD_EXECUTABLE(gen_keyword_list
  gen_keyword_list.cc
  DEPENDENCIES GenServerSource
  LINK_LIBRARIES ${ICU_LIBRARIES} ${CMAKE_DL_LIBS}
  SKIP_INSTALL
  )

IF(HAS_MISSING_PROFILE)
  ADD_COMPILE_FLAGS(gen_keyword_list.cc COMPILE_FLAGS ${HAS_MISSING_PROFILE})
ENDIF()

# ICU headers for WIN32 need extra info about their libs state
IF(WIN32)
  ADD_COMPILE_DEFINITIONS(gen_keyword_list.cc COMPILE_DEFINITIONS
    U_STATIC_IMPLEMENTATION)
ENDIF()

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/keyword_list.h
  COMMAND gen_keyword_list ${CMAKE_CURRENT_SOURCE_DIR}/sql_yacc.yy > ${CMAKE_CURRENT_BINARY_DIR}/keyword_list.h
  DEPENDS gen_keyword_list
)

ADD_DEFINITIONS(-DMYSQL_SERVER)
ADD_SUBDIRECTORY(server_component)

SET(DD_SOURCES
  dd/collection.cc
  dd/dd_event.cc
  dd/dd_resource_group.cc
  dd/dd_routine.cc
  dd/dd_schema.cc
  dd/dd_table.cc
  dd/dd_tablespace.cc
  dd/dd_trigger.cc
  dd/dd_view.cc
  dd/dd_utility.cc
  dd/properties.cc

  dd/impl/dd.cc
  dd/impl/dictionary_impl.cc
  dd/impl/properties_impl.cc
  dd/impl/system_registry.cc
  dd/impl/transaction_impl.cc
  dd/impl/utils.cc
  dd/impl/sdi.cc
  dd/impl/sdi_api.cc
  dd/impl/sdi_file.cc
  dd/impl/sdi_tablespace.cc
  dd/impl/string_type.cc

  dd/impl/bootstrap/bootstrap_ctx.cc
  dd/impl/bootstrap/bootstrapper.cc

  dd/impl/cache/dictionary_client.cc
  dd/impl/cache/local_multi_map.cc
  dd/impl/cache/multi_map_base.cc
  dd/impl/cache/shared_multi_map.cc
  dd/impl/cache/shared_dictionary_cache.cc
  dd/impl/cache/storage_adapter.cc

  dd/impl/raw/object_keys.cc
  dd/impl/raw/raw_record.cc
  dd/impl/raw/raw_record_set.cc
  dd/impl/raw/raw_table.cc

  dd/impl/system_views/administrable_role_authorizations.cc
  dd/impl/system_views/applicable_roles.cc
  dd/impl/system_views/character_sets.cc
  dd/impl/system_views/check_constraints.cc
  dd/impl/system_views/collations.cc
  dd/impl/system_views/collation_charset_applicability.cc
  dd/impl/system_views/columns.cc
  dd/impl/system_views/column_statistics.cc
  dd/impl/system_views/enabled_roles.cc
  dd/impl/system_views/events.cc
  dd/impl/system_views/files.cc
  dd/impl/system_views/innodb_datafiles.cc
  dd/impl/system_views/innodb_foreign.cc
  dd/impl/system_views/innodb_foreign_cols.cc
  dd/impl/system_views/innodb_fields.cc
  dd/impl/system_views/innodb_tablespaces_brief.cc
  dd/impl/system_views/key_column_usage.cc
  dd/impl/system_views/keywords.cc
  dd/impl/system_views/parameters.cc
  dd/impl/system_views/partitions.cc
  dd/impl/system_views/referential_constraints.cc
  dd/impl/system_views/resource_groups.cc
  dd/impl/system_views/role_column_grants.cc
  dd/impl/system_views/role_routine_grants.cc
  dd/impl/system_views/role_table_grants.cc
  dd/impl/system_views/routines.cc
  dd/impl/system_views/schemata.cc
  dd/impl/system_views/schemata_ext.cc
  dd/impl/system_views/statistics.cc
  dd/impl/system_views/st_spatial_reference_systems.cc
  dd/impl/system_views/st_units_of_measure.cc
  dd/impl/system_views/st_geometry_columns.cc
  dd/impl/system_views/tables.cc
  dd/impl/system_views/table_constraints.cc
  dd/impl/system_views/triggers.cc
  dd/impl/system_views/view_routine_usage.cc
  dd/impl/system_views/view_table_usage.cc
  dd/impl/system_views/views.cc

  dd/impl/tables/character_sets.cc
  dd/impl/tables/check_constraints.cc
  dd/impl/tables/collations.cc
  dd/impl/tables/columns.cc
  dd/impl/tables/column_statistics.cc
  dd/impl/tables/column_type_elements.cc
  dd/impl/tables/dd_properties.cc
  dd/impl/tables/events.cc
  dd/impl/tables/foreign_key_column_usage.cc
  dd/impl/tables/foreign_keys.cc
  dd/impl/tables/index_column_usage.cc
  dd/impl/tables/indexes.cc
  dd/impl/tables/index_partitions.cc
  dd/impl/tables/index_stats.cc
  dd/impl/tables/parameters.cc
  dd/impl/tables/parameter_type_elements.cc
  dd/impl/tables/resource_groups.cc
  dd/impl/tables/routines.cc
  dd/impl/tables/schemata.cc
  dd/impl/tables/spatial_reference_systems.cc
  dd/impl/tables/table_partitions.cc
  dd/impl/tables/table_stats.cc
  dd/impl/tables/table_partition_values.cc
  dd/impl/tables/tables.cc
  dd/impl/tables/tablespace_files.cc
  dd/impl/tables/tablespaces.cc
  dd/impl/tables/triggers.cc
  dd/impl/tables/view_routine_usage.cc
  dd/impl/tables/view_table_usage.cc

  dd/impl/types/index_stat_impl.cc
  dd/impl/types/table_stat_impl.cc
  dd/impl/types/abstract_table_impl.cc
  dd/impl/types/charset_impl.cc
  dd/impl/types/check_constraint_impl.cc
  dd/impl/types/collation_impl.cc
  dd/impl/types/column_impl.cc
  dd/impl/types/column_statistics_impl.cc
  dd/impl/types/column_type_element_impl.cc
  dd/impl/types/entity_object_table_impl.cc
  dd/impl/types/entity_object_impl.cc
  dd/impl/types/event_impl.cc
  dd/impl/types/foreign_key_element_impl.cc
  dd/impl/types/foreign_key_impl.cc
  dd/impl/types/function_impl.cc
  dd/impl/types/index_element_impl.cc
  dd/impl/types/index_impl.cc
  dd/impl/types/object_table_impl.cc
  dd/impl/types/object_table_definition_impl.cc
  dd/impl/types/parameter_impl.cc
  dd/impl/types/parameter_type_element_impl.cc
  dd/impl/types/partition_impl.cc
  dd/impl/types/partition_index_impl.cc
  dd/impl/types/partition_value_impl.cc
  dd/impl/types/procedure_impl.cc
  dd/impl/types/resource_group_impl.cc
  dd/impl/types/routine_impl.cc
  dd/impl/types/schema_impl.cc
  dd/impl/types/spatial_reference_system_impl.cc
  dd/impl/types/table_impl.cc
  dd/impl/types/tablespace_file_impl.cc
  dd/impl/types/tablespace_impl.cc
  dd/impl/types/trigger_impl.cc
  dd/impl/types/view_impl.cc
  dd/impl/types/view_routine_impl.cc
  dd/impl/types/view_table_impl.cc
  dd/impl/types/weak_object_impl.cc

  dd/impl/upgrade/server.cc
  dd/impl/upgrade/dd.cc

  dd/info_schema/metadata.cc
  dd/info_schema/show.cc
  dd/info_schema/show_query_builder.cc
  dd/info_schema/table_stats.cc
  dd/info_schema/tablespace_stats.cc

  dd/performance_schema/init.cc

  dd/ndbinfo_schema/init.cc

  dd/upgrade_57/event.cc
  dd/upgrade_57/global.cc
  dd/upgrade_57/routine.cc
  dd/upgrade_57/schema.cc
  dd/upgrade_57/table.cc
  dd/upgrade_57/upgrade.cc
)

SET(SQL_GIS_SOURCES
  geometry_rtree.cc
  gis/area.cc
  gis/covered_by.cc
  gis/crosses.cc
  gis/difference.cc
  gis/disjoint.cc
  gis/distance.cc
  gis/distance_functor.cc
  gis/distance_sphere.cc
  gis/equals.cc
  gis/gc_utils.cc
  gis/geometries.cc
  gis/intersects.cc
  gis/is_simple.cc
  gis/is_valid.cc
  gis/length.cc
  gis/mbr_utils.cc
  gis/overlaps.cc
  gis/ring_flip_visitor.cc
  gis/rtree_support.cc
  gis/simplify.cc
  gis/srs/srs.cc
  gis/srs/wkt_parser.cc
  gis/st_units_of_measure.cc
  gis/touches.cc
  gis/transform.cc
  gis/union.cc
  gis/within.cc
  gis/wkb.cc
  gis/wkb_size_visitor.cc
  gis/wkb_visitor.cc
  item_geofunc.cc
  item_geofunc_buffer.cc
  item_geofunc_internal.cc
  item_geofunc_relchecks.cc
  item_geofunc_relchecks_bgwrap.cc
  item_geofunc_setops.cc
  options_parser.cc
)

SET(SQL_SHARED_SOURCES
  abstract_query_plan.cc
  auth/auth_acls.cc
  auth/auth_common.cc
  auth/dynamic_privileges_impl.cc
  auth/dynamic_privilege_table.cc
  auth/acl_table_user.cc
  auth/sql_authentication.cc
  auth/sql_auth_cache.cc
  auth/sql_authorization.cc
  auth/sql_user_table.cc
  auth/sql_user.cc
  auth/partial_revokes.cc
  auth/password.cc
  auth/password_policy_service.cc
  auth/sql_security_ctx.cc
  auth/service_security_context.cc
  auto_thd.cc
  keyring_service.cc
  auth/roles.cc
  auth/role_tables.cc
  auth/sha2_password_common.cc
  auth/sha2_password.cc
  ssl_wrapper_service.cc
  bka_iterator.cc
  bootstrap.cc
  check_stack.cc
  conn_handler/connection_handler_manager.cc
  clone_handler.cc
  column_statistics.cc
  composite_iterators.cc
  create_field.cc
  current_thd.cc
  dd_sql_view.cc
  dd_sp.cc
  dd_table_share.cc
  debug_sync.cc
  default_values.cc
  derror.cc
  error_handler.cc
  field.cc
  field_conv.cc
  filesort.cc
  filesort_utils.cc
  aggregate_check.cc
  slave_stats_daemon.cc
  rpl_lag_manager.cc
  gstream.cc
  handler.cc
  hash_join_buffer.cc
  hash_join_chunk.cc
  hash_join_iterator.cc
  histograms/equi_height.cc
  histograms/equi_height_bucket.cc
  histograms/histogram.cc
  histograms/singleton.cc
  histograms/value_map.cc
  hostname_cache.cc
  index_statistics.cc
  init.cc
  log_resource.cc
  item.cc
  item_buff.cc
  item_cmpfunc.cc
  item_create.cc
  item_func.cc
  item_pfs_func.cc
  item_json_func.cc
  item_regexp_func.cc
  item_row.cc
  item_strfunc.cc
  item_subselect.cc
  item_sum.cc
  window.cc
  item_timefunc.cc
  item_xmlfunc.cc
  item_inetfunc.cc
  json_binary.cc
  json_diff.cc
  json_dom.cc
  json_path.cc
  json_schema.cc
  json_syntax_check.cc
  key.cc
  key_spec.cc
  keycaches.cc
  lock.cc
  locked_tables_list.cc
  locking_service.cc
  log.cc
  mdl.cc
  mdl_context_backup.cc
  migrate_keyring.cc
  my_decimal.cc
  mysqld.cc
  mysqld_thd_manager.cc
  opt_costconstantcache.cc
  opt_costconstants.cc
  opt_costmodel.cc
  opt_explain.cc
  opt_explain_format.cc
  opt_explain_traditional.cc
  opt_explain_json.cc
  opt_hints.cc
  opt_range.cc
  opt_statistics.cc
  opt_sum.cc
  opt_trace.cc
  opt_trace2server.cc
  parse_file.cc
  parse_tree_handler.cc
  parse_tree_helpers.cc
  parse_tree_hints.cc
  parse_tree_items.cc
  parse_tree_node_base.cc
  parse_tree_nodes.cc
  parse_tree_partitions.cc
  select_lex_visitor.cc
  parser_service.cc
  partition_info.cc
  partitioning/partition_handler.cc
  persisted_variable.cc
  protocol_classic.cc
  psi_memory_key.cc
  query_result.cc
  query_tag_perf_counter.cc
  records.cc
  regexp/errors.cc
  regexp/regexp_engine.cc
  regexp/regexp_facade.cc
  resourcegroups/thread_resource_control.cc
  resourcegroups/platform/thread_attrs_api_common.cc
  resourcegroups/resource_group_mgr.cc
  resourcegroups/resource_group_sql_cmd.cc
  rpl_group_replication.cc
  rpl_transaction_ctx.cc
  rpl_transaction_write_set_ctx.cc
  rpl_write_set_handler.cc
  rules_table_service.cc
  rwlock_scoped_lock.cc
  sd_notify.cc
  sdi_utils.cc
  session_tracker.cc
  set_var.cc
  sorting_iterator.cc
  sp.cc
  sp_cache.cc
  sp_head.cc
  sp_instr.cc
  sp_pcontext.cc
  sp_rcontext.cc
  spatial.cc
  string_service.cc
  sql_admin.cc
  sql_admission_control.cc
  sql_alloc_error_handler.cc
  sql_alter.cc
  sql_alter_instance.cc
  sql_backup_lock.cc
  sql_base.cc
  sql_bootstrap.cc
  sql_initialize.cc
  sql_call.cc
  sql_check_constraint.cc
  sql_class.cc
  sql_component.cc
  sql_const_folding.cc
  sql_cmd_ddl_table.cc
  sql_cmd_srs.cc
  sql_connect.cc
  sql_constraint.cc
  sql_cursor.cc
  sql_data_change.cc
  sql_db.cc
  sql_delete.cc
  sql_derived.cc
  sql_digest.cc
  sql_do.cc
  sql_error.cc
  sql_exception_handler.cc
  sql_executor.cc
  sql_get_diagnostics.cc
  sql_handler.cc
  sql_help.cc
  sql_import.cc
  sql_info.cc
  sql_insert.cc
  sql_join_buffer.cc
  sql_lex.cc
  sql_lex_hash.cc
  sql_lex_hints.cc
  sql_list.cc
  sql_load.cc
  sql_locale.cc
  sql_manager.cc
  sql_optimizer.cc
  sql_parse.cc
  sql_partition.cc
  sql_partition_admin.cc
  sql_planner.cc
  sql_plugin.cc
  sql_plugin_var.cc
  sql_prepare.cc
  sql_profile.cc
  sql_query_rewrite.cc
  sql_reload.cc
  sql_rename.cc
  sql_resolver.cc
  sql_restart_server.cc
  sql_rewrite.cc
  sql_select.cc
  sql_servers.cc
  sql_show.cc
  sql_show_status.cc
  sql_signal.cc
  sql_state.cc
  sql_table.cc
  sql_tablespace.cc
  sql_test.cc
  sql_thd_internal_api.cc
  sql_thd_api.cc
  sql_time.cc
  sql_timer.cc
  sql_tmp_table.cc
  sql_trigger.cc
  sql_truncate.cc
  sql_udf.cc
  sql_union.cc
  sql_update.cc
  sql_view.cc
  stateless_allocator.cc
  strfunc.cc
  sys_vars.cc
  sys_vars_resource_mgr.cc
  system_variables.cc
  table.cc
  table_cache.cc
  table_function.cc
  table_trigger_dispatcher.cc
  tc_log.cc
  thr_malloc.cc
  transaction.cc
  transaction_info.cc
  trigger.cc
  trigger_creation_ctx.cc
  trigger_chain.cc
  tztime.cc
  uniques.cc
  xa.cc
  ssl_acceptor_context.cc
)

# BISON_TARGET(<Name> <YaccInput> <CodeOutput>
#              [COMPILE_FLAGS <flags>]
#              [DEFINES_FILE <file>]
#              [VERBOSE <file>]
#              )
# FLEX_TARGET(Name FlexInput FlexOutput
#             [COMPILE_FLAGS <string>]
#             [DEFINES_FILE <string>]
#             )
# The Name argument is just an alias.
# Both macros will ADD_CUSTOM_COMMAND, with WORKING_DIRECTORY current src.
IF(WITH_LOCK_ORDER)

  FIND_PACKAGE(BISON REQUIRED)
  FIND_PACKAGE(FLEX REQUIRED)

  BISON_TARGET(debug_lo_parser
    debug_lo_parser.yy
    ${CMAKE_BINARY_DIR}/sql/debug_lo_parser.cc
    COMPILE_FLAGS "--name-prefix=LOCK_ORDER_ ${BISON_NO_LINE_OPT}"
    DEFINES_FILE ${CMAKE_BINARY_DIR}/sql/debug_lo_parser.h
    )
  FLEX_TARGET(debug_lo_scanner
    debug_lo_scanner.ll
    ${CMAKE_BINARY_DIR}/sql/debug_lo_scanner.cc
    COMPILE_FLAGS "--prefix=LOCK_ORDER_"
    DEFINES_FILE ${CMAKE_BINARY_DIR}/sql/debug_lo_scanner.h
    )
  MESSAGE(STATUS "BISON outputs ${BISON_debug_lo_parser_OUTPUTS}")
  MESSAGE(STATUS "FLEX outputs ${FLEX_debug_lo_scanner_OUTPUTS}")

  # Some targets, for easier debugging.
  ADD_CUSTOM_TARGET(GenBison_lo DEPENDS ${BISON_debug_lo_parser_OUTPUTS})
  ADD_CUSTOM_TARGET(GenFlex_lo DEPENDS ${FLEX_debug_lo_scanner_OUTPUTS})

  LIST(APPEND SQL_SHARED_SOURCES
    ${BISON_debug_lo_parser_OUTPUTS}
    ${FLEX_debug_lo_scanner_OUTPUTS}
    debug_lock_order.cc
    )

  IF(MY_COMPILER_IS_GNU_OR_CLANG)
    ADD_COMPILE_FLAGS(${FLEX_debug_lo_scanner_OUTPUTS}
      COMPILE_FLAGS
      "-Wno-sign-compare"
      "-Wno-unused-parameter"
      )
    ADD_COMPILE_FLAGS(${CMAKE_BINARY_DIR}/sql/debug_lo_parser.cc
      COMPILE_FLAGS
      "-Wno-undef"
      )
  ENDIF()

  # flex annotates some variables as 'register' and that keyword
  # will go away with C++17
  IF(APPLE)
    ADD_COMPILE_FLAGS(${FLEX_debug_lo_scanner_OUTPUTS}
      COMPILE_FLAGS
      "-Wno-deprecated-register"
      )
  ENDIF()
ENDIF()

UNSET(SQL_HEADERS)
IF(CMAKE_GENERATOR STREQUAL "Xcode")
  OPTION(GLOB_RECURSE_SQL_HEADERS "Find header files with GLOB_RECURSE" ON)
ELSE()
  OPTION(GLOB_RECURSE_SQL_HEADERS "Find header files with GLOB_RECURSE" OFF)
ENDIF()
IF(GLOB_RECURSE_SQL_HEADERS)
  # Needed so that IDEs (e.g. Xcode) can see all sources when searching
  # This recursive search finds over 1500 header files, and may actually
  # take a significant amount of time. Hence, do it only upon request.
  # Visual Studio will scan/parse for headers at startup anyways.
  FILE(GLOB_RECURSE SQL_HEADERS
    "${CMAKE_SOURCE_DIR}/include/*.h"
    "${CMAKE_SOURCE_DIR}/sql/*.h"
    "${CMAKE_SOURCE_DIR}/storage/*.h"
    )
ENDIF()

IF(LINUX)
  LIST(APPEND SQL_SHARED_SOURCES
    resourcegroups/platform/thread_attrs_api_linux.cc)
ELSEIF(APPLE)
  LIST(APPEND SQL_SHARED_SOURCES
    resourcegroups/platform/thread_attrs_api_apple.cc)
ELSEIF(FREEBSD)
  LIST(APPEND SQL_SHARED_SOURCES
    resourcegroups/platform/thread_attrs_api_freebsd.cc)
ELSEIF(SOLARIS)
  LIST(APPEND SQL_SHARED_SOURCES
    resourcegroups/platform/thread_attrs_api_solaris.cc)
ELSEIF(WIN32)
  LIST(APPEND SQL_SHARED_SOURCES
    resourcegroups/platform/thread_attrs_api_win.cc)
ENDIF()

SET(SQL_SOURCE
  ${GEN_SOURCES}
  ${GEN_DIGEST_SOURCES}
  ${GEN_KEYWORD_LIST_SOURCES}
  ${CONF_SOURCES}
  ${SQL_SHARED_SOURCES}
  ${SQL_HEADERS}
  ../extra/lz4/xxhash.c
  ../libmysql/errmsg.cc
  ../sql-common/client.cc
  ../sql-common/client_plugin.cc
  ../sql-common/client_authentication.cc
  ../sql-common/compression.cc
  ../sql-common/get_password.cc
  ../sql-common/my_path_permissions.cc
  ../sql-common/net_ns.cc
  ../sql-common/net_serv.cc
  ../sql-common/sql_string.cc
  command_service.cc
  conn_handler/channel_info.cc
  conn_handler/connection_handler_per_thread.cc
  conn_handler/connection_handler_one_thread.cc
  conn_handler/socket_connection.cc
  conn_handler/init_net_server_extension.cc
  event_data_objects.cc
  event_db_repository.cc
  event_parse_data.cc
  event_queue.cc
  event_scheduler.cc
  events.cc
  mf_iocache.cc
  protocol_callback.cc
  signal_handler.cc
  sql_audit.cc
  sql_client.cc
  srv_session.cc
  srv_session_info_service.cc
  srv_session_service.cc
  )

IF(NOT WIN32)
  LIST(APPEND SQL_SOURCE mysqld_daemon.cc)
ENDIF()

IF(WIN32)
  LIST(APPEND SQL_SOURCE
              conn_handler/named_pipe_connection.cc
              conn_handler/shared_memory_connection.cc
              named_pipe.cc
              restart_monitor_win.cc
      )
ENDIF()

# Segfault in boost::geometry code at high optimization levels
IF(CMAKE_CXX_COMPILER_ID MATCHES "SunPro")
  IF(CMAKE_BUILD_TYPE_UPPER MATCHES "REL")
    ADD_COMPILE_FLAGS(item_geofunc_buffer.cc COMPILE_FLAGS "-xO2")
  ENDIF()
ENDIF()

ADD_COMPILE_DEFINITIONS(
  ../extra/lz4/xxhash.c
  COMPILE_DEFINITIONS XXH_NAMESPACE=MY_
)

# Fixes "C1128: number of sections exceeded object file format limit" in MSVC
IF(WIN32)
  ADD_COMPILE_FLAGS(
    gis/difference.cc
    gis/touches.cc
    gis/union.cc
    gis/within.cc
    item_geofunc.cc
    item_geofunc_relchecks.cc
    item_geofunc_setops.cc
    COMPILE_FLAGS "/bigobj")
ENDIF()

# Downgrade -Werror to warning for gcc 8.2.x
IF(MY_COMPILER_IS_GNU
    AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.2
    AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.3)
  MY_CHECK_CXX_COMPILER_WARNING("error=maybe-uninitialized" HAS_WARN_FLAG)
  IF(HAS_WARN_FLAG)
    ADD_COMPILE_FLAGS(
      item_geofunc_relchecks_bgwrap.cc
      COMPILE_FLAGS ${HAS_WARN_FLAG}
      )
  ENDIF()
ENDIF()

# Force optimization of (large) MYSQLparse function in MSVC
IF(WIN32 AND NOT WIN32_CLANG)
  ADD_COMPILE_FLAGS(
    ${CMAKE_CURRENT_BINARY_DIR}/sql_yacc.cc
    COMPILE_FLAGS "/d2OptimizeHugeFunctions")
ENDIF()

# Common for all versions of GCC/Clang
# Bison output uses undefined symbols in #if checks
IF(MY_COMPILER_IS_GNU_OR_CLANG)
  ADD_COMPILE_FLAGS(${CMAKE_CURRENT_BINARY_DIR}/sql_yacc.cc
                    ${CMAKE_CURRENT_BINARY_DIR}/sql_hints.yy.cc
                    COMPILE_FLAGS "-Wno-undef")

ENDIF()

MY_CHECK_CXX_COMPILER_FLAG("-fno-builtin-memcmp" HAVE_NO_BUILTIN_MEMCMP)
# See comments in filesort_compare-t.cc about __builtin_memcmp
IF(HAVE_NO_BUILTIN_MEMCMP)
  ADD_COMPILE_FLAGS(
    filesort_utils.cc
    COMPILE_FLAGS "-fno-builtin-memcmp"
  )
ENDIF()


ADD_LIBRARY(sql_main STATIC ${SQL_SOURCE})
ADD_DEPENDENCIES(sql_main GenServerSource)
ADD_DEPENDENCIES(sql_main GenDigestServerSource)
ADD_DEPENDENCIES(sql_main GenBootstrapPriv)
ADD_DEPENDENCIES(sql_main GenSysSchema)
TARGET_LINK_LIBRARIES(sql_main ${MYSQLD_STATIC_PLUGIN_LIBS}
  mysql_server_component_services mysys strings vio
  binlogevents_static ${LIBWRAP} ${LIBCRYPT} ${LIBDL} ${SSL_LIBRARIES}
  ${LIBCAP})

ADD_LIBRARY(sql_gis STATIC ${SQL_GIS_SOURCES})
ADD_DEPENDENCIES(sql_gis GenServerSource)
ADD_DEPENDENCIES(sql_gis GenDigestServerSource)
ADD_DEPENDENCIES(sql_gis GenBootstrapPriv)
ADD_DEPENDENCIES(sql_gis GenSysSchema)
ADD_DEPENDENCIES(sql_gis GenError)
TARGET_LINK_LIBRARIES(sql_gis sql_dd sql_main)
SET_TARGET_PROPERTIES(sql_gis PROPERTIES LINK_INTERFACE_MULTIPLICITY 3)

# A class with a reference member lacks a user-defined constructor,
# which can lead to errors.
# Constexpr function return type is not a literal type.
IF(MY_COMPILER_IS_SUNPRO)
  SET_TARGET_PROPERTIES(sql_gis PROPERTIES
    COMPILE_FLAGS "-erroff=refmemnoconstr,w_constexprnonlitret")
  ADD_COMPILE_FLAGS(
    item_geofunc_setops.cc
    COMPILE_FLAGS "-erroff=refmemnoconstr")
ENDIF()

# -g0 is the same as -g, except that inlining is enabled.
# With Studio 12.6 we may get linking errors for large binaries,
# so we add -g0 here to enable inlining of boost/geometry.
# This also cuts link time by almost one hour for large binaries ....
IF(MY_COMPILER_IS_SUNPRO)
  ADD_COMPILE_FLAGS(
    ${SQL_GIS_SOURCES}
    sql_exception_handler.cc
    COMPILE_FLAGS " -g0 -xdebuginfo=no%decl")
ENDIF()

# segfault in boost geometry ....
IF(SOLARIS AND MY_COMPILER_IS_GNU AND
    CMAKE_BUILD_TYPE_UPPER MATCHES "REL")
  ADD_COMPILE_FLAGS(
    ${SQL_GIS_SOURCES}
    COMPILE_FLAGS "-O0"
    )
ENDIF()

ADD_LIBRARY(sql_dd STATIC ${DD_SOURCES})
ADD_DEPENDENCIES(sql_dd GenFixPrivs)
ADD_DEPENDENCIES(sql_dd GenServerSource)
ADD_DEPENDENCIES(sql_dd GenDigestServerSource)
ADD_DEPENDENCIES(sql_dd GenBootstrapPriv)
ADD_DEPENDENCIES(sql_dd GenSysSchema)
ADD_DEPENDENCIES(sql_dd GenError)
ADD_DEPENDENCIES(sql_dd GenKeywordList)
TARGET_LINK_LIBRARIES(sql_dd sql_gis sql_main)
SET_TARGET_PROPERTIES(sql_dd PROPERTIES LINK_INTERFACE_MULTIPLICITY 3)

#
# On Windows platform we compile in the client-side Windows Native Authentication
# plugin which is used by the client connection code included in the server.
#
IF(WIN32)
  ADD_DEFINITIONS(-DAUTHENTICATION_WIN)
  TARGET_LINK_LIBRARIES(sql_main auth_win_client)
  IF(OPENSSL_APPLINK_C)
    ADD_COMPILE_DEFINITIONS(
      ../sql-common/client_authentication.cc
      COMPILE_DEFINITIONS HAVE_OPENSSL_APPLINK_C)
  ENDIF()
ENDIF()

SET(BINLOG_SOURCE
  basic_istream.cc
  basic_ostream.cc
  binlog/global.cc
  binlog/monitoring/context.cc
  binlog/tools/iterators.cc
  binlog.cc
  binlog_istream.cc
  binlog_ostream.cc
  binlog_reader.cc
  log_event.cc
  rpl_commit_stage_manager.cc
  rpl_filter.cc
  rpl_gtid_execution.cc
  rpl_gtid_misc.cc
  rpl_gtid_mutex_cond_array.cc
  rpl_gtid_owned.cc
  rpl_gtid_persist.cc
  rpl_gtid_set.cc
  rpl_gtid_sid_map.cc
  rpl_gtid_specification.cc
  rpl_gtid_state.cc
  rpl_injector.cc
  rpl_log_encryption.cc
  rpl_record.cc
  rpl_table_access.cc
  rpl_trx_tracking.cc
  rpl_utility.cc
  sql_binlog.cc
  stream_cipher.cc
  )
ADD_LIBRARY(binlog STATIC ${BINLOG_SOURCE})
TARGET_LINK_LIBRARIES(binlog binlogevents_static)
TARGET_LINK_LIBRARIES(binlog rpl)

SET (RPL_SOURCE rpl_handler.cc rpl_tblmap.cc rpl_context.cc)
ADD_DEPENDENCIES(binlog GenError)
ADD_LIBRARY(rpl STATIC ${RPL_SOURCE})
TARGET_LINK_LIBRARIES(rpl sql_main)

SET (MASTER_SOURCE rpl_master.cc rpl_binlog_sender.cc binlog_reader.cc)
ADD_DEPENDENCIES(rpl GenError)
ADD_LIBRARY(master STATIC ${MASTER_SOURCE})
ADD_DEPENDENCIES(master GenError)
TARGET_LINK_LIBRARIES(master binlog sql_main)

SET (SLAVE_SOURCE
  dynamic_ids.cc
  rpl_applier_reader.cc
  rpl_channel_service_interface.cc
  rpl_info.cc
  rpl_info_dummy.cc
  rpl_info_factory.cc
  rpl_info_file.cc
  rpl_info_handler.cc
  rpl_info_table.cc
  rpl_info_table_access.cc
  rpl_info_values.cc
  rpl_mi.cc
  rpl_msr.cc
  rpl_mts_submode.cc
  rpl_reporting.cc
  rpl_rli.cc
  rpl_rli_pdb.cc
  rpl_slave.cc
  rpl_slave_commit_order_manager.cc
  rpl_slave_until_options.cc
  rpl_trx_boundary_parser.cc
  log_event_wrapper.cc 
  dependency_slave_worker.cc
  )
ADD_LIBRARY(slave STATIC ${SLAVE_SOURCE})
ADD_DEPENDENCIES(slave GenError)
TARGET_LINK_LIBRARIES(slave binlog sql_main)

# All these libraries depend on each other.
TARGET_LINK_LIBRARIES(sql_main rpl master slave binlog)
IF(WITH_ICU STREQUAL "bundled")
  TARGET_LINK_LIBRARIES(sql_main ${ICU_LIBRARIES})
ENDIF()

IF(WIN32)
  SET(MYSQLD_SOURCE main.cc nt_servc.cc nt_servc.h message.rc)
ELSE()
  SET(MYSQLD_SOURCE main.cc)
ENDIF()

IF(HAVE_BUILD_INFO)
  SET(MYSQLD_SOURCE ${MYSQLD_SOURCE} ${CMAKE_CURRENT_BINARY_DIR}/mysqld.build_info.c)
ENDIF()

LIST(APPEND MYSQLD_SOURCE partitioning/partition_base.cc)

MYSQL_ADD_EXECUTABLE(mysqld
  ${MYSQLD_SOURCE} DESTINATION ${INSTALL_SBINDIR} COMPONENT Server
  ) 

# We depend on protobuf because of the mysqlx plugin.
IF(UNIX_INSTALL_RPATH_ORIGIN_PRIV_LIBDIR)
  ADD_INSTALL_RPATH_FOR_PROTOBUF(mysqld)
ENDIF()

OPTION(DEBUG_EXTNAME "Build server as mysqld-debug (debug builds only)" OFF)
MARK_AS_ADVANCED(DEBUG_EXTNAME)

IF(DEBUG_EXTNAME)
  SET_TARGET_PROPERTIES(mysqld PROPERTIES DEBUG_OUTPUT_NAME "mysqld-debug")
ENDIF()

# macOS has problems with enabling LTO for shared libs, so we only set it
# as target property for mysqld.
IF(APPLE AND WITH_LTO)
  SET_TARGET_PROPERTIES(mysqld PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
ENDIF()

IF(APPLE)
  # Add CoreServices framework since some dloadable plugins may need it
  FIND_LIBRARY(CORESERVICES NAMES CoreServices)
  IF(CORESERVICES)
    TARGET_LINK_LIBRARIES(mysqld ${CORESERVICES})
  ENDIF()
  IF(NOT BUILD_IS_SINGLE_CONFIG)
    ADD_COMPILE_DEFINITIONS(mysqld.cc COMPILE_DEFINITIONS APPLE_XCODE)
  ENDIF()
ENDIF()

SET_TARGET_PROPERTIES(mysqld PROPERTIES ENABLE_EXPORTS TRUE)
GET_TARGET_PROPERTY(mysqld_link_flags mysqld LINK_FLAGS)
IF(NOT mysqld_link_flags)
  SET(mysqld_link_flags)
ENDIF()

IF(UNIX_INSTALL_RPATH_ORIGIN_PRIV_LIBDIR OR LINUX_INSTALL_RPATH_ORIGIN)
  # Because of INSTALL_DEBUG_TARGET which does INSTALL(FILES ...)
  IF(UNIX AND NOT APPLE AND CMAKE_BUILD_TYPE_UPPER STREQUAL "DEBUG")
    STRING_APPEND(mysqld_link_flags
      " -Wl,-rpath,'\$ORIGIN/../${INSTALL_PRIV_LIBDIR}'")
    SET_TARGET_PROPERTIES(mysqld
      PROPERTIES LINK_FLAGS "${mysqld_link_flags}")
    MESSAGE(STATUS "LINK_FLAGS for mysqld ${mysqld_link_flags}")
  ENDIF()

  # Insert absolute INSTALL_RPATH before $ORIGIN values.
  # This is done in order to support setuid executables, needed by:
  # https://dev.mysql.com/doc/refman/8.0/en/resource-groups.html
  IF(INSTALL_LAYOUT STREQUAL "DEB" OR INSTALL_LAYOUT STREQUAL "RPM")
    GET_TARGET_PROPERTY(CURRENT_RPATH_mysqld mysqld INSTALL_RPATH)
    LIST(INSERT CURRENT_RPATH_mysqld 0
      "${CMAKE_INSTALL_PREFIX}/${INSTALL_PRIV_LIBDIR}")
    SET_TARGET_PROPERTIES(mysqld
      PROPERTIES INSTALL_RPATH "${CURRENT_RPATH_mysqld}")
  ENDIF()
ENDIF()

IF(MSVC)
  # Set module definition file.
  SET_TARGET_PROPERTIES(mysqld
    PROPERTIES LINK_FLAGS
    "${mysqld_link_flags} /DEF:${CMAKE_CURRENT_BINARY_DIR}/mysqld.def")

  SET(_PLATFORM  x64)
  ADD_CUSTOM_COMMAND(TARGET mysqld PRE_LINK
    COMMAND echo ${_PLATFORM} && cscript ARGS //nologo ${PROJECT_SOURCE_DIR}/win/create_def_file.js
    ${_PLATFORM}
    "$<TARGET_FILE:binlog>"
    "$<TARGET_FILE:binlogevents_static>"
    "$<TARGET_FILE:mysys>"
    "$<TARGET_FILE:mytime>"
    "$<TARGET_FILE:rpl>"
    "$<TARGET_FILE:slave>"
    "$<TARGET_FILE:sql_main>"
    "$<TARGET_FILE:strings>"
    "$<TARGET_FILE:vio>"
    > mysqld.def
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  ADD_DEPENDENCIES(sql_main GenError)
ENDIF()

SET_TARGET_PROPERTIES(mysqld PROPERTIES ENABLE_EXPORTS TRUE)
TARGET_LINK_LIBRARIES(mysqld
  sql_main sql_gis binlog rpl master slave sql_dd mysys
  minchassis binlogevents_static ${ICU_LIBRARIES})

# Remove transitive link dependencies when linking plugins
SET_TARGET_PROPERTIES(mysqld PROPERTIES INTERFACE_LINK_LIBRARIES "")

SET(WITH_MYSQLD_LDFLAGS "" CACHE STRING "Additional linker flags for mysqld")
MARK_AS_ADVANCED(WITH_MYSQLD_LDFLAGS)
IF(WITH_MYSQLD_LDFLAGS)
  GET_TARGET_PROPERTY(MYSQLD_LINK_FLAGS mysqld LINK_FLAGS)
  IF(NOT MYSQLD_LINK_FLAGS)
    SET(MYSQLD_LINK_FLAGS)
  ENDIF()
  SET_TARGET_PROPERTIES(mysqld PROPERTIES LINK_FLAGS
     "${MYSQLD_LINK_FLAGS} ${WITH_MYSQLD_LDFLAGS}")
ENDIF()

# See DEBUG_EXTNAME above: mysqld may already have name mysqld-debug,
# and the rename is a no-op. If it is called 'mysqld' we rename it.
INSTALL_DEBUG_TARGET(mysqld
  DESTINATION ${INSTALL_SBINDIR}
  RENAME mysqld-debug${CMAKE_EXECUTABLE_SUFFIX}
)

# Check BISON_VERSION and set BISON_FLAGS_WARNINGS
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/bison.cmake)

IF(BISON_FOUND)
  SET(USE_BISON_RESULTS_FROM_MAKE_DIST_DEFAULT OFF)
ELSE()
  SET(USE_BISON_RESULTS_FROM_MAKE_DIST_DEFAULT ON)
ENDIF()

# bison should be REQUIRED, but we have to support source tarball build.
# https://dev.mysql.com/doc/refman/8.0/en/source-installation.html
OPTION(USE_BISON_RESULTS_FROM_MAKE_DIST
  "Use bison results from source distribution tarball"
  ${USE_BISON_RESULTS_FROM_MAKE_DIST_DEFAULT}
  )

IF(USE_BISON_RESULTS_FROM_MAKE_DIST)
  # No bison found, see if generated files are available as sources.
  FOREACH(genfile sql_yacc.h sql_yacc.cc sql_hints.yy.h sql_hints.yy.cc)
    IF(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${genfile}")
      MESSAGE(FATAL_ERROR "Cannot find ${CMAKE_CURRENT_SOURCE_DIR}/${genfile}")
    ENDIF()
    ADD_CUSTOM_COMMAND(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${genfile}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/${genfile}
        ${CMAKE_CURRENT_BINARY_DIR}/${genfile}
      COMMENT "Copying file ${genfile}"
      VERBATIM
      )
  ENDFOREACH()
ELSE()
  BISON_TARGET(mysql_parser
    sql_yacc.yy
    ${CMAKE_CURRENT_BINARY_DIR}/sql_yacc.cc
    COMPILE_FLAGS
    "--name-prefix=MYSQL --yacc ${BISON_FLAGS_WARNINGS} ${BISON_NO_LINE_OPT}"
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/sql_yacc.h
    )

  BISON_TARGET(hints_parser
    sql_hints.yy
    ${CMAKE_CURRENT_BINARY_DIR}/sql_hints.yy.cc
    COMPILE_FLAGS
    "--name-prefix=HINT_PARSER_ --yacc ${BISON_FLAGS_WARNINGS} ${BISON_NO_LINE_OPT}"
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/sql_hints.yy.h
    )

  MESSAGE(STATUS "BISON outputs ${BISON_mysql_parser_OUTPUTS}")
  MESSAGE(STATUS "BISON outputs ${BISON_hints_parser_OUTPUTS}")

  # Some targets, for easier debugging.
  ADD_CUSTOM_TARGET(GenBison_mysql DEPENDS ${BISON_mysql_parser_OUTPUTS})
  ADD_CUSTOM_TARGET(GenBison_hints DEPENDS ${BISON_hints_parser_OUTPUTS})

ENDIF()

SET_SOURCE_FILES_PROPERTIES(
  ${CMAKE_CURRENT_BINARY_DIR}/sql_yacc.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/sql_digest.cc
  PROPERTIES OBJECT_DEPENDS ${GEN_DIGEST_SOURCES}
)

# Gen_lex_hash
MYSQL_ADD_EXECUTABLE(gen_lex_hash gen_lex_hash.cc SKIP_INSTALL)

IF(HAS_MISSING_PROFILE)
  ADD_COMPILE_FLAGS(gen_lex_hash.cc COMPILE_FLAGS ${HAS_MISSING_PROFILE})
ENDIF()

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lex_hash.h
  COMMAND gen_lex_hash > lex_hash.h
  DEPENDS gen_lex_hash
)

MYSQL_ADD_EXECUTABLE(mysql_tzinfo_to_sql
  tztime.cc
  DEPENDENCIES GenError
  LINK_LIBRARIES mysys
  )
SET_TARGET_PROPERTIES(mysql_tzinfo_to_sql
  PROPERTIES COMPILE_FLAGS "-DTZINFO2SQL")

ADD_CUSTOM_TARGET(
        GenServerSource
        DEPENDS ${GEN_SOURCES}
)

ADD_CUSTOM_TARGET(
        GenDigestServerSource
        DEPENDS ${GEN_DIGEST_SOURCES}
)

ADD_CUSTOM_TARGET(
  GenKeywordList
  DEPENDS ${GEN_KEYWORD_LIST_SOURCES}
)

MYSQL_ADD_PLUGIN(udf_example udf_example.cc
  MODULE_ONLY TEST_ONLY MODULE_OUTPUT_NAME "udf_example")
IF(WIN32)
  SET_TARGET_PROPERTIES(udf_example
    PROPERTIES LINK_FLAGS
    "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/udf_example.def")
ENDIF()

MYSQL_ADD_PLUGIN(locking_service locking_service_udf.cc
  MODULE_ONLY MODULE_OUTPUT_NAME "locking_service")
IF(WIN32)
  SET_TARGET_PROPERTIES(locking_service
    PROPERTIES LINK_FLAGS
    "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/locking_service.def")
ENDIF()

FOREACH(tool gtar tar git)
  STRING(TOUPPER ${tool}  TOOL)
  FIND_PROGRAM(${TOOL}_EXECUTABLE ${tool} DOC "path to the executable")
  MARK_AS_ADVANCED(${TOOL}_EXECUTABLE)
ENDFOREACH()

CONFIGURE_FILE(
  ${CMAKE_SOURCE_DIR}/cmake/make_dist.cmake.in
  ${CMAKE_BINARY_DIR}/make_dist.cmake @ONLY)

ADD_CUSTOM_TARGET(dist
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/make_dist.cmake
  DEPENDS GenBison_mysql GenBison_hints
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

ADD_CUSTOM_TARGET(distclean
  COMMAND ${CMAKE_COMMAND} -E echo  WARNING: distclean target is not functional
  COMMAND ${CMAKE_COMMAND} -E echo  Use 'git clean -fdx' instead
  VERBATIM
  )

ADD_CUSTOM_TARGET(show-dist-name
  COMMAND ${CMAKE_COMMAND} -E echo "${CPACK_PACKAGE_FILE_NAME}"
)

# If the repository was modified (via push or pull) or if the
# mysql_githash.h.in changed rebuild the mysql_githash.h file
SET(GITHASH_HEADER_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/mysql_githash.h.in)
SET(GITHASH_HEADER ${CMAKE_CURRENT_BINARY_DIR}/mysql_githash.h)


SET(GITHASH_DEPENDS ${GITHASH_HEADER_SOURCE})
SET(GITHASH_PERL_OPTS --git_root=${CMAKE_SOURCE_DIR} --file=${GITHASH_HEADER})

IF(DEFINED MYSQL_GITHASH)
  # If the hashes and dates are passed in add them to the parameters for
  # the perl script
  SET(GITHASH_PERL_OPTS ${GITHASH_PERL_OPTS}
                        --mysql_githash=${MYSQL_GITHASH}
                        --mysql_gitdate=${MYSQL_GITDATE}
                        --rocksdb_githash=${ROCKSDB_GITHASH}
                        --rocksdb_gitdate=${ROCKSDB_GITDATE})
ELSE()
  # Otherwise we want to add the .git/logs/HEAD file to the list of
  # dependencies
  SET(GITHASH_DEPENDS ${GITHASH_DEPENDS} ${CMAKE_SOURCE_DIR}/.git/logs/HEAD)
ENDIF()

SET(GITHASH_PERL ${CMAKE_SOURCE_DIR}/cmake/githash.pl)
ADD_CUSTOM_COMMAND(OUTPUT ${GITHASH_HEADER}
  COMMAND ${CMAKE_COMMAND} -E copy ${GITHASH_HEADER_SOURCE} ${GITHASH_HEADER}
  COMMAND ${CMAKE_COMMAND} -E echo ${GITHASH_PERL} ${GITHASH_PERL_OPTS}
  COMMAND perl ${GITHASH_PERL} ${GITHASH_PERL_OPTS}
  DEPENDS ${GITHASH_DEPENDS}
  VERBATIM
)
