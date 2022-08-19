/* Copyright (c) 2006, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_CONNECT_INCLUDED
#define SQL_CONNECT_INCLUDED

#include <stddef.h>
#include <sys/types.h>

#include "my_inttypes.h"

class THD;

struct LEX_USER;

/*
  This structure specifies the maximum amount of resources which
  can be consumed by each account. Zero value of a member means
  there is no limit.
*/
typedef struct user_resources {
  /* Maximum number of queries/statements per hour. */
  uint questions;
  /*
     Maximum number of updating statements per hour (which statements are
     updating is defined by sql_command_flags array).
  */
  uint updates;
  /* Maximum number of connections established per hour. */
  uint conn_per_hour;
  /* Maximum number of concurrent connections. */
  uint user_conn;
  /*
     Values of this enum and specified_limits member are used by the
     parser to store which user limits were specified in GRANT statement.
  */
  enum {
    QUERIES_PER_HOUR = 1,
    UPDATES_PER_HOUR = 2,
    CONNECTIONS_PER_HOUR = 4,
    USER_CONNECTIONS = 8
  };
  uint specified_limits;
} USER_RESOURCES;

/*
  This structure is used for counting resources consumed and for checking
  them against specified user limits.
*/
typedef struct user_conn {
  /*
     Pointer to user+host key (pair separated by '\0') defining the entity
     for which resources are counted (By default it is user account thus
     priv_user/priv_host pair is used. If --old-style-user-limits option
     is enabled, resources are counted for each user+host separately).
  */
  char *user;
  /* Pointer to host part of the key. */
  char *host;
  /**
     The moment of time when per hour counters were reset last time
     (i.e. start of "hour" for conn_per_hour, updates, questions counters).
  */
  ulonglong reset_utime;
  /* Total length of the key. */
  size_t len;
  /* Current amount of concurrent connections for this account. */
  uint connections;
  /*
     Current number of connections per hour, number of updating statements
     per hour and total number of statements per hour for this account.
  */
  uint conn_per_hour, updates, questions;
  /* Maximum amount of resources which account is allowed to consume. */
  USER_RESOURCES user_resources;
} USER_CONN;

void init_max_user_conn(void);
void free_max_user_conn(void);
void reset_mqh(THD *thd, LEX_USER *lu, bool get_them);
bool check_mqh(THD *thd, uint check_command);
void decrease_user_connections(USER_CONN *uc);
void release_user_connection(THD *thd);
bool thd_init_client_charset(THD *thd, uint cs_number);
bool thd_prepare_connection(THD *thd);
void close_connection(THD *thd, uint sql_errno = 0,
                      bool server_shutdown = false, bool generate_event = true);
bool thd_connection_alive(THD *thd);
void end_connection(THD *thd);
int get_or_create_user_conn(THD *thd, const char *user, const char *host,
                            const USER_RESOURCES *mqh);
int check_for_max_user_connections(THD *thd, const USER_CONN *uc);

#endif /* SQL_CONNECT_INCLUDED */
