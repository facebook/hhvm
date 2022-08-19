/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/log_resource.h"
#include "sql/json_dom.h"

int MY_ATTRIBUTE((visibility("default")))
    Log_resource::dummy_function_to_ensure_we_are_linked_into_the_server() {
  return 1;
}

void Log_resource_mi_wrapper::lock() { mysql_mutex_lock(&mi->data_lock); }

void Log_resource_mi_wrapper::unlock() { mysql_mutex_unlock(&mi->data_lock); }

bool Log_resource_mi_wrapper::collect_info() {
  bool error = false;

  mysql_mutex_assert_owner(&mi->data_lock);

  Json_array *json_replication = static_cast<Json_array *>(get_json());

  Json_string json_channel_name(mi->get_channel());

  LOG_INFO log_info;
  mi->get_flushed_relay_log_info(&log_info);
  size_t dir_len = dirname_length(log_info.log_file_name);
  Json_string json_log_file(log_info.log_file_name + dir_len);
  Json_int json_log_pos(log_info.pos);

  Json_object json_channel;
  error = json_channel.add_clone("channel_name",
                                 (const Json_dom *)&json_channel_name);
  if (!error) error = json_channel.add_clone("relay_log_file", &json_log_file);
  if (!error)
    error = json_channel.add_clone("relay_log_position", &json_log_pos);
  if (!error) json_replication->append_clone(&json_channel);

  return error;
}

void Log_resource_binlog_wrapper::lock() {
  mysql_mutex_lock(binlog->get_log_lock());
}

void Log_resource_binlog_wrapper::unlock() {
  mysql_mutex_unlock(binlog->get_log_lock());
}

bool Log_resource_binlog_wrapper::collect_info() {
  bool error = false;

  mysql_mutex_assert_owner(binlog->get_log_lock());

  if (binlog->is_open()) {
    Json_object *json_local = static_cast<Json_object *>(get_json());

    LOG_INFO log_info;
    binlog->get_current_log(&log_info, false);
    size_t dir_len = dirname_length(log_info.log_file_name);
    Json_string json_log_file(log_info.log_file_name + dir_len);
    Json_int json_log_pos(log_info.pos);

    error = json_local->add_clone("binary_log_file", &json_log_file);
    if (!error) json_local->add_clone("binary_log_position", &json_log_pos);
  }
  return error;
}

void Log_resource_gtid_state_wrapper::lock() { global_sid_lock->wrlock(); }

void Log_resource_gtid_state_wrapper::unlock() { global_sid_lock->unlock(); }

bool Log_resource_gtid_state_wrapper::collect_info() {
  bool error = false;
  global_sid_lock->assert_some_wrlock();

  char *gtid_executed_string;
  Json_object *json_local = static_cast<Json_object *>(get_json());
  int len = gtid_state->get_executed_gtids()->to_string(&gtid_executed_string);
  if (!(error = (len < 0))) {
    Json_string json_gtid_executed(gtid_executed_string);
    error = json_local->add_clone("gtid_executed", &json_gtid_executed);
  }
  my_free(gtid_executed_string);
  return error;
}

void Log_resource_hton_wrapper::lock() {
  if (hton->lock_hton_log) hton->lock_hton_log(hton);
}

void Log_resource_hton_wrapper::unlock() {
  if (hton->unlock_hton_log) hton->unlock_hton_log(hton);
}

bool Log_resource_hton_wrapper::collect_info() {
  bool result = false;
  if (hton->collect_hton_log_info) {
    result = hton->collect_hton_log_info(hton, get_json());
  }
  return result;
}

Log_resource *Log_resource_factory::get_wrapper(Master_info *mi,
                                                Json_dom *json) {
  Log_resource_mi_wrapper *wrapper;
  wrapper = new Log_resource_mi_wrapper(mi, json);
  return wrapper;
}

Log_resource *Log_resource_factory::get_wrapper(MYSQL_BIN_LOG *binlog,
                                                Json_dom *json) {
  Log_resource_binlog_wrapper *wrapper;
  wrapper = new Log_resource_binlog_wrapper(binlog, json);
  return wrapper;
}

Log_resource *Log_resource_factory::get_wrapper(Gtid_state *gtid_state,
                                                Json_dom *json) {
  Log_resource_gtid_state_wrapper *wrapper;
  wrapper = new Log_resource_gtid_state_wrapper(gtid_state, json);
  return wrapper;
}

Log_resource *Log_resource_factory::get_wrapper(handlerton *hton,
                                                Json_dom *json) {
  Log_resource_hton_wrapper *wrapper;
  wrapper = new Log_resource_hton_wrapper(hton, json);
  return wrapper;
}
