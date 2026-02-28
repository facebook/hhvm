/* Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef COMPONENTS_SERVICES_MYSQL_PSI_SYSTEM_SERVICE_H
#define COMPONENTS_SERVICES_MYSQL_PSI_SYSTEM_SERVICE_H

#include <mysql/components/service.h>

typedef void (*mysql_unload_plugin_v1_t)(const char *plugin_name);

BEGIN_SERVICE_DEFINITION(mysql_psi_system_v1)
mysql_unload_plugin_v1_t unload_plugin;
END_SERVICE_DEFINITION(mysql_psi_system_v1)

#define REQUIRES_MYSQL_PSI_SYSTEM_SERVICE REQUIRES_SERVICE(mysql_psi_system_v1)

#endif /* COMPONENTS_SERVICES_MYSQL_PSI_SYSTEM_SERVICE_H */
