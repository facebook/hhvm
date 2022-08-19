/* Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_GROUP_REPLICATION_INCLUDED
#define RPL_GROUP_REPLICATION_INCLUDED

class View_change_log_event;
struct GROUP_REPLICATION_CONNECTION_STATUS_CALLBACKS;
struct GROUP_REPLICATION_GROUP_MEMBERS_CALLBACKS;
struct GROUP_REPLICATION_GROUP_MEMBER_STATS_CALLBACKS;

/*
  Group Replication plugin handler function accessors.
*/
bool is_group_replication_plugin_loaded();

int group_replication_start(char **error_message);
int group_replication_stop(char **error_message);
bool is_group_replication_running();
bool is_group_replication_cloning();
int set_group_replication_retrieved_certification_info(
    View_change_log_event *view_change_event);

bool get_group_replication_connection_status_info(
    const GROUP_REPLICATION_CONNECTION_STATUS_CALLBACKS &callbacks);
bool get_group_replication_group_members_info(
    unsigned int index,
    const GROUP_REPLICATION_GROUP_MEMBERS_CALLBACKS &callbacks);
bool get_group_replication_group_member_stats_info(
    unsigned int index,
    const GROUP_REPLICATION_GROUP_MEMBER_STATS_CALLBACKS &callbacks);
unsigned int get_group_replication_members_number_info();

#endif /* RPL_GROUP_REPLICATION_INCLUDED */
