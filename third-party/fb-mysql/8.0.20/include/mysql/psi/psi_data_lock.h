/* Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_PSI_DATA_LOCK_H
#define MYSQL_PSI_DATA_LOCK_H

/**
  @file include/mysql/psi/psi_data_lock.h
  Performance schema instrumentation interface.

  @defgroup psi_abi_data_lock Row Lock Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

#include "my_inttypes.h"
#include "my_sharedlib.h"
#include "psi_base.h"

#ifdef HAVE_PSI_DATA_LOCK_INTERFACE

/**
  @def PSI_DATA_LOCK_VERSION_1
  Performance Schema Row Lock Interface number for version 1.
  This version is supported.
*/
#define PSI_DATA_LOCK_VERSION_1 1

/**
  @def PSI_DATA_LOCK_VERSION_2
  Performance Schema Row Lock Interface number for version 2.
  This version is not implemented, it's a placeholder.
*/
#define PSI_DATA_LOCK_VERSION_2 2

/**
  @def PSI_CURRENT_DATA_LOCK_VERSION
  Performance Schema Row Lock Interface number for the most recent version.
  The most current version is @c PSI_DATA_LOCK_VERSION_1
*/
#define PSI_CURRENT_DATA_LOCK_VERSION 1

#ifndef USE_PSI_DATA_LOCK_2
#ifndef USE_PSI_DATA_LOCK_1
#ifdef __cplusplus
#define USE_PSI_DATA_LOCK_1
#endif /* __cplusplus */
#endif /* USE_PSI_DATA_LOCK_1 */
#endif /* USE_PSI_DATA_LOCK_2 */

#ifdef USE_PSI_DATA_LOCK_1
#define HAVE_PSI_DATA_LOCK_1
#endif /* USE_PSI_DATA_LOCK_1 */

#ifdef USE_PSI_DATA_LOCK_2
#define HAVE_PSI_DATA_LOCK_2
#endif

/** Entry point for the performance schema interface. */
struct PSI_data_lock_bootstrap {
  /**
    ABI interface finder.
    Calling this method with an interface version number returns either
    an instance of the ABI for this version, or NULL.
    @sa PSI_DATA_LOCK_VERSION_1
    @sa PSI_DATA_LOCK_VERSION_2
    @sa PSI_CURRENT_DATA_LOCK_VERSION
  */
  void *(*get_interface)(int version);
};
typedef struct PSI_data_lock_bootstrap PSI_data_lock_bootstrap;

#ifdef HAVE_PSI_DATA_LOCK_1

/**
  Server interface, row lock container.
  This is the interface exposed
  - by the server
  - to the storage engines
  used to collect the data for table DATA_LOCKS.
  The server is to implement this interface.
  The storage engine is to call all_lock_row()
  to advertise row locks that exists within
  the storage engine tables.
*/
class PSI_server_data_lock_container {
 public:
  PSI_server_data_lock_container() {}
  virtual ~PSI_server_data_lock_container() {}

  /**
    Add a string to the container cache.
    Cached strings have the same life cycle as the data container,
    and are freed when the container is destroyed.
    Also, duplicated strings value are cached with the same copy,
    avoiding memory duplication.
    This is useful in particular to cache table schema or table names,
    which are duplicated a lot for different row locks on the same table.
  */
  virtual const char *cache_string(const char *string) = 0;
  /**
    Add binary data to the container cache.
    @sa cache_string
  */
  virtual const char *cache_data(const char *ptr, size_t length) = 0;

  /**
    Check if the container accepts data for a particular engine.
    This methods is used to prune data for queries like
    @code SELECT * from performance_schema.data_locks WHERE ENGINE = ...
    @endcode
  */
  virtual bool accept_engine(const char *engine, size_t engine_length) = 0;

  /**
    Check if the container accepts data for a particular lock.
    This methods is used to prune data for queries like
    @code SELECT * from performance_schema.data_locks WHERE ENGINE_LOCK_ID = ...
    @endcode
  */
  virtual bool accept_lock_id(const char *engine_lock_id,
                              size_t engine_lock_id_length) = 0;

  /**
    Check if the container accepts data for a particular transaction.
    This methods is used to prune data for queries like
    @code SELECT * from performance_schema.data_locks WHERE
    ENGINE_TRANSACTION_ID = ... @endcode
  */
  virtual bool accept_transaction_id(ulonglong transaction_id) = 0;

  /**
    Check if the container accepts data for a particular event.
    This methods is used to prune data for queries like
    @code SELECT * from performance_schema.data_locks
    WHERE THREAD_ID = ... AND EVENT_ID = ... @endcode
  */
  virtual bool accept_thread_id_event_id(ulonglong thread_id,
                                         ulonglong event_id) = 0;

  /**
    Check if the container accepts data for a particular object.
    This methods is used to prune data for queries like
    @code SELECT * from performance_schema.data_locks
    WHERE OBJECT_SCHEMA = ...
    AND OBJECT_NAME = ...
    AND PARTITION_NAME = ...
    AND SUBPARTITION_NAME = ... @endcode
  */
  virtual bool accept_object(const char *table_schema,
                             size_t table_schema_length, const char *table_name,
                             size_t table_name_length,
                             const char *partition_name,
                             size_t partition_name_length,
                             const char *sub_partition_name,
                             size_t sub_partition_name_length) = 0;

  /** Add a row to table performance_schema.data_locks. */
  virtual void add_lock_row(
      const char *engine, size_t engine_length, const char *engine_lock_id,
      size_t engine_lock_id_length, ulonglong transaction_id,
      ulonglong thread_id, ulonglong event_id, const char *table_schema,
      size_t table_schema_length, const char *table_name,
      size_t table_name_length, const char *partition_name,
      size_t partition_name_length, const char *sub_partition_name,
      size_t sub_partition_name_length, const char *index_name,
      size_t index_name_length, const void *identity, const char *lock_mode,
      const char *lock_type, const char *lock_status,
      const char *lock_data) = 0;
};

class PSI_server_data_lock_wait_container {
 public:
  PSI_server_data_lock_wait_container() {}
  virtual ~PSI_server_data_lock_wait_container() {}

  /** @sa PSI_server_data_lock_container::cache_string. */
  virtual const char *cache_string(const char *string) = 0;

  /** @sa PSI_server_data_lock_container::cache_data. */
  virtual const char *cache_data(const char *ptr, size_t length) = 0;

  /**
    Check if the container accepts data for a particular engine.
    This methods is used to prune data for queries like
    @code SELECT * from performance_schema.data_lock_waits WHERE ENGINE = ...
    @endcode
  */
  virtual bool accept_engine(const char *engine, size_t engine_length) = 0;

  /**
    Check if the container accepts data for a particular requesting lock id.
    This methods is used to prune data for queries like
    @code SELECT * from performance_schema.data_lock_waits WHERE
    REQUESTING_ENGINE_LOCK_ID = ... @endcode
  */
  virtual bool accept_requesting_lock_id(const char *engine_lock_id,
                                         size_t engine_lock_id_length) = 0;

  /**
    Check if the container accepts data for a particular blocking lock id.
    This methods is used to prune data for queries like
    @code SELECT * from performance_schema.data_lock_waits WHERE
    BLOCKING_ENGINE_LOCK_ID = ... @endcode
  */
  virtual bool accept_blocking_lock_id(const char *engine_lock_id,
                                       size_t engine_lock_id_length) = 0;

  /**
    Check if the container accepts data for a particular requesting transaction
    id.
    This methods is used to prune data for queries like
    @code SELECT * from performance_schema.data_lock_waits WHERE
    REQUESTING_ENGINE_TRANSACTION_ID = ... @endcode
  */
  virtual bool accept_requesting_transaction_id(ulonglong transaction_id) = 0;

  /**
    Check if the container accepts data for a particular blocking transaction
    id.
    This methods is used to prune data for queries like
    @code SELECT * from performance_schema.data_lock_waits WHERE
    BLOCKING_ENGINE_TRANSACTION_ID = ... @endcode
  */
  virtual bool accept_blocking_transaction_id(ulonglong transaction_id) = 0;

  /**
    Check if the container accepts data for a particular requesting event.
    This methods is used to prune data for queries like
    @code SELECT * from performance_schema.data_lock_waits
    WHERE REQUESTING_THREAD_ID = ... AND REQUESTING_EVENT_ID = ... @endcode
  */
  virtual bool accept_requesting_thread_id_event_id(ulonglong thread_id,
                                                    ulonglong event_id) = 0;

  /**
    Check if the container accepts data for a particular blocking event.
    This methods is used to prune data for queries like
    @code SELECT * from performance_schema.data_lock_waits
    WHERE BLOCKING_THREAD_ID = ... AND BLOCKING_EVENT_ID = ... @endcode
  */
  virtual bool accept_blocking_thread_id_event_id(ulonglong thread_id,
                                                  ulonglong event_id) = 0;

  /** Add a row to table performance_schema.data_lock_waits. */
  virtual void add_lock_wait_row(
      const char *engine, size_t engine_length,
      const char *requesting_engine_lock_id,
      size_t requesting_engine_lock_id_length,
      ulonglong requesting_transaction_id, ulonglong requesting_thread_id,
      ulonglong requesting_event_id, const void *requesting_identity,
      const char *blocking_engine_lock_id,
      size_t blocking_engine_lock_id_length, ulonglong blocking_transaction_id,
      ulonglong blocking_thread_id, ulonglong blocking_event_id,
      const void *blocking_identity) = 0;
};

/**
  Engine interface, row lock iterator.
  This is the interface exposed
  - by a storage engine
  - to the server
  used to iterate over all the row locks
  present within the storage engine tables.
  The storage engine is to implement this interface.
  The server is to call scan() to ask the storage
  engine to add more rows to the container given.
*/
class PSI_engine_data_lock_iterator {
 public:
  PSI_engine_data_lock_iterator() {}
  virtual ~PSI_engine_data_lock_iterator() {}

  /**
    Scan for more data locks.
    @param container The container to fill
    @param with_lock_data True if column LOCK_DATA is required.
    @return true if the iterator is done
  */
  virtual bool scan(PSI_server_data_lock_container *container,
                    bool with_lock_data) = 0;

  /**
    Fetch a given data lock.
    @param container The container to fill
    @param engine_lock_id The lock id to search
    @param engine_lock_id_length Lock id length
    @param with_lock_data True if column LOCK_DATA is required.
    @return true if the iterator is done
  */
  virtual bool fetch(PSI_server_data_lock_container *container,
                     const char *engine_lock_id, size_t engine_lock_id_length,
                     bool with_lock_data) = 0;
};

class PSI_engine_data_lock_wait_iterator {
 public:
  PSI_engine_data_lock_wait_iterator() {}
  virtual ~PSI_engine_data_lock_wait_iterator() {}

  /**
    Scan for more data lock waits.
    @param container The container to fill
    @return true if the iterator is done
  */
  virtual bool scan(PSI_server_data_lock_wait_container *container) = 0;

  /**
    Fetch a given data lock wait.
    @param container The container to fill
    @param requesting_engine_lock_id The requesting lock id to search
    @param requesting_engine_lock_id_length The requesting lock id length
    @param blocking_engine_lock_id The blocking lock id to search
    @param blocking_engine_lock_id_length The blocking lock id length
    @return true if the iterator is done
  */
  virtual bool fetch(PSI_server_data_lock_wait_container *container,
                     const char *requesting_engine_lock_id,
                     size_t requesting_engine_lock_id_length,
                     const char *blocking_engine_lock_id,
                     size_t blocking_engine_lock_id_length) = 0;
};

/**
  Engine interface, row lock inspector.
  This is the interface exposed
  - by a storage engine
  - to the server
  to create an iterator over all row locks.
  The storage engine is to implement this interface.
  The server is to call create_iterator()
  to ask the engine to create an iterator over all row locks.
  A PSI_engine_data_lock_inspector is meant to be stateless,
  and not associated to any opened table handle,
  while the iterator created is meant to be stateful,
  and dedicated to an opened performance_schema.row_locks table handle.
*/
class PSI_engine_data_lock_inspector {
 public:
  PSI_engine_data_lock_inspector() {}
  virtual ~PSI_engine_data_lock_inspector() {}

  /**
    Create a data lock iterator.
    The iterator returned is used to extract data_locks rows from the storage
    engine.
    @sa destroy_data_lock_iterator
  */
  virtual PSI_engine_data_lock_iterator *create_data_lock_iterator() = 0;

  /**
    Create a data lock wait iterator.
    The iterator returned is used to extract data_lock_waits rows from the
    storage engine.
    @sa destroy_data_lock_wait_iterator
  */
  virtual PSI_engine_data_lock_wait_iterator *
  create_data_lock_wait_iterator() = 0;

  /**
    Destroy a data lock iterator.
  */
  virtual void destroy_data_lock_iterator(
      PSI_engine_data_lock_iterator *it) = 0;

  /**
    Destroy a data lock wait iterator.
  */
  virtual void destroy_data_lock_wait_iterator(
      PSI_engine_data_lock_wait_iterator *it) = 0;
};

/**
  Row Lock registration API.
*/
typedef void (*register_data_lock_v1_t)(
    PSI_engine_data_lock_inspector *inspector);

/**
  Row Lock un registration API.
*/
typedef void (*unregister_data_lock_v1_t)(
    PSI_engine_data_lock_inspector *inspector);

/**
  Performance Schema Row Lock Interface, version 1.
  @since PSI_DATA_LOCK_VERSION_1
*/
struct PSI_data_lock_service_v1 {
  register_data_lock_v1_t register_data_lock;
  unregister_data_lock_v1_t unregister_data_lock;
};

#endif /* HAVE_PSI_DATA_LOCK_1 */

/* Export the required version */
#ifdef USE_PSI_DATA_LOCK_1
typedef struct PSI_data_lock_service_v1 PSI_data_lock_service_t;
#else
typedef struct PSI_placeholder PSI_data_lock_service_t;
#endif

extern MYSQL_PLUGIN_IMPORT PSI_data_lock_service_t *psi_data_lock_service;

#endif /* HAVE_PSI_DATA_LOCK_INTERFACE */

/** @} (end of group psi_abi_data_lock) */

#endif /* MYSQL_PSI_DATA_LOCK_H */
