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

#include <gtest/gtest.h>
#include <mysql/components/minimal_chassis.h>
#include <mysql/components/my_service.h>
#include <mysql/components/services/mysql_server_runnable_service.h>
#include <stdlib.h>
#include <string>
#include "my_io.h"
#include "my_sys.h"
#include "unit_test_common.h"

/* This is the test driver which uses the minimal chassis library

   This testdriver does the following:
   * minimal_chassis_init() Which makes the registry and dynamic loader
     services ready.
   * load mysql_server component, this will register the server component
     services to the registry.
   * acquire "mysql_libminchassis_runnable" service from the registry.
   * call run() service api.
   * unregisters all the services at the exit time.
*/

using registry_type_t = SERVICE_TYPE_NO_CONST(registry);
using runnable_type_t = SERVICE_TYPE_NO_CONST(mysql_server_runnable);
using loader_type_t = SERVICE_TYPE_NO_CONST(dynamic_loader);
using registrator_type_t = SERVICE_TYPE_NO_CONST(registry_registration);
SERVICE_TYPE_NO_CONST(registry) * registry_ptr;
SERVICE_TYPE_NO_CONST(dynamic_loader) * loader;
SERVICE_TYPE_NO_CONST(registry_registration) * registrator;
const char *progname;
const char *urn;
std::string path;

extern mysql_component_t mysql_component_mysql_server;

SERVICE_TYPE(mysql_server_runnable) * runnable;

static void atexit_main(void) {
  /* set back the dynamic_loader_scheme_file service to minimal_chassis
     service */
  registrator->set_default("dynamic_loader_scheme_file.mysql_minimal_chassis");
  registry_ptr->release(reinterpret_cast<my_h_service>(
      const_cast<registrator_type_t *>(registrator)));
  registry_ptr->release(
      reinterpret_cast<my_h_service>(const_cast<runnable_type_t *>(runnable)));
  registry_ptr->release(
      reinterpret_cast<my_h_service>(const_cast<loader_type_t *>(loader)));
  /* Before unloading the test component we need to set the global
     service handles to minimal chassis services */
  minimal_chassis_services_refresh(1);
  loader->unload(&urn, 1);
  minimal_chassis_deinit(registry_ptr, &COMPONENT_REF(mysql_server));
}

int main(int argc, char **argv) {
  INIT(argv[0]);

  char realpath_buf[FN_REFLEN];
  char basedir_buf[FN_REFLEN];
  size_t res_length;
  static const char *urns[] = {"file://component_example_component1"};

  make_realpath(realpath_buf, progname);
  make_dirname_part(basedir_buf, realpath_buf, &res_length);
  if (res_length > 0) basedir_buf[res_length - 1] = '\0';
  make_setwd(basedir_buf);
  minimal_chassis_init((&registry_ptr), &COMPONENT_REF(mysql_server));

  registry_ptr->acquire("dynamic_loader", (my_h_service *)&loader);

  make_absolute_urn(*urns, &path);
  urn = path.c_str();
  loader->load(&urn, 1);
  /* After loading the test component, update the global service
     handles to mysql_server services */
  minimal_chassis_services_refresh(0);

  registry_ptr->acquire("mysql_server_runnable",
                        reinterpret_cast<my_h_service *>(
                            const_cast<runnable_type_t **>(&runnable)));
  registry_ptr->acquire("registry_registration",
                        reinterpret_cast<my_h_service *>(
                            const_cast<registrator_type_t **>(&registrator)));
  /* runnable->run() method may call the exit, so to handle that we need
     atexit() registration to clean the resources */
  atexit(atexit_main);
  /* This method calls the test executables with passed arguments */
  runnable->run(argc - 1, (argv + 1));

  return 0;
}
