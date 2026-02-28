/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef PFS_NOTIFY_SERVICE_H
#define PFS_NOTIFY_SERVICE_H

#include <mysql/components/service.h>
#include <mysql/psi/mysql_thread.h>

/**
  @page PAGE_PFS_NOTIFICATION_SERVICE Notification service

  @section PFS_NOTIFICATION_INTRO Introduction
  The Performance Schema Notification service provides a means to register
  user-defined callback functions for the following thread and session events:

  - thread create
  - thread destroy
  - session connect
  - session disconnect
  - session change user

  This is a synchronous, low-level API designed to impose minimal performance
  overhead.

  @section PFS_NOTIFICATION_CALLBACKS Callback Function Definition

  Callback functions are of the type #PSI_notification_cb:

  @code
    typedef void (*PSI_notification_cb)(const PSI_thread_attrs *thread_attrs);
  @endcode

  For example:
  @code
    void callback_thread_create(const PSI_thread_attrs *thread_attrs)
    {
      set_thread_resource_group(...)
    }
  @endcode

  When the callback is invoked, the #PSI_thread_attrs structure will contain the
  system attributes of the thread.

  @section PFS_NOTIFICATION_REGISTER Registering Events

  To register for one or more events, set the corresponding callback function
  pointers in the #PSI_notification structure, leaving the function pointer
  NULL for unused callbacks.

  Use the service function @c register_notification() to register the callbacks
  with the server

  @code
    int register_notification(PSI_notification *callbacks,
                              bool with_ref_count);
  @endcode

  where
  - @c callbacks is the callback function set
  - @c with_ref_count determines whether a reference count is used for the
                      callbacks. Set TRUE for callback functions in
                      dynamically loaded modules. Set FALSE for callback
                      functions in static or unloadable modules.

  For example:
  @code
    PSI_notification_cb my_callbacks;

    my_callbacks.thread_create   = &thread_create_callback;
    my_callbacks.thread_destroy  = &thread_destroy_callback;
    my_callbacks.session_connect = &session_connect_callback;
    my_callbacks.session_disconnect  = &session_disconnect_callback;
    my_callbacks.session_change_user = nullptr;

    int my_handle =
     mysql_service_pfs_notification->register_notification(&my_callbacks, true);
  @endcode

  A non-zero handle is returned if the registration is successful. This handle
  is used to unregister the callback set.

  A callback set can be registered more than once. No error is returned
  for calling @c register_notification() more than once for a given callback
  set. Callbacks are invoked once for each time they are registered.

  For callback functions that reside in dynamically loaded modules, set
  @verbatim with_ref_count = TRUE @endverbatim so that the module can be safely
  unloaded after the callbacks are unregistered.

  For callback functions that reside in static, built-in or otherwise unloadable
  modules, set @verbatim with_ref_count = FALSE @endverbatim to optimize
  callback performance in high-concurrency environments.

  Callbacks that reside in a dynamically loaded module such as a server plugin,
  must be successfully unregistered before the module is unloaded.

  For callbacks in static or unloadable modules, @c unregister_notification()
  will disable the callback functions, but the function pointers will remain.

  @section PFS_NOTIFICATION_UNREGISTER Unregistering Events

  To unregister callback functions from the Notification service, use the handle
  returned from @c register_notification(). For example:

  @code
    int ret =
  mysql_service_pfs_notification->unregister_notification(my_handle);

    if (ret == 0)
    {
      // unload component
    }
    else
    {
      // error
    }
  @endcode

  Callbacks that reside in a dynamically loaded module such as a server plugin
  or component must be successfully unregistered before the module is unloaded.

  If @c unregister_notification() returns an error, then the module should not
  be unloaded.

  If the callbacks were registered with @verbatim with_ref_count = TRUE
  @endverbatim then
  @c unregister_notification() will return an error if any of the functions are
  in use and fail to return after 2 seconds.

  If the callbacks were registered with @verbatim with_ref_count = FALSE
  @endverbatim then
  @c unregister_notification() will disable the callback functions, but the
  callback function pointers will be assumed to be valid until the server is
  shutdown.

  @c unregister_callback() can be called multiple times for the same handle.

  Failing to unregister all callback functions from a dynamically loaded plugin
  or component may result in a undocumented behavior during server shutdown.

*/

/*
  SERVICE_DEFINITION(pfs_notification)
  Introduced in MySQL 8.0.2
  Removed in MySQL 8.0.17
  Status: Removed, use version 3 instead.
*/

/*
  Version 3.
  Introduced in MySQL 8.0.17
  Status: active
*/

BEGIN_SERVICE_DEFINITION(pfs_notification_v3)
register_notification_v3_t register_notification;
unregister_notification_v1_t unregister_notification;
END_SERVICE_DEFINITION(pfs_notification_v3)

#define REQUIRES_PFS_NOTIFICATION_SERVICE REQUIRES_SERVICE(pfs_notification_v3)

#endif
