/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_msr.h"

#include <string.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"
#include "sql/rpl_mi.h"
#include "sql/rpl_rli.h"  // Relay_log_info

const char *Multisource_info::default_channel = "";
const char *Multisource_info::group_replication_channel_names[] = {
    "group_replication_applier", "group_replication_recovery"};

bool Multisource_info::add_mi(const char *channel_name, Master_info *mi) {
  DBUG_TRACE;

  m_channel_map_lock->assert_some_wrlock();

  mi_map::const_iterator it;
  std::pair<mi_map::iterator, bool> ret;
  bool res = false;

  /* The check of mi exceeding MAX_CHANNELS shall be done in the caller */
  DBUG_ASSERT(current_mi_count < MAX_CHANNELS);

  replication_channel_map::iterator map_it;
  enum_channel_type type = is_group_replication_channel_name(channel_name)
                               ? GROUP_REPLICATION_CHANNEL
                               : SLAVE_REPLICATION_CHANNEL;

  map_it = rep_channel_map.find(type);

  if (map_it == rep_channel_map.end()) {
    std::pair<replication_channel_map::iterator, bool> map_ret =
        rep_channel_map.insert(
            replication_channel_map::value_type(type, mi_map()));

    if (!map_ret.second) return true;

    map_it = rep_channel_map.find(type);
  }

  ret = map_it->second.insert(mi_map::value_type(channel_name, mi));

  /* If a map insert fails, ret.second is false */
  if (!ret.second) return true;

  /* Save the pointer for the default_channel to avoid searching it */
  if (!strcmp(channel_name, get_default_channel())) default_channel_mi = mi;

#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE
  res = add_mi_to_rpl_pfs_mi(mi);
#endif
  current_mi_count++;

  return res;
}

Master_info *Multisource_info::get_mi(const char *channel_name) {
  DBUG_TRACE;

  m_channel_map_lock->assert_some_lock();

  DBUG_ASSERT(channel_name != nullptr);

  mi_map::iterator it;
  replication_channel_map::iterator map_it;

  map_it = rep_channel_map.find(SLAVE_REPLICATION_CHANNEL);
  if (map_it != rep_channel_map.end()) {
    it = map_it->second.find(channel_name);
  }

  if (map_it ==
          rep_channel_map.end() ||  // If not a slave channel, maybe a group one
      it == map_it->second.end()) {
    map_it = rep_channel_map.find(GROUP_REPLICATION_CHANNEL);
    if (map_it == rep_channel_map.end()) {
      return nullptr;
    }
    it = map_it->second.find(channel_name);
    if (it == map_it->second.end()) {
      return nullptr;
    }
  }

  return it->second;
}

void Multisource_info::delete_mi(const char *channel_name) {
  DBUG_TRACE;

  m_channel_map_lock->assert_some_wrlock();

  Master_info *mi = nullptr;
  mi_map::iterator it;

  DBUG_ASSERT(channel_name != nullptr);

  replication_channel_map::iterator map_it;
  map_it = rep_channel_map.find(SLAVE_REPLICATION_CHANNEL);

  if (map_it != rep_channel_map.end()) {
    it = map_it->second.find(channel_name);
  }
  if (map_it ==
          rep_channel_map.end() ||  // If not a slave channel, maybe a group one
      it == map_it->second.end()) {
    map_it = rep_channel_map.find(GROUP_REPLICATION_CHANNEL);
    DBUG_ASSERT(map_it != rep_channel_map.end());

    it = map_it->second.find(channel_name);
    DBUG_ASSERT(it != map_it->second.end());
  }

#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE
  int index = -1;
  /* get the index of mi from rpl_pfs_mi */
  index = get_index_from_rpl_pfs_mi(channel_name);

  DBUG_ASSERT(index != -1);

  /* set the current index to  0  and decrease current_mi_count */
  rpl_pfs_mi[index] = nullptr;
#endif

  current_mi_count--;

  mi = it->second;
  it->second = 0;
  /* erase from the map */
  map_it->second.erase(it);

  if (default_channel_mi == mi) default_channel_mi = nullptr;

  /* delete the master info */
  if (mi) {
    mi->channel_assert_some_wrlock();
    mi->wait_until_no_reference(current_thd);

    if (mi->rli) {
      delete mi->rli;
    }
    delete mi;
  }
}

bool Multisource_info::is_group_replication_channel_name(const char *channel,
                                                         bool is_applier) {
  if (is_applier)
    return !strcmp(channel, group_replication_channel_names[0]);
  else
    return !strcmp(channel, group_replication_channel_names[0]) ||
           !strcmp(channel, group_replication_channel_names[1]);
}

#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE

bool Multisource_info::add_mi_to_rpl_pfs_mi(Master_info *mi) {
  DBUG_TRACE;

  m_channel_map_lock->assert_some_wrlock();

  bool res = true;  // not added

  /* Point to this added mi in the rpl_pfs_mi*/
  for (uint i = 0; i < MAX_CHANNELS; i++) {
    if (rpl_pfs_mi[i] == nullptr) {
      rpl_pfs_mi[i] = mi;
      res = false;  // success
      break;
    }
  }
  return res;
}

int Multisource_info::get_index_from_rpl_pfs_mi(const char *channel_name) {
  m_channel_map_lock->assert_some_lock();

  Master_info *mi = nullptr;
  for (uint i = 0; i < MAX_CHANNELS; i++) {
    mi = rpl_pfs_mi[i];
    if (mi) {
      if (!strcmp(mi->get_channel(), channel_name)) return i;
    }
  }
  return -1;
}

Master_info *Multisource_info::get_mi_at_pos(uint pos) {
  DBUG_TRACE;

  m_channel_map_lock->assert_some_lock();

  if (pos < MAX_CHANNELS) return rpl_pfs_mi[pos];

  return nullptr;
}

Rpl_filter *Rpl_channel_filters::create_filter(const char *channel_name) {
  DBUG_TRACE;

  Rpl_filter *rpl_filter;
  filter_map::iterator it;
  std::pair<filter_map::iterator, bool> ret;

  rpl_filter = new Rpl_filter;

  m_channel_to_filter_lock->wrlock();
  it = channel_to_filter.find(channel_name);
  DBUG_ASSERT(it == channel_to_filter.end());
  ret = channel_to_filter.insert(
      std::pair<std::string, Rpl_filter *>(channel_name, rpl_filter));
#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE
  reset_pfs_view();
#endif /* WITH_PERFSCHEMA_STORAGE_ENGINE */
  m_channel_to_filter_lock->unlock();

  if (DBUG_EVALUATE_IF("simulate_out_of_memory_on_create_filter", 1, 0) ||
      !ret.second) {
    LogErr(ERROR_LEVEL, ER_FAILED_TO_ADD_RPL_FILTER, channel_name);
    my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), 0);
    return nullptr;
  }

  return rpl_filter;
}

void Rpl_channel_filters::delete_filter(Rpl_filter *rpl_filter) {
  DBUG_TRACE;

  /* Traverse the filter map. */
  m_channel_to_filter_lock->wrlock();
  for (filter_map::iterator it = channel_to_filter.begin();
       it != channel_to_filter.end(); it++) {
    if (it->second == rpl_filter) {
      /* Find the replication filter and delete it. */
      delete it->second;
      channel_to_filter.erase(it);
#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE
      reset_pfs_view();
#endif /* WITH_PERFSCHEMA_STORAGE_ENGINE */
      m_channel_to_filter_lock->unlock();
      return;
    }
  }
  m_channel_to_filter_lock->unlock();
}

void Rpl_channel_filters::discard_group_replication_filters() {
  /* Traverse the filter map. */
  m_channel_to_filter_lock->wrlock();

  filter_map::iterator it = channel_to_filter.begin();
  while (it != channel_to_filter.end()) {
    if (channel_map.is_group_replication_channel_name(it->first.c_str())) {
      LogErr(WARNING_LEVEL, ER_PER_CHANNEL_RPL_FILTER_CONF_FOR_GRP_RPL,
             it->first.c_str());
      delete it->second;
      it->second = nullptr;
      it = channel_to_filter.erase(it);
    } else
      it++;
  }
  m_channel_to_filter_lock->unlock();
}

void Rpl_channel_filters::discard_all_unattached_filters() {
  DBUG_TRACE;

  /* Traverse the filter map. */
  m_channel_to_filter_lock->wrlock();
  filter_map::iterator it = channel_to_filter.begin();
  while (it != channel_to_filter.end()) {
    if (it->second->is_attached()) {
      /*
        Do not discard the replication filter if it is attached to a channel.
      */
      it++;
      continue;
    }
    /*
      Discard the replication filter with a warning if it is not attached
      to a channel.
    */
    delete it->second;
    it->second = nullptr;
    LogErr(WARNING_LEVEL, ER_RPL_FILTERS_NOT_ATTACHED_TO_CHANNEL,
           it->first.c_str());
    it = channel_to_filter.erase(it);
  }
  /* Reset the P_S view at the end of server startup */
#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE
  reset_pfs_view();
#endif /* WITH_PERFSCHEMA_STORAGE_ENGINE */
  m_channel_to_filter_lock->unlock();
}

Rpl_filter *Rpl_channel_filters::get_channel_filter(const char *channel_name) {
  DBUG_TRACE;
  filter_map::iterator it;
  Rpl_filter *rpl_filter = nullptr;

  DBUG_ASSERT(channel_name != nullptr);

  m_channel_to_filter_lock->rdlock();
  it = channel_to_filter.find(channel_name);

  if (it == channel_to_filter.end()) {
    m_channel_to_filter_lock->unlock();
    rpl_filter = create_filter(channel_name);
  } else {
    rpl_filter = it->second;
    m_channel_to_filter_lock->unlock();
  }

  return rpl_filter;
}

#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE

void Rpl_channel_filters::reset_pfs_view() {
  DBUG_TRACE;
  m_channel_to_filter_lock->assert_some_wrlock();

  rpl_pfs_filter_vec.clear();

  // Traverse the filter map.
  for (filter_map::iterator it = channel_to_filter.begin();
       it != channel_to_filter.end(); it++) {
    it->second->rdlock();
    it->second->put_filters_into_vector(rpl_pfs_filter_vec, it->first.c_str());
    it->second->unlock();
  }
}

Rpl_pfs_filter *Rpl_channel_filters::get_filter_at_pos(uint pos) {
  DBUG_TRACE;
  m_channel_to_filter_lock->assert_some_rdlock();
  Rpl_pfs_filter *res = nullptr;

  if (pos < rpl_pfs_filter_vec.size()) res = &rpl_pfs_filter_vec[pos];

  return res;
}

uint Rpl_channel_filters::get_filter_count() {
  DBUG_TRACE;
  m_channel_to_filter_lock->assert_some_rdlock();

  return rpl_pfs_filter_vec.size();
}

#endif /*WITH_PERFSCHEMA_STORAGE_ENGINE */

bool Rpl_channel_filters::build_do_and_ignore_table_hashes() {
  DBUG_TRACE;

  /* Traverse the filter map. */
  m_channel_to_filter_lock->rdlock();
  for (filter_map::iterator it = channel_to_filter.begin();
       it != channel_to_filter.end(); it++) {
    if (it->second->build_do_table_hash() ||
        it->second->build_ignore_table_hash()) {
      LogErr(ERROR_LEVEL, ER_FAILED_TO_BUILD_DO_AND_IGNORE_TABLE_HASHES);
      return -1;
    }
  }
  m_channel_to_filter_lock->unlock();

  return false;
}

#endif /*WITH_PERFSCHEMA_STORAGE_ENGINE */

/* There is only one channel_map for the whole server */
Multisource_info channel_map;
/* There is only one rpl_channel_filters for the whole server */
Rpl_channel_filters rpl_channel_filters;
