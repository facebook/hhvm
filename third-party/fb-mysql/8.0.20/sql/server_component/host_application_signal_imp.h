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

#ifndef HOST_APPLICATION_SIGNAL_IMP_H
#define HOST_APPLICATION_SIGNAL_IMP_H

#include <mysql/components/service_implementation.h>
#include <mysql/components/services/host_application_signal.h>

/**
  An implementation of host application signal service for the
  mysql server as a host application.
*/
class mysql_component_host_application_signal_imp {
 public:
  static DEFINE_BOOL_METHOD(signal, (int signal_no, void *arg));
};

#endif /* HOST_APPLICATION_SIGNAL_IMP_H */
