/* Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MY_HOST_APPLICATION_SIGNAL_H
#define MY_HOST_APPLICATION_SIGNAL_H

/**
  @file
  Convenience wrappers for @ref mysql_service_host_application_signal_t
 */

#include <mysql/components/my_service.h>
#include <mysql/components/services/host_application_signal.h>

/**
  Template to simplify sending application signals via the
  @ref mysql_service_host_application_signal_t service.
*/
template <int signal_number, typename arg_type>
bool my_host_application_signal(SERVICE_TYPE(registry) * registry,
                                arg_type argument) {
  my_service<SERVICE_TYPE(host_application_signal)> host_app(
      "host_application_signal", registry);

  return host_app->signal(signal_number, argument);
}

/**
  Ease of use utility function to emit the @ref HOST_APPLICATION_SIGNAL_SHUTDOWN
  signal via @ref mysql_service_host_application_signal_t

  @sa host_application_signal, mysql_service_host_application_signal_t

  @param registry the registry handle to use
  @retval true failure
  @retval false success
*/
inline bool my_host_application_signal_shutdown(SERVICE_TYPE(registry) *
                                                registry) {
  return my_host_application_signal<HOST_APPLICATION_SIGNAL_SHUTDOWN, void *>(
      registry, NULL);
}

#endif /* MY_HOST_APPLICATION_SIGNAL_H */
