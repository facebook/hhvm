# Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.
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

IF(NOT WITH_MYSQLX)
  RETURN()
ENDIF()

INCLUDE(CTest)

OPTION(MYSQLX_ADDITIONAL_TESTS_ENABLE "MYSQLX - enable additional tests" OFF)

SET(MYSQLX_PROJECT_DIR "${PROJECT_SOURCE_DIR}/plugin/x")

GET_DIRECTORY_PROPERTY(MYSQLX_CLIENT_LIB
  DIRECTORY ${MYSQLX_PROJECT_DIR}
  DEFINITION MYSQLX_CLIENT_LIB
)

GET_DIRECTORY_PROPERTY(MYSQLX_PLUGIN_INCLUDE_DIRS
  DIRECTORY ${MYSQLX_PROJECT_DIR} INCLUDE_DIRECTORIES
)

# Protobuf 3 will generate #if PROTOBUF_INLINE_NOT_IN_HEADERS
# which may fail with -Werror=undef
ADD_DEFINITIONS(-DPROTOBUF_INLINE_NOT_IN_HEADERS=0 -DXPLUGIN_DISABLE_LOG)
# Protobuf 3.7.0 will generate #if 3007000 < PROTOBUF_MIN_PROTOC_VERSION
IF(PROTOBUF_VERSION_NUMBER MATCHES "3007000")
  ADD_DEFINITIONS(-DPROTOBUF_MIN_PROTOC_VERSION=2006000)
ENDIF()

MY_INCLUDE_SYSTEM_DIRECTORIES(PROTOBUF)

INCLUDE_DIRECTORIES(
  ${MYSQLX_PLUGIN_INCLUDE_DIRS}
)

SET(XPL_UNIT_TESTS xplugin_unit_tests)

SET(XPL_TEST_SRC
  test_main.cc

  xpl/stubs/command_service.cc
  xpl/stubs/log.cc
  xpl/stubs/log_subsystem.cc
  xpl/stubs/misc.cc
  xpl/stubs/pfs.cc
  xpl/stubs/plugin.cc
  xpl/stubs/security_context_service.cc
  xpl/stubs/sql_session_service.cc

  xpl/mysqlx_pb_wrapper.cc

  xpl/account_verification_handler_t.cc
  xpl/admin_cmd_arguments_object_t.cc
  xpl/admin_cmd_index_t.cc
  xpl/admin_create_collection_t.cc
  xpl/admin_get_collection_options_t.cc
  xpl/admin_modify_collection_options_t.cc
  xpl/broker_input_queue_task_t.cc
  xpl/callback_command_delegate_t.cc
  xpl/capabilities_configurator_t.cc
  xpl/capabilities_handlers_t.cc
  xpl/crud_statement_builder_t.cc
  xpl/cursor_t.cc
  xpl/delete_statement_builder_t.cc
  xpl/document_id_generator_t.cc
  xpl/expect_noerror_t.cc
  xpl/expr_generator_t.cc
  xpl/find_statement_builder_t.cc
  xpl/getter_any_t.cc
  xpl/index_array_field_t.cc
  xpl/index_field_t.cc
  xpl/insert_statement_builder_t.cc
  xpl/json_utils_t.cc
  xpl/listener_tcp_t.cc
  xpl/listener_unix_socket_t.cc
  xpl/message_builder_t.cc
  xpl/meta_schema_validator_t.cc
  xpl/prepared_statement_builder_t.cc
  xpl/prepare_param_handler_t.cc
  xpl/protocol_decoder_t.cc
  xpl/query_string_builder_t.cc
  xpl/row_builder_t.cc
  xpl/sasl_authentication_t.cc
  xpl/sasl_challenge_response_auth_t.cc
  xpl/sasl_plain_auth_t.cc
  xpl/scheduler_t.cc
  xpl/server_client_timeout_t.cc
  xpl/set_variable_adaptor_t.cc
  xpl/sha256_cache_t.cc
  xpl/socket_acceptor_task_t.cc
  xpl/sql_statement_builder_t.cc
  xpl/sync_variable_t.cc
  xpl/timeouts_t.cc
  xpl/update_statement_builder_t.cc
  xpl/user_password_verification_t.cc
  xpl/view_statement_builder_t.cc
  xpl/xdatetime_t.cc
  xpl/xdecimal_t.cc
  xpl/xmessage_buffer_t.cc
  xpl/xrow_buffer_t.cc
  xpl/xpl_regex_t.cc
)


IF(MYSQLX_ADDITIONAL_TESTS_ENABLE)
  INCLUDE(mysql_function_names.cmake)
  LIST(APPEND XPL_TEST_SRC "${MYSQLX_GENERATE_DIR}/mysql_function_names_t.cc")
ENDIF()


MYSQL_ADD_EXECUTABLE(${XPL_UNIT_TESTS}
  ${XPL_TEST_SRC}
  ADD_TEST xplugin
)

ADD_COMPILE_FLAGS(${XPL_TEST_SRC}
  COMPILE_FLAGS "${MYSQLX_PROTOCOL_FLAGS}"
)

IF(DISABLE_MYSQLX_PROTOBUF_LITE)
  ADD_COMPILE_DEFINITIONS(${XPL_TEST_SRC}
    COMPILE_DEFINITIONS USE_MYSQLX_FULL_PROTO)
ENDIF()

ADD_DEPENDENCIES(${XPL_UNIT_TESTS}
  mysqlx
)

# New warning in Visual Studio 2008
# '%$S': virtual function overrides '%$pS', previous versions of the compiler
# did not override when parameters only differed by const/volatile qualifiers
IF(WIN32)
  SET_TARGET_PROPERTIES(${XPL_UNIT_TESTS} PROPERTIES COMPILE_FLAGS "/wd4373")
ENDIF(WIN32)

IF(MY_COMPILER_IS_CLANG)
  STRING_APPEND(CMAKE_CXX_FLAGS " -Wno-deprecated")
ENDIF()

# assertion failed in function assert_with_dumps() @ ssa.c:621 when using -xO3
# Segfault in the unit test, so lower optimization level there as well
IF(MY_COMPILER_IS_SUNPRO)
  ADD_COMPILE_FLAGS(
    ${CMAKE_CURRENT_SOURCE_DIR}/xpl/callback_command_delegate_t.cc
    COMPILE_FLAGS "-xO2")
ENDIF()

TARGET_LINK_LIBRARIES(${XPL_UNIT_TESTS}
  ${GCOV_LDFLAGS}
  ${MYSQLX_CLIENT_LIB}
  mysqlx
  mysqlclient
  gtest
  gmock
)


# --== XClient unit tests (always full version) ==--

GET_DIRECTORY_PROPERTY(MYSQLX_CLIENT_FULL_LIB
  DIRECTORY ${MYSQLX_PROJECT_DIR}
  DEFINITION MYSQLX_CLIENT_FULL_LIB
)

GET_DIRECTORY_PROPERTY(MYSQLX_PROTOCOL_FULL_LIB
  DIRECTORY ${MYSQLX_PROJECT_DIR}
  DEFINITION MYSQLX_PROTOCOL_FULL_LIB
)

SET(XCL_UNIT_TESTS xclient_unit_tests)

SET(XCL_SRC
  test_main.cc
  xcl/auth_chaining_t.cc
  xcl/connection_general_t.cc
  xcl/protocol_auth_t.cc
  xcl/protocol_execute_t.cc
  xcl/protocol_global_error_t.cc
  xcl/protocol_notices_t.cc
  xcl/protocol_send_recv_t.cc
  xcl/query_t.cc
  xcl/session_capability_t.cc
  xcl/session_connect_t.cc
  xcl/session_execute_t.cc
  xcl/session_general_t.cc
  xcl/session_options_t.cc
  xcl/session_negotiation_t.cc
  xcl/sha256_scramble_t.cc
  xcl/ssl_config_t.cc
  xcl/xpriority_list_t.cc
)

ADD_COMPILE_DEFINITIONS(${XCL_SRC}
  COMPILE_DEFINITIONS USE_MYSQLX_FULL_PROTO
)

MYSQL_ADD_EXECUTABLE(${XCL_UNIT_TESTS}
  ${XCL_SRC}
  ADD_TEST xclient
)

ADD_DEPENDENCIES(${XCL_UNIT_TESTS}
  ${MYSQLX_CLIENT_FULL_LIB}
  ${MYSQLX_PROTOCOL_FULL_LIB}
  xprotocol_tags
)

TARGET_LINK_LIBRARIES(${XCL_UNIT_TESTS}
  ${GCOV_LDFLAGS}
  ${MYSQLX_CLIENT_FULL_LIB}
  ${MYSQLX_PROTOCOL_FULL_LIB}
  ${PROTOBUF_LIBRARY}
  ${LZ4_LIBRARY}
  mysqlclient
  gtest
  gmock
)
