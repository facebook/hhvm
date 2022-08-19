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
#include <string>

#include "my_io.h"
#include "my_sys.h"
#include "unit_test_common.h"

/* This is the test which uses the minimal chassis library and checks the
   dynamic loader load and unload functions using mysql_server component.

   This testdriver does the following:
   * minimal_chassis_init() Which makes the registry and dynamic loader
     services ready.
   * acquire "dynamic_loader" service from the registry.
   * loads mysql_server component
   * unloads mysql_server component
   * unregisters all the services at the exit time.
*/

using registry_type_t = SERVICE_TYPE_NO_CONST(registry);
using loader_type_t = SERVICE_TYPE_NO_CONST(dynamic_loader);

class minimal_chassis : public ::testing::Test {
 protected:
  virtual void SetUp() {
    reg = NULL;
    loader = NULL;
    ASSERT_FALSE(minimal_chassis_init((&reg), NULL));
    ASSERT_FALSE(reg->acquire("dynamic_loader",
                              reinterpret_cast<my_h_service *>(
                                  const_cast<loader_type_t **>(&loader))));
  }

  virtual void TearDown() {
    if (reg) {
      ASSERT_FALSE(reg->release(
          reinterpret_cast<my_h_service>(const_cast<registry_type_t *>(reg))));
    }
    if (loader) {
      ASSERT_FALSE(reg->release(
          reinterpret_cast<my_h_service>(const_cast<loader_type_t *>(loader))));
    }
    ASSERT_FALSE(minimal_chassis_deinit(reg, NULL));
  }
  SERVICE_TYPE_NO_CONST(registry) * reg;
  SERVICE_TYPE(dynamic_loader) * loader;
};

TEST_F(minimal_chassis, try_load_component) {
  static const char *urns[] = {"file://component_example_component1"};
  std::string path;
  const char *urn;
  make_absolute_urn(*urns, &path);
  urn = path.c_str();
  ASSERT_FALSE(loader->load(&urn, 1));
  ASSERT_FALSE(loader->unload(&urn, 1));
}

/* mandatory main function */
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  MY_INIT(argv[0]);

  char realpath_buf[FN_REFLEN];
  char basedir_buf[FN_REFLEN];
  my_realpath(realpath_buf, my_progname, 0);
  size_t res_length;
  dirname_part(basedir_buf, realpath_buf, &res_length);
  if (res_length > 0) basedir_buf[res_length - 1] = '\0';
  my_setwd(basedir_buf, 0);

  int retval = RUN_ALL_TESTS();
  return retval;
}
