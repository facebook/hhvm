/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_info.h"

#include "m_string.h"  // strmake
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "thr_mutex.h"

Rpl_info::Rpl_info(const char *type,
#ifdef HAVE_PSI_INTERFACE
                   PSI_mutex_key *param_key_info_run_lock,
                   PSI_mutex_key *param_key_info_data_lock,
                   PSI_mutex_key *param_key_info_sleep_lock,
                   PSI_mutex_key *param_key_info_thd_lock,
                   PSI_mutex_key *param_key_info_data_cond,
                   PSI_mutex_key *param_key_info_start_cond,
                   PSI_mutex_key *param_key_info_stop_cond,
                   PSI_mutex_key *param_key_info_sleep_cond,
#endif
                   uint param_id, const char *param_channel)
    : Slave_reporting_capability(type),
#ifdef HAVE_PSI_INTERFACE
      key_info_run_lock(param_key_info_run_lock),
      key_info_data_lock(param_key_info_data_lock),
      key_info_sleep_lock(param_key_info_sleep_lock),
      key_info_thd_lock(param_key_info_thd_lock),
      key_info_data_cond(param_key_info_data_cond),
      key_info_start_cond(param_key_info_start_cond),
      key_info_stop_cond(param_key_info_stop_cond),
      key_info_sleep_cond(param_key_info_sleep_cond),
#endif
      info_thd(nullptr),
      inited(false),
      abort_slave(false),
      slave_running(0),
      slave_run_id(0),
      handler(nullptr),
      internal_id(param_id) {
#ifdef HAVE_PSI_INTERFACE
  mysql_mutex_init(*key_info_run_lock, &run_lock, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(*key_info_data_lock, &data_lock, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(*key_info_sleep_lock, &sleep_lock, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(*key_info_thd_lock, &info_thd_lock, MY_MUTEX_INIT_FAST);
  mysql_cond_init(*key_info_data_cond, &data_cond);
  mysql_cond_init(*key_info_start_cond, &start_cond);
  mysql_cond_init(*key_info_stop_cond, &stop_cond);
  mysql_cond_init(*key_info_sleep_cond, &sleep_cond);
#else
  mysql_mutex_init(nullptr, &run_lock, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(nullptr, &data_lock, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(nullptr, &sleep_lock, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(nullptr, &info_thd_lock, MY_MUTEX_INIT_FAST);
  mysql_cond_init(nullptr, &data_cond);
  mysql_cond_init(nullptr, &start_cond);
  mysql_cond_init(nullptr, &stop_cond);
  mysql_cond_init(nullptr, &sleep_cond);
#endif

  if (param_channel)
    strmake(channel, param_channel, sizeof(channel) - 1);
  else
    /*create a default empty channel*/
    strmake(channel, "", sizeof(channel) - 1);
}

Rpl_info::~Rpl_info() {
  delete handler;

  mysql_mutex_destroy(&run_lock);
  mysql_mutex_destroy(&data_lock);
  mysql_mutex_destroy(&sleep_lock);
  mysql_mutex_destroy(&info_thd_lock);
  mysql_cond_destroy(&data_cond);
  mysql_cond_destroy(&start_cond);
  mysql_cond_destroy(&stop_cond);
  mysql_cond_destroy(&sleep_cond);
}
