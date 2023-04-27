/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_MSR_H
#define RPL_MSR_H

#include "my_config.h"

#include <stddef.h>
#include <sys/types.h>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "my_dbug.h"
#include "my_psi_config.h"
#include "sql/mysqld.h"                         // key_rwlock_channel_map_lock
#include "sql/rpl_channel_service_interface.h"  // enum_channel_type
#include "sql/rpl_filter.h"
#include "sql/rpl_gtid.h"

class Master_info;

/**
   Maps a channel name to it's Master_info.
*/

// Maps a master info object to a channel name
typedef std::map<std::string, Master_info *> mi_map;
// Maps a channel type to a map of channels of that type.
typedef std::map<int, mi_map> replication_channel_map;
// Maps a replication filter to a channel name.
typedef std::map<std::string, Rpl_filter *> filter_map;

/**
  Class to store all the Master_info objects of a slave
  to access them in the replication code base or performance
  schema replication tables.

  In a Multisourced replication setup, a slave connects
  to several masters (also called as sources). This class
  stores the Master_infos where each Master_info belongs
  to a slave.

  The important objects for a slave are the following:
  i) Master_info and Relay_log_info (slave_parallel_workers == 0)
  ii) Master_info, Relay_log_info and Slave_worker(slave_parallel_workers >0 )

  Master_info is always assosiated with a Relay_log_info per channel.
  So, it is enough to store Master_infos and call the corresponding
  Relay_log_info by mi->rli;

  This class is not yet thread safe. Any part of replication code that
  calls this class member function should always lock the channel_map.

  Only a single global object for a server instance should be created.

  The two important data structures in this class are
  i) C++ std map to store the Master_info pointers with channel name as a key.
    These are the base channel maps.
    @todo Convert to boost after it's introduction.

  ii) C++ std map to store the channel maps with a channel type as its key.
      This map stores slave channel maps, group replication channels or others
  iii) An array of Master_info pointers to access from performance schema
     tables. This array is specifically implemented in a way to make
      a) pfs indices simple i.e a simple integer counter
      b) To avoid recalibration of data structure if master info is deleted.
         * Consider the following high level implementation of a pfs table
            to make a row.
          @code
          highlevel_pfs_funciton()
          {
           while(replication_table_xxxx.rnd_next())
           {
             do stuff;
           }
          }
          @endcode
         However, we lock channel_map lock for every rnd_next(); There is a gap
         where an addition/deletion of a channel would rearrange the map
         making the integer indices of the pfs table point to a wrong value.
         Either missing a row or duplicating a row.

         We solve this problem, by using an array exclusively to use in
         replciation pfs tables, by marking a master_info defeated as 0
         (i.e NULL). A new master info is added to this array at the
         first NULL always.
*/
class Multisource_info {
 private:
  /* Maximum number of channels per slave */
  static const unsigned int MAX_CHANNELS = 256;

  /* A Map that maps, a channel name to a Master_info grouped by channel type */
  replication_channel_map rep_channel_map;

  /* Number of master_infos at the moment*/
  uint current_mi_count;

  /**
    Default_channel for this instance, currently is predefined
    and cannot be modified.
  */
  static const char *default_channel;
  Master_info *default_channel_mi;
  static const char *group_replication_channel_names[];

  /**
    This lock was designed to protect the channel_map from adding or removing
    master_info objects from the map (adding or removing replication channels).
    In fact it also acts like the LOCK_active_mi of MySQL 5.6, preventing two
    replication administrative commands to run in parallel.
  */
  Checkable_rwlock *m_channel_map_lock;

#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE

  /* Array for  replication performance schema related tables */
  Master_info *rpl_pfs_mi[MAX_CHANNELS];

#endif /* WITH_PERFSCHEMA_STORAGE_ENGINE */

  /*
    A empty mi_map to allow Multisource_info::end() to return a
    valid constant value.
  */
  mi_map empty_mi_map;

 public:
  /* Constructor for this class.*/
  Multisource_info() {
    /*
      This class should be a singleton.
      The assert below is to prevent it to be instantiated more than once.
    */
#ifndef DBUG_OFF
    static int instance_count = 0;
    instance_count++;
    DBUG_ASSERT(instance_count == 1);
#endif
    current_mi_count = 0;
    default_channel_mi = nullptr;
#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE
    init_rpl_pfs_mi();
#endif /* WITH_PERFSCHEMA_STORAGE_ENGINE */

    m_channel_map_lock = new Checkable_rwlock(
#ifdef HAVE_PSI_INTERFACE
        key_rwlock_channel_map_lock
#endif
    );
  }

  /* Destructor for this class.*/
  ~Multisource_info() { delete m_channel_map_lock; }

  /**
    Adds the Master_info object to both replication_channel_map and rpl_pfs_mi

    @param[in]  channel_name      channel name
    @param[in]  mi                pointer to master info corresponding
                                  to this channel
    @retval      false       succesfully added
    @retval      true        couldn't add channel
  */
  bool add_mi(const char *channel_name, Master_info *mi);

  /**
    Find the master_info object corresponding to a channel explicitly
    from replication channel_map;
    Return if it exists, otherwise return 0

    @param[in]  channel_name  channel name for the master info object.

    @returns                  pointer to the master info object if exists
                              in the map. Otherwise, NULL;
  */
  Master_info *get_mi(const char *channel_name);

  /**
    Return the master_info object corresponding to the default channel.
    @retval                   pointer to the master info object if exists.
                              Otherwise, NULL;
  */
  Master_info *get_default_channel_mi() {
    m_channel_map_lock->assert_some_lock();
    return default_channel_mi;
  }

  /**
    Remove the entry corresponding to the channel, from the
    replication_channel_map and sets index in the  multisource_mi to 0;
    And also delete the {mi, rli} pair corresponding to this channel

    @param[in]    channel_name     Name of the channel for a Master_info
                                   object which must exist.
  */
  void delete_mi(const char *channel_name);

  /**
    Get the default channel for this multisourced_slave;
  */
  inline const char *get_default_channel() { return default_channel; }

  /**
    Get the number of instances of Master_info in the map.

    @param all  If it should count all channels.
                If false, only slave channels are counted.

    @return The number of channels or 0 if empty.
  */
  inline size_t get_num_instances(bool all = false) {
    DBUG_TRACE;

    m_channel_map_lock->assert_some_lock();

    replication_channel_map::iterator map_it;

    if (all) {
      size_t count = 0;

      for (map_it = rep_channel_map.begin(); map_it != rep_channel_map.end();
           map_it++) {
        count += map_it->second.size();
      }
      return count;
    } else  // Return only the slave channels
    {
      map_it = rep_channel_map.find(SLAVE_REPLICATION_CHANNEL);

      if (map_it == rep_channel_map.end())
        return 0;
      else
        return map_it->second.size();
    }
  }

  /**
    Get max channels allowed for this map.
  */
  inline uint get_max_channels() { return MAX_CHANNELS; }

  /**
    Returns true if the current number of channels in this slave
    is less than the MAX_CHANNLES
  */
  inline bool is_valid_channel_count() {
    m_channel_map_lock->assert_some_lock();
    bool is_valid = current_mi_count < MAX_CHANNELS;
    DBUG_EXECUTE_IF("max_replication_channels_exceeded", is_valid = false;);
    return (is_valid);
  }

  /**
    Returns if a channel name is one of the reserved group replication names

    @param channel    the channel name to test
    @param is_applier compare only with applier name

    @retval      true   the name is a reserved name
    @retval      false  non reserved name
  */
  bool is_group_replication_channel_name(const char *channel,
                                         bool is_applier = false);

  /**
     Forward iterators to initiate traversing of a map.

     @todo: Not to expose iterators. But instead to return
            only Master_infos or create generators when
            c++11 is introduced.
  */
  mi_map::iterator begin(
      enum_channel_type channel_type = SLAVE_REPLICATION_CHANNEL) {
    replication_channel_map::iterator map_it;
    map_it = rep_channel_map.find(channel_type);

    if (map_it != rep_channel_map.end()) {
      return map_it->second.begin();
    }

    return end(channel_type);
  }

  mi_map::iterator end(
      enum_channel_type channel_type = SLAVE_REPLICATION_CHANNEL) {
    replication_channel_map::iterator map_it;
    map_it = rep_channel_map.find(channel_type);

    if (map_it != rep_channel_map.end()) {
      return map_it->second.end();
    }

    return empty_mi_map.end();
  }

 private:
#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE

  /* Initialize the rpl_pfs_mi array to NULLs */
  inline void init_rpl_pfs_mi() {
    for (uint i = 0; i < MAX_CHANNELS; i++) rpl_pfs_mi[i] = nullptr;
  }

  /**
     Add a master info pointer to the rpl_pfs_mi array at the first
     NULL;

     @param[in]        mi        master info object to be added.

     @return                     false if success.Else true.
  */
  bool add_mi_to_rpl_pfs_mi(Master_info *mi);

  /**
     Get the index of the master info corresponding to channel name
     from the rpl_pfs_mi array.
     @param[in]       channel_name     Channel name to get the index from

     @return         index of mi for the channel_name. Else -1;
  */
  int get_index_from_rpl_pfs_mi(const char *channel_name);

 public:
  /**
    Used only by replication performance schema indices to get the master_info
    at the position 'pos' from the rpl_pfs_mi array.

    @param[in]   pos   the index in the rpl_pfs_mi array

    @retval            pointer to the master info object at pos 'pos';
  */
  Master_info *get_mi_at_pos(uint pos);
#endif /*WITH_PERFSCHEMA_STORAGE_ENGINE */

  /**
    Acquire the read lock.
  */
  inline void rdlock() { m_channel_map_lock->rdlock(); }

  /**
    Acquire the write lock.
  */
  inline void wrlock() { m_channel_map_lock->wrlock(); }

  /**
    Release the lock (whether it is a write or read lock).
  */
  inline void unlock() { m_channel_map_lock->unlock(); }

  /**
    Assert that some thread holds either the read or the write lock.
  */
  inline void assert_some_lock() const {
    m_channel_map_lock->assert_some_lock();
  }

  /**
    Assert that some thread holds the write lock.
  */
  inline void assert_some_wrlock() const {
    m_channel_map_lock->assert_some_wrlock();
  }
};

/**
  The class is a container for all the per-channel filters, both a map of
  Rpl_filter objects and a list of Rpl_pfs_filter objects.
  It maintains a filter map which maps a replication filter to a channel
  name. Which is needed, because replication channels are not created and
  channel_map is not filled in when these global and per-channel replication
  filters are evaluated with current code frame.
  In theory, after instantiating all channels from the repository and throwing
  all the warnings about the filters configured for non-existent channels, we
  can forget about its global object rpl_channel_filters and rely only on the
  global and per channel Rpl_filter objects. But to avoid holding the
  channel_map.rdlock() when quering P_S.replication_applier_filters table,
  we keep the rpl_channel_filters. So that we just need to hold the small
  rpl_channel_filters.rdlock() when quering P_S.replication_applier_filters
  table. Many operations (RESET SLAVE [FOR CHANNEL], START SLAVE, INIT SLAVE,
  END SLAVE, CHANGE MASTER TO, FLUSH RELAY LOGS, START CHANNEL, PURGE CHANNEL,
  and so on) hold the channel_map.wrlock().

  There is one instance, rpl_channel_filters, created globally for Multisource
  channel filters. The rpl_channel_filters is created when the server is
  started, destroyed when the server is stopped.
*/
class Rpl_channel_filters {
 private:
  /* Store all replication filters with channel names. */
  filter_map channel_to_filter;
  /* Store all Rpl_pfs_filter objects in the channel_to_filter. */
  std::vector<Rpl_pfs_filter> rpl_pfs_filter_vec;
  /*
    This lock was designed to protect the channel_to_filter from reading,
    adding, or removing its objects from the map. It is used to preventing
    the following commands to run in parallel:
      RESET SLAVE ALL [FOR CHANNEL '<channel_name>']
      CHANGE MASTER TO ... FOR CHANNEL
      SELECT FROM performance_schema.replication_applier_filters

    Please acquire a wrlock when modifying the map structure (RESET SLAVE ALL
    [FOR CHANNEL '<channel_name>'], CHANGE MASTER TO ... FOR CHANNEL).
    Please acqurie a rdlock when querying existing filter(s) (SELECT FROM
    performance_schema.replication_applier_filters).

    Note: To modify the object from the map, please see the protection of
    m_rpl_filter_lock in Rpl_filter.
  */
  Checkable_rwlock *m_channel_to_filter_lock;

 public:
  /**
    Create a new replication filter and add it into a filter map.

    @param channel_name A name of a channel.

    @retval Rpl_filter A pointer to a replication filter, or NULL
                       if we failed to add it into fiter_map.
  */
  Rpl_filter *create_filter(const char *channel_name);
  /**
    Delete the replication filter from the filter map.

    @param rpl_filter A pointer to point to a replication filter.
  */
  void delete_filter(Rpl_filter *rpl_filter);
  /**
    Discard all replication filters if they are not attached to channels.
  */
  void discard_all_unattached_filters();
  /**
     discard filters on group replication channels.
  */
  void discard_group_replication_filters();
  /**
    Get a replication filter of a channel.

    @param channel_name A name of a channel.

    @retval Rpl_filter A pointer to a replication filter, or NULL
                       if we failed to add a replication filter
                       into fiter_map when creating it.
  */
  Rpl_filter *get_channel_filter(const char *channel_name);

#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE

  /**
    This member function is called everytime a filter is created or deleted,
    or its filter rules are changed. Once that happens the PFS view is
    recreated.
  */
  void reset_pfs_view();

  /**
    Used only by replication performance schema indices to get the replication
    filter at the position 'pos' from the rpl_pfs_filter_vec vector.

    @param pos the index in the rpl_pfs_filter_vec vector.

    @retval Rpl_filter A pointer to a Rpl_pfs_filter, or NULL if it
                       arrived the end of the rpl_pfs_filter_vec.
  */
  Rpl_pfs_filter *get_filter_at_pos(uint pos);
  /**
    Used only by replication performance schema indices to get the count
    of replication filters from the rpl_pfs_filter_vec vector.

    @retval the count of the replication filters.
  */
  uint get_filter_count();
#endif /*WITH_PERFSCHEMA_STORAGE_ENGINE */

  /**
    Traverse the filter map, build do_table and ignore_table
    rules to hashes for every filter.

    @retval
      0    OK
    @retval
      -1   Error
  */
  bool build_do_and_ignore_table_hashes();

  /* Constructor for this class.*/
  Rpl_channel_filters() {
    m_channel_to_filter_lock = new Checkable_rwlock(
#ifdef HAVE_PSI_INTERFACE
        key_rwlock_channel_to_filter_lock
#endif
    );
  }

  /* Destructor for this class. */
  ~Rpl_channel_filters() { delete m_channel_to_filter_lock; }

  /**
    Traverse the filter map and free all filters. Delete all objects
    in the rpl_pfs_filter_vec vector and then clear the vector.
  */
  void clean_up() {
    /* Traverse the filter map and free all filters */
    for (filter_map::iterator it = channel_to_filter.begin();
         it != channel_to_filter.end(); it++) {
      if (it->second != nullptr) {
        delete it->second;
        it->second = nullptr;
      }
    }

    rpl_pfs_filter_vec.clear();
  }

  /**
    Acquire the write lock.
  */
  inline void wrlock() { m_channel_to_filter_lock->wrlock(); }

  /**
    Acquire the read lock.
  */
  inline void rdlock() { m_channel_to_filter_lock->rdlock(); }

  /**
    Release the lock (whether it is a write or read lock).
  */
  inline void unlock() { m_channel_to_filter_lock->unlock(); }
};

/* Global object for multisourced slave. */
extern Multisource_info channel_map;

/* Global object for storing per-channel replication filters */
extern Rpl_channel_filters rpl_channel_filters;

static bool inline is_slave_configured() {
  /* Server was started with server_id == 0
     OR
     failure to load slave info repositories because of repository
     mismatch i.e Assume slave had a multisource replication with several
     channels setup with TABLE repository. Then if the slave is restarted
     with FILE repository, we fail to load any of the slave repositories,
     including the default channel one.
     Hence, channel_map.get_default_channel_mi() will return NULL.
  */
  return (channel_map.get_default_channel_mi() != nullptr);
}

#endif /*RPL_MSR_H*/
