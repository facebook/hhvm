/* Copyright (c) 2008, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef COMPONENTS_SERVICES_PSI_THREAD_BITS_H
#define COMPONENTS_SERVICES_PSI_THREAD_BITS_H

#ifndef MYSQL_ABI_CHECK
#include <stddef.h> /* size_t */
#endif

#include <mysql/components/services/my_io_bits.h>     /* sockaddr_storage */
#include <mysql/components/services/my_thread_bits.h> /* my_thread_handle */

/**
  @file
  Performance schema instrumentation interface.

  @defgroup psi_abi_thread Thread Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

/**
  Instrumented thread key.
  To instrument a thread, a thread key must be obtained
  using @c register_thread.
  Using a zero key always disable the instrumentation.
*/
typedef unsigned int PSI_thread_key;

#ifdef __cplusplus
class THD;
#else
/*
  Phony declaration when compiling C code.
  This is ok, because the C code will never have a THD anyway.
*/
struct opaque_THD {
  int dummy;
};
typedef struct opaque_THD THD;
#endif

/** @sa enum_vio_type. */
typedef int opaque_vio_type;

/**
  Interface for an instrumented thread.
  This is an opaque structure.
*/
struct PSI_thread;
typedef struct PSI_thread PSI_thread;

/**
  Thread instrument information.
  @since PSI_THREAD_VERSION_1
  This structure is used to register an instrumented thread.
*/
struct PSI_thread_info_v1 {
  /**
    Pointer to the key assigned to the registered thread.
  */
  PSI_thread_key *m_key;
  /**
    The name of the thread instrument to register.
  */
  const char *m_name;
  /**
    The flags of the thread to register.
    @sa PSI_FLAG_SINGLETON
    @sa PSI_FLAG_USER
  */
  unsigned int m_flags;
  /** Volatility index. */
  int m_volatility;
  /** Documentation. */
  const char *m_documentation;
};
typedef struct PSI_thread_info_v1 PSI_thread_info_v1;

/**
  Thread registration API.
  @param category a category name (typically a plugin name)
  @param info an array of thread info to register
  @param count the size of the info array
*/
typedef void (*register_thread_v1_t)(const char *category,
                                     struct PSI_thread_info_v1 *info,
                                     int count);

/**
  Spawn a thread.
  This method creates a new thread, with instrumentation.
  @param key the instrumentation key for this thread
  @param thread the resulting thread
  @param attr the thread attributes
  @param start_routine the thread start routine
  @param arg the thread start routine argument
*/
typedef int (*spawn_thread_v1_t)(PSI_thread_key key, my_thread_handle *thread,
                                 const my_thread_attr_t *attr,
                                 void *(*start_routine)(void *), void *arg);

/**
  Create instrumentation for a thread.
  @param key the registered key
  @param identity an address typical of the thread
  @param thread_id PROCESSLIST_ID of the thread
  @return an instrumented thread
*/
typedef struct PSI_thread *(*new_thread_v1_t)(PSI_thread_key key,
                                              const void *identity,
                                              unsigned long long thread_id);

/**
  Assign a THD to an instrumented thread.
  @param thread the instrumented thread
  @param thd the sql layer THD to assign
*/
typedef void (*set_thread_THD_v1_t)(struct PSI_thread *thread, THD *thd);

/**
  Assign an id to an instrumented thread.
  @param thread the instrumented thread
  @param id the PROCESSLIST_ID to assign
*/
typedef void (*set_thread_id_v1_t)(struct PSI_thread *thread,
                                   unsigned long long id);
/**
  Read the THREAD_ID of the current thread.
  @return the id of the instrumented thread
*/
typedef unsigned long long (*get_current_thread_internal_id_v2_t)();

/**
  Read the THREAD_ID of an instrumented thread.
  @param thread the instrumented thread
  @return the id of the instrumented thread
*/
typedef unsigned long long (*get_thread_internal_id_v2_t)(
    struct PSI_thread *thread);

/**
  Get the instrumentation for the thread of given PROCESSLIST_ID.
  @param processlist_id the thread id
  @return the instrumented thread
*/
typedef struct PSI_thread *(*get_thread_by_id_v2_t)(
    unsigned long long processlist_id);

/**
  Assign the current operating system thread id to an instrumented thread.
  The operating system task id is obtained from @c gettid()
  @param thread the instrumented thread
*/
typedef void (*set_thread_os_id_v1_t)(struct PSI_thread *thread);

/**
  Get the current operating system thread id of an instrumented thread.
  The operating system task id is obtained from @c gettid()
  @param thread the instrumented thread
*/
typedef unsigned long long (*get_thread_os_id_v1_t)(struct PSI_thread *thread);

/**
  Assign priority to an instrumented thread.
  @param thread the instrumented thread
  @param priority the nice value of the OS thread
*/
typedef void (*set_thread_priority_v1_t)(struct PSI_thread *thread, int pri);

/**
  Get the current operating system thread priority of an instrumented thread.
  @param thread the instrumented thread
*/
typedef int (*get_thread_priority_v1_t)(struct PSI_thread *thread);

/**
  Get the instrumentation for the running thread.
  For this function to return a result,
  the thread instrumentation must have been attached to the
  running thread using @c set_thread()
  @return the instrumentation for the running thread
*/
typedef struct PSI_thread *(*get_thread_v1_t)(void);

/**
  Assign a user name to the instrumented thread.
  @param user the user name
  @param user_len the user name length
*/
typedef void (*set_thread_user_v1_t)(const char *user, int user_len);

/**
  Assign a user name and host name to the instrumented thread.
  @param user the user name
  @param user_len the user name length
  @param host the host name
  @param host_len the host name length
*/
typedef void (*set_thread_account_v1_t)(const char *user, int user_len,
                                        const char *host, int host_len);

/**
  Assign a current database to the instrumented thread.
  @param db the database name
  @param db_len the database name length
*/
typedef void (*set_thread_db_v1_t)(const char *db, int db_len);

/**
  Assign a current command to the instrumented thread.
  @param command the current command
*/
typedef void (*set_thread_command_v1_t)(int command);

/**
  Assign a connection type to the instrumented thread.
  @param conn_type the connection type
*/
typedef void (*set_connection_type_v1_t)(opaque_vio_type conn_type);

/**
  Assign a start time to the instrumented thread.
  @param start_time the thread start time
*/
typedef void (*set_thread_start_time_v1_t)(time_t start_time);

/**
  Assign a state to the instrumented thread.
  @param state the thread state
*/
typedef void (*set_thread_state_v1_t)(const char *state);

/**
  Assign a process info to the instrumented thread.
  @param info the process into string
  @param info_len the process into string length
*/
typedef void (*set_thread_info_v1_t)(const char *info, unsigned int info_len);

/**
  Assign a resource group name to the current thread.

  @param group_name resource group name string
  @param group_name_len resource group name string length
  @param user_data user-defined data
  return 0 if successful, 1 otherwise
*/
typedef int (*set_thread_resource_group_v1_t)(const char *group_name,
                                              int group_name_len,
                                              void *user_data);

/**
  Assign a resource group name to an instrumented thread, identified either by
  the thread instrumentation or Performance Schema thread id.

  @param thread pointer to the thread instrumentation. Ignored if NULL.
  @param thread_id thread id of the target thread. Only used if thread is NULL.
  @param group_name resource group name string
  @param group_name_len resource group name string length
  @param user_data user-defined data
  return 0 if successful, 1 otherwise
*/
typedef int (*set_thread_resource_group_by_id_v1_t)(
    PSI_thread *thread, unsigned long long thread_id, const char *group_name,
    int group_name_len, void *user_data);

/**
  Attach a thread instrumentation to the running thread.
  In case of thread pools, this method should be called when
  a worker thread picks a work item and runs it.
  Also, this method should be called if the instrumented code does not
  keep the pointer returned by @c new_thread() and relies on @c get_thread()
  instead.
  @param thread the thread instrumentation
*/
typedef void (*set_thread_v1_t)(struct PSI_thread *thread);

/** Aggregate the thread status variables. */
typedef void (*aggregate_thread_status_v2_t)(struct PSI_thread *thread);

/** Delete the current thread instrumentation. */
typedef void (*delete_current_thread_v1_t)(void);

/** Delete a thread instrumentation. */
typedef void (*delete_thread_v1_t)(struct PSI_thread *thread);

/**
  Stores an array of connection attributes
  @param buffer         char array of length encoded connection attributes
                        in network format
  @param length         length of the data in buffer
  @param from_cs        charset in which @c buffer is encoded
  @return state
    @retval  non_0    attributes truncated
    @retval  0        stored the attribute
*/
typedef int (*set_thread_connect_attrs_v1_t)(const char *buffer,
                                             unsigned int length,
                                             const void *from_cs);
/**
  Stores client id and attributes
  @param client_id                 MD5 hash of client attributes
  @param client_attributes         set of client attributes serialized as JSON
  @param client_attributes_length  length of client_attributes
  @return state
    @retval  non_0    attributes truncated
    @retval  0        stored the attribute
*/
typedef int (*set_thread_client_attrs_v1_t)(const unsigned char *client_id,
                                            const char *client_attributes,
                                            uint client_attributes_length);

/**
  Get the current thread current event.
  @param [out] thread_internal_id The thread internal id
  @param [out] event_id The per thread event id.
*/
typedef void (*get_current_thread_event_id_v2_t)(
    unsigned long long *thread_internal_id, unsigned long long *event_id);

/**
  Get the thread current event.
  @deprecated
  @param [out] thread_internal_id The thread internal id
  @param [out] event_id The per thread event id.
*/
typedef void (*get_thread_event_id_v1_t)(unsigned long long *thread_internal_id,
                                         unsigned long long *event_id);

/**
  Get the thread current event.
  @param psi the instrumented thread
  @param [out] thread_internal_id The thread internal id
  @param [out] event_id The per thread event id.
*/
typedef void (*get_thread_event_id_v2_t)(struct PSI_thread *psi,
                                         unsigned long long *thread_internal_id,
                                         unsigned long long *event_id);

/* Duplicate definitions to avoid dependency on mysql_com.h */
#define PSI_USERNAME_LENGTH (80 * 3)
#define PSI_NAME_LEN (64 * 3)
#define PSI_HOSTNAME_LENGTH (255)

/**
  Performance Schema thread type: user/foreground or system/background.
  @sa get_thread_system_attrs
*/
struct PSI_thread_attrs_v3 {
  /* PFS internal thread id, unique. */
  unsigned long long m_thread_internal_id;

  /* SHOW PROCESSLIST thread id, not unique. */
  unsigned long m_processlist_id;

  /* Operating system thread id, if any. */
  unsigned long long m_thread_os_id;

  /* User-defined data. */
  void *m_user_data;

  /* User name. */
  char m_username[PSI_USERNAME_LENGTH];

  /* User name length. */
  size_t m_username_length;

  /* Host name. */
  char m_hostname[PSI_HOSTNAME_LENGTH];

  /* Host name length. */
  size_t m_hostname_length;

  /* Resource group name. */
  char m_groupname[PSI_NAME_LEN];

  /* Resource group name length. */
  size_t m_groupname_length;

  /** Raw socket address */
  struct sockaddr_storage m_sock_addr;

  /** Length of address */
  socklen_t m_sock_addr_length;

  /* True if system/background thread, false if user/foreground thread. */
  bool m_system_thread;
};

typedef struct PSI_thread_attrs_v3 PSI_thread_attrs;

/**
  Callback for the pfs_notification service.
  @param thread_attrs system attributes of the current thread.
*/
typedef void (*PSI_notification_cb_v3)(const PSI_thread_attrs_v3 *thread_attrs);

/**
  Registration structure for the pfs_notification service.
  @sa register_notification_v3_t
*/
struct PSI_notification_v3 {
  PSI_notification_cb_v3 thread_create;
  PSI_notification_cb_v3 thread_destroy;
  PSI_notification_cb_v3 session_connect;
  PSI_notification_cb_v3 session_disconnect;
  PSI_notification_cb_v3 session_change_user;
};

typedef struct PSI_notification_v3 PSI_notification;

/**
  Get system attributes for the current thread.

  @param thread_attrs pointer to pfs_thread_attr struct
  @return 0 if successful, 1 otherwise
*/
typedef int (*get_thread_system_attrs_v3_t)(PSI_thread_attrs_v3 *thread_attrs);

/**
  Get system attributes for an instrumented thread, identified either by the
  thread instrumentation or Performance Schema thread id.

  @param thread pointer to the thread instrumentation. Ignored if NULL.
  @param thread_id thread id of the target thread. Only used if thread is NULL.
  @param thread_attrs pointer to pfs_thread_attr struct
  @return 0 if successful, 1 otherwise
*/
typedef int (*get_thread_system_attrs_by_id_v3_t)(
    PSI_thread *thread, unsigned long long thread_id,
    PSI_thread_attrs_v3 *thread_attrs);

/**
  Register callback functions for the Notification service.
  For best performance, set with_ref_count = false.

  @param callbacks structure of user-defined callback functions
  @param with_ref_count true if callbacks can be unregistered
  @sa unregister_notification
  @return registration handle on success, 0 if failure
*/
typedef int (*register_notification_v3_t)(const PSI_notification_v3 *callbacks,
                                          bool with_ref_count);

/**
  Unregister callback functions for the Notification service.

  @param handle  registration handle returned by register_notification()
  @sa register_notification
  @return 0 if successful, non-zero otherwise
*/
typedef int (*unregister_notification_v1_t)(int handle);

/**
  Invoke the callback function registered for a session connect event.

  @param thread the thread instrumentation
*/
typedef void (*notify_session_connect_v1_t)(PSI_thread *thread);

/**
  Invoke the callback function registered for a session disconnect event.

  @param thread the thread instrumentation
*/
typedef void (*notify_session_disconnect_v1_t)(PSI_thread *thread);

/**
  Invoke the callback function registered for a changer user event.

  @param thread the thread instrumentation
*/
typedef void (*notify_session_change_user_v1_t)(PSI_thread *thread);

typedef struct PSI_thread_info_v1 PSI_thread_info;

/** @} (end of group psi_abi_thread) */

#endif /* COMPONENTS_SERVICES_PSI_THREAD_BITS_H */
