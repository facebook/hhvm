/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

Without limiting anything contained in the foregoing, this file,
which is part of C Driver for MySQL (Connector/C), is also subject to the
Universal FOSS Exception, version 1.0, a copy of which can be found at
http://oss.oracle.com/licenses/universal-foss-exception.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef _mysql_command_h
#define _mysql_command_h

/**
  @file include/my_command.h
*/

/**
  @enum  enum_server_command

  @brief A list of all MySQL protocol commands.

  These are the top level commands the server can receive
  while it listens for a new command in ::dispatch_command

  @par Warning
  Add new commands to the end of this list, otherwise old
  servers won't be able to handle them as 'unsupported'.
*/
enum enum_server_command {
  /**
    Currently refused by the server. See ::dispatch_command.
    Also used internally to mark the start of a session.
  */
  COM_SLEEP,
  COM_QUIT,       /**< See @ref page_protocol_com_quit */
  COM_INIT_DB,    /**< See @ref page_protocol_com_init_db */
  COM_QUERY,      /**< See @ref page_protocol_com_query */
  COM_FIELD_LIST, /**< Deprecated. See @ref page_protocol_com_field_list */
  COM_CREATE_DB, /**< Currently refused by the server. See ::dispatch_command */
  COM_DROP_DB,   /**< Currently refused by the server. See ::dispatch_command */
  COM_REFRESH,   /**< Deprecated. See @ref page_protocol_com_refresh */
  COM_DEPRECATED_1, /**< Deprecated, used to be COM_SHUTDOWN */
  COM_STATISTICS,   /**< See @ref page_protocol_com_statistics */
  COM_PROCESS_INFO, /**< Deprecated. See @ref page_protocol_com_process_info */
  COM_CONNECT,      /**< Currently refused by the server. */
  COM_PROCESS_KILL, /**< Deprecated. See @ref page_protocol_com_process_kill */
  COM_DEBUG,        /**< See @ref page_protocol_com_debug */
  COM_PING,         /**< See @ref page_protocol_com_ping */
  COM_TIME,         /**< Currently refused by the server. */
  COM_DELAYED_INSERT, /**< Functionality removed. */
  COM_CHANGE_USER,    /**< See @ref page_protocol_com_change_user */
  COM_BINLOG_DUMP,    /**< See @ref page_protocol_com_binlog_dump */
  COM_TABLE_DUMP,
  COM_CONNECT_OUT,
  COM_REGISTER_SLAVE,
  COM_STMT_PREPARE, /**< See @ref page_protocol_com_stmt_prepare */
  COM_STMT_EXECUTE, /**< See @ref page_protocol_com_stmt_execute */
  /** See  @ref page_protocol_com_stmt_send_long_data */
  COM_STMT_SEND_LONG_DATA,
  COM_STMT_CLOSE, /**< See @ref page_protocol_com_stmt_close */
  COM_STMT_RESET, /**< See @ref page_protocol_com_stmt_reset */
  COM_SET_OPTION, /**< See @ref page_protocol_com_set_option */
  COM_STMT_FETCH, /**< See @ref page_protocol_com_stmt_fetch */
  /**
    Currently refused by the server. See ::dispatch_command.
    Also used internally to mark the session as a "daemon",
    i.e. non-client THD. Currently the scheduler and the GTID
    code does use this state.
    These threads won't be killed by `KILL`

    @sa Event_scheduler::start, ::init_thd, ::kill_one_thread,
    ::Find_thd_with_id
  */
  COM_DAEMON,
  COM_BINLOG_DUMP_GTID,
  COM_RESET_CONNECTION, /**< See @ref page_protocol_com_reset_connection */
  COM_CLONE,
  /* don't forget to update const char *command_name[] in sql_parse.cc */

  /* Must be last */
  COM_END, /**< Not a real command. Refused. */
  /*
    The following are Facebook specific commands. They are put at the top end
    to avoid conflicting with upstream.
  */
  COM_TOP_BEGIN = 253,
  COM_SEND_REPLICA_STATISTICS = 254,
  COM_QUERY_ATTRS = 255,
  COM_TOP_END = 256,
};

#endif /* _mysql_command_h */
