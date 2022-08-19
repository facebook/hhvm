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

#ifndef HOST_APPLICATION_SIGNAL_H
#define HOST_APPLICATION_SIGNAL_H

#include <mysql/components/service.h>

enum host_application_signal_signals {
  HOST_APPLICATION_SIGNAL_SHUTDOWN =
      0,                            ///< shut the application down. no argument
  HOST_APPLICATION_SIGNAL_LAST = 1  ///< Internal. Not a signal Keep that last
};
/**
  @ingroup group_components_services_inventory

  A service to deliver a signal to host application.

  Typically there'll be just one implementation of this by the main
  application.

  Other parties interested in listening to shutdown may override the
  default implementation with a broadcast one and have multiple implementations
  receiving the shutdown signal. Or do message queueing to a set of background
  threads etc.

  @sa mysql_component_host_application_signal_imp
*/
BEGIN_SERVICE_DEFINITION(host_application_signal)
/**
  Send a signal.

  Can send one of @ref host_application_signal_signals

  @param signal_no The signal to trigger
  @param arg Signal dependent argument
  @return Status of performed operation
  @retval false success (valid password)
  @retval true failure (invalid password)

  @sa my_host_application_signal
*/
DECLARE_BOOL_METHOD(signal, (int signal_no, void *arg));

END_SERVICE_DEFINITION(host_application_signal)

#endif /* HOST_APPLICATION_SIGNAL_H */
