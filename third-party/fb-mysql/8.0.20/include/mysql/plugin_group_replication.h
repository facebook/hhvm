/* Copyright (c) 2013, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_PLUGIN_GROUP_REPLICATION_INCLUDED
#define MYSQL_PLUGIN_GROUP_REPLICATION_INCLUDED

/**
  @file include/mysql/plugin_group_replication.h
  API for Group Replication plugin. (MYSQL_GROUP_REPLICATION_PLUGIN)
*/

#include <mysql/plugin.h>
#define MYSQL_GROUP_REPLICATION_INTERFACE_VERSION 0x0104

enum enum_group_replication_consistency_level {
  // allow executing reads from newer primary even when backlog isn't applied
  GROUP_REPLICATION_CONSISTENCY_EVENTUAL = 0,
  // hold data reads and writes on the new primary until applies all the backlog
  GROUP_REPLICATION_CONSISTENCY_BEFORE_ON_PRIMARY_FAILOVER = 1,
  GROUP_REPLICATION_CONSISTENCY_BEFORE = 2,
  GROUP_REPLICATION_CONSISTENCY_AFTER = 3,
  GROUP_REPLICATION_CONSISTENCY_BEFORE_AND_AFTER = 4
};

/*
  Callbacks for get_connection_status_info function.

  context field can have NULL value, plugin will always pass it
  through all callbacks, independent of its value.
  Its value will not be used by plugin.

  All callbacks are mandatory.
*/
struct GROUP_REPLICATION_CONNECTION_STATUS_CALLBACKS {
  void *const context;
  void (*set_channel_name)(void *const context, const char &value,
                           size_t length);
  void (*set_group_name)(void *const context, const char &value, size_t length);
  void (*set_source_uuid)(void *const context, const char &value,
                          size_t length);
  void (*set_service_state)(void *const context, bool state);
};

/*
  Callbacks for get_group_members_info function.

  context field can have NULL value, plugin will always pass it
  through all callbacks, independent of its value.
  Its value will not be used by plugin.

  All callbacks are mandatory.
*/
struct GROUP_REPLICATION_GROUP_MEMBERS_CALLBACKS {
  void *const context;
  void (*set_channel_name)(void *const context, const char &value,
                           size_t length);
  void (*set_member_id)(void *const context, const char &value, size_t length);
  void (*set_member_host)(void *const context, const char &value,
                          size_t length);
  void (*set_member_port)(void *const context, unsigned int value);
  void (*set_member_state)(void *const context, const char &value,
                           size_t length);
  void (*set_member_role)(void *const context, const char &value,
                          size_t length);
  void (*set_member_version)(void *const context, const char &value,
                             size_t length);
};

/*
  Callbacks for get_group_member_stats_info function.

  context field can have NULL value, plugin will always pass it
  through all callbacks, independent of its value.
  Its value will not be used by plugin.

  All callbacks are mandatory.
*/
struct GROUP_REPLICATION_GROUP_MEMBER_STATS_CALLBACKS {
  void *const context;
  void (*set_channel_name)(void *const context, const char &value,
                           size_t length);
  void (*set_view_id)(void *const context, const char &value, size_t length);
  void (*set_member_id)(void *const context, const char &value, size_t length);
  void (*set_transactions_committed)(void *const context, const char &value,
                                     size_t length);
  void (*set_last_conflict_free_transaction)(void *const context,
                                             const char &value, size_t length);
  void (*set_transactions_in_queue)(void *const context,
                                    unsigned long long int value);
  void (*set_transactions_certified)(void *const context,
                                     unsigned long long int value);
  void (*set_transactions_conflicts_detected)(void *const context,
                                              unsigned long long int value);
  void (*set_transactions_rows_in_validation)(void *const context,
                                              unsigned long long int value);
  void (*set_transactions_remote_applier_queue)(void *const context,
                                                unsigned long long int value);
  void (*set_transactions_remote_applied)(void *const context,
                                          unsigned long long int value);
  void (*set_transactions_local_proposed)(void *const context,
                                          unsigned long long int value);
  void (*set_transactions_local_rollback)(void *const context,
                                          unsigned long long int value);
};

struct st_mysql_group_replication {
  int interface_version;

  /*
    This function is used to start the group replication.
  */
  int (*start)(char **error_message);
  /*
    This function is used to stop the group replication.
  */
  int (*stop)(char **error_message);
  /*
    This function is used to get the current group replication running status.
  */
  bool (*is_running)();
  /*
    This function is used to get the current group replication status about
    its internal cloning process for data provisioning.
  */
  bool (*is_cloning)();
  /*
   This function initializes conflict checking module with info received
   from group on this member.

   @param info  View_change_log_event with conflict checking info.
  */
  int (*set_retrieved_certification_info)(void *info);

  /*
    This function is used to fetch information for group replication kernel
    stats.

    @param callbacks The set of callbacks and its context used to set the
                     information on caller.

    @note The caller is responsible to free memory from the info structure and
          from all its fields.
  */
  bool (*get_connection_status_info)(
      const GROUP_REPLICATION_CONNECTION_STATUS_CALLBACKS &callbacks);

  /*
    This function is used to fetch information for group replication members.

    @param index     Row index for which information needs to be fetched

    @param callbacks The set of callbacks and its context used to set the
                     information on caller.

    @note The caller is responsible to free memory from the info structure and
          from all its fields.
  */
  bool (*get_group_members_info)(
      unsigned int index,
      const GROUP_REPLICATION_GROUP_MEMBERS_CALLBACKS &callbacks);

  /*
    This function is used to fetch information for group replication members
    statistics.

    @param index     Row index for which information needs to be fetched

    @param callbacks The set of callbacks and its context used to set the
                     information on caller.

    @note The caller is responsible to free memory from the info structure and
          from all its fields.
  */
  bool (*get_group_member_stats_info)(
      unsigned int index,
      const GROUP_REPLICATION_GROUP_MEMBER_STATS_CALLBACKS &callbacks);

  /*
    Get number of group replication members.
  */
  unsigned int (*get_members_number_info)();
};

#endif
