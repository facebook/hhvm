/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MDL_CONTEXT_BACKUP_H
#define MDL_CONTEXT_BACKUP_H

#include <cstring>
#include <map>
#include <memory>

#include "sql/malloc_allocator.h"
#include "sql/mdl.h"

/**
  Class which is used to store MDL locks associated with XA transactions
  in prepared state which clients have disconnected. These locks are not
  associated with any particular THD/thread. Later they will be retrieved
  by the thread which attempts to process the prepared XA transaction.

  This is singleton class. It contains map where each element represents
  MDL_context of disconnected prepared XA transaction and holds related
  metadata locks.
*/

class MDL_context_backup_manager {
 private:
  /**
    Key for uniquely identifying MDL_context in the MDL_context_backup map.
  */
  typedef std::basic_string<uchar> MDL_context_backup_key;

  class MDL_context_backup;

  typedef std::map<
      MDL_context_backup_key, std::unique_ptr<MDL_context_backup>,
      std::less<MDL_context_backup_key>,
      Malloc_allocator<std::pair<const MDL_context_backup_key,
                                 std::unique_ptr<MDL_context_backup>>>>
      Element_map_type;  // Real map type.

  /* Singleton. No explicit Objects */
  MDL_context_backup_manager(PSI_memory_key key);

  /* Singleton Object */
  static MDL_context_backup_manager *m_single;

 public:
  /* Not copyable. */
  MDL_context_backup_manager(const MDL_context_backup_manager &) = delete;

  /* Not assignable. */
  void operator=(const MDL_context_backup_manager &) = delete;

  /**
    Initialize member variables and singleton object
  */

  static bool init();

  /**
    Return singleton object
  */

  static MDL_context_backup_manager &instance();

  /**
    Cleanup and delete singleton object
  */

  static void destroy();

 private:
  /**
    destroy mutex and clear backup map
  */

  ~MDL_context_backup_manager();

  static void init_psi_keys(void);

 public:
  /**
    Create backup from given MDL_context by cloning all transactional
    locks to backup context and adds to backup context manager collection.
    This method is used during user's session disconnect for storing MDL
    locks acquired by prepared XA transaction(s).

    This function does not set error codes beyond what is set by the
    functions it calls.

    @param[in]  context      MDL_context from which backup is created.
    @param[in]  key          Key to identity MDL_context
    @param[in]  keylen       Key Length

    @retval     true         Error, e.g. Fail to create backup object, fail
                             to clone locks.
    @retval     false        Success  or a backup already exist for this key.
  */

  bool create_backup(const MDL_context *context, const uchar *key,
                     const size_t keylen);

  /**
    Create backup MDL_context, process request on it and add to backup context
    manager collection. This method is called during server start up in order
    to acquire MDL locks held by prepared XA transactions existed before server
    shutdown.

    This function does not set error codes beyond what is set by the
    functions it calls.

    @param[in]  mdl_requests      Requests need to be processed and backed up.
    @param[in]  key               Key to identity MDL_context
    @param[in]  keylen            Key Length

    @retval     true            Error, e.g. Fail to create backup object, fail
                                to clone locks.
    @retval     false           Success or a backup already exist for this key.
  */

  bool create_backup(MDL_request_list *mdl_requests, const uchar *key,
                     const size_t keylen);

  /**
    Restore locks from backup to given MDL_context.

    This function does not set error codes beyond what is set by the
    functions it calls.

    @param[out]  mdl_context  MDL_context to which backup is restored.
    @param[in]  key           Key to identity MDL_context
    @param[in]  keylen        Key Length

    @retval     true          Error, e.g. There is no element in the
                              collection matching given key, fail
                              to retore locks.
    @retval     false         Success
  */

  bool restore_backup(MDL_context *mdl_context, const uchar *key,
                      const size_t keylen);

  /**
    Delete backup context and release associated locks.

    @param[in]  key             Key to identity MDL_context
    @param[in]  keylen          Key Length
  */

  void delete_backup(const uchar *key, const size_t keylen);

 private:
  // Collection for holding MDL_context_backup elements
  Element_map_type m_backup_map;

  // Mutex to protect m_backup_map
  mysql_mutex_t m_LOCK_mdl_context_backup;

  /**
    Check for presence of a record with specified key

    @param[in]  key_obj          Key to identity MDL_context

    @return true if there is a record for specified key, else false
  */
  bool check_key_exist(const MDL_context_backup_key &key_obj);
};

#endif  // MDL_CONTEXT_BACKUP_H
