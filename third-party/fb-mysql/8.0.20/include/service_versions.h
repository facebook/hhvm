/* Copyright (c) 2009, 2019, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file include/service_versions.h
*/

#ifdef _WIN32
#define SERVICE_VERSION __declspec(dllexport) void *
#else
#define SERVICE_VERSION void *
#endif

#define VERSION_command 0x0100
#define VERSION_thd_alloc 0x0100
#define VERSION_thd_wait 0x0100
#define VERSION_my_thread_scheduler 0x0100
#define VERSION_my_plugin_log 0x0100
#define VERSION_mysql_string 0x0100
#define VERSION_mysql_malloc 0x0100
#define VERSION_mysql_password_policy 0x0100
#define VERSION_parser 0x0100
#define VERSION_rpl_transaction_ctx_service 0x0100
#define VERSION_transaction_write_set_service 0x0100
#define VERSION_security_context_service 0x0101
#define VERSION_locking_service 0x0100
#define VERSION_srv_session_info_service 0x0100
#define VERSION_srv_session_service 0x0100
#define VERSION_mysql_keyring_service 0x0100
#define VERSION_plugin_registry_service 0x100
#define VERSION_clone_protocol_service 0x100
