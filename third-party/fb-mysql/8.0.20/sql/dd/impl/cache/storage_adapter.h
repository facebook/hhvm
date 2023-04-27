/* Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_CACHE__STORAGE_ADAPTER_INCLUDED
#define DD_CACHE__STORAGE_ADAPTER_INCLUDED

#include <stddef.h>

#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_base.h"
#include "sql/dd/cache/object_registry.h"  // Object_registry
#include "sql/dd/object_id.h"
#include "sql/handler.h"  // enum_tx_isolation
#include "thr_mutex.h"

class THD;

namespace dd_cache_unittest {
class CacheStorageTest;
}

namespace dd {

namespace cache {

/**
  Handling of access to persistent storage.

  This class provides static template functions that manipulates an object on
  persistent storage based on the submitted key and object type. There is also
  an object registry instance to keep the core DD objects that are needed to
  handle cache misses for table meta data. The storage adapter owns the objects
  in the core registry. When adding objects to the registry using core_store(),
  the storage adapter will clone the object and take ownership of the clone.
  When retrieving objects from the registry using core_get(), a clone of the
  object will be returned, and this is therefore owned by the caller.
*/

class Storage_adapter {
  friend class dd_cache_unittest::CacheStorageTest;

 private:
  /**
    Use an id not starting at 1 to make it easy to recognize ids generated
    before objects are stored persistently.
  */

  static const Object_id FIRST_OID = 10001;

  /**
    Generate a new object id for a registry partition.

    Simulate an auto increment column. Used when the server is starting,
    while the scaffolding is being built.

    @tparam T      Object registry partition.

    @return Next object id to be used.
  */

  template <typename T>
  Object_id next_oid();

  /**
    Get a dictionary object from core storage.

    A clone of the registry object will be returned, owned by the caller.

    @tparam      K         Key type.
    @tparam      T         Dictionary object type.
    @param       key       Key for which to get the object.
    @param [out] object    Object retrieved, possibly nullptr if not present.
  */

  template <typename K, typename T>
  void core_get(const K &key, const T **object);

  Object_registry m_core_registry;  // Object registry storing core DD objects.
  mysql_mutex_t m_lock;             // Single mutex to protect the registry.
  static bool s_use_fake_storage;   // Whether to use the core registry to
                                    // simulate the storage engine.

  Storage_adapter() {
    mysql_mutex_init(PSI_NOT_INSTRUMENTED, &m_lock, MY_MUTEX_INIT_FAST);
  }

  ~Storage_adapter() {
    mysql_mutex_lock(&m_lock);
    m_core_registry.erase_all();
    mysql_mutex_unlock(&m_lock);
    mysql_mutex_destroy(&m_lock);
  }

 public:
  static Storage_adapter *instance();

  /**
    Get the number of core objects in a registry partition.

    @tparam      T         Dictionary object type.
    @return      Number of elements.
  */

  template <typename T>
  size_t core_size();

  /**
    Get a dictionary object id from core storage.

    @tparam      T         Dictionary object type.
    @param       key       Name key for which to get the object id.
    @return      Object id, INVALID_OBJECT_ID if the object is not present.
  */

  template <typename T>
  Object_id core_get_id(const typename T::Name_key &key);

  /**
     Update the dd object in the core registry.  This is a noop unless
     this member function is overloaded for a given type. See below.
   */
  template <typename T>
  void core_update(const T *) {}

  /**
     Overload of core_update for dd::Tablespace. Currently the core
     registry can only be updated for the DD tablespace when
     encrypting it. A clone of the dd::Tablespace object passed in is
     stored in the registry.

     @param new_tsp the new dd::Tablespace object to keep in the core registry.
  */
  void core_update(const dd::Tablespace *new_tsp);

  /**
    Get a dictionary object from persistent storage.

    Create an access key based on the submitted key, and find the record
    from the appropriate table. Restore the record into a new dictionary
    object.

    @tparam      K         Key type.
    @tparam      T         Dictionary object type.
    @param       thd       Thread context.
    @param       key       Key for which to get the object.
    @param       isolation Isolation level.
    @param       bypass_core_registry If set to true, get the object from the
                                      DD tables. Needed during DD bootstrap.
    @param [out] object    Object retrieved, possibly NULL if not present.

    @retval      false   No error.
    @retval      true    Error.
  */

  template <typename K, typename T>
  static bool get(THD *thd, const K &key, enum_tx_isolation isolation,
                  bool bypass_core_registry, const T **object);

  /**
    Drop a dictionary object from core storage.

    @tparam  T       Dictionary object type.
    @param   thd     Thread context.
    @param   object  Object to be dropped.
  */

  template <typename T>
  void core_drop(THD *thd, const T *object);

  /**
    Drop a dictionary object from persistent storage.

    @tparam  T       Dictionary object type.
    @param   thd     Thread context.
    @param   object  Object to be dropped.

    @retval  false   No error.
    @retval  true    Error.
  */

  template <typename T>
  static bool drop(THD *thd, const T *object);

  /**
    Store a dictionary object to core storage.

    A clone of the submitted object will be added to the core
    storage. The caller is still the owner of the submitted
    objecct.

    @tparam  T       Dictionary object type.
    @param   thd     Thread context.
    @param   object  Object to be stored.
  */

  template <typename T>
  void core_store(THD *thd, T *object);

  /**
    Store a dictionary object to persistent storage.

    @tparam  T       Dictionary object type.
    @param   thd     Thread context.
    @param   object  Object to be stored.

    @retval  false   No error.
    @retval  true    Error.
  */

  template <typename T>
  static bool store(THD *thd, T *object);

  /**
    Sync a dictionary object from persistent to core storage.

    @tparam      T         Dictionary object type.
    @param       thd       Thread context.
    @param       key       Key for object to get from persistent storage.
    @param       object    Object to drop from the core registry.
  */

  template <typename T>
  bool core_sync(THD *thd, const typename T::Name_key &key, const T *object);

  /**
    Remove and delete all elements and objects from core storage.
  */

  void erase_all();

  /**
    Dump the contents of the core storage.
  */

  void dump();
};

}  // namespace cache
}  // namespace dd

#endif  // DD_CACHE__STORAGE_ADAPTER_INCLUDED
