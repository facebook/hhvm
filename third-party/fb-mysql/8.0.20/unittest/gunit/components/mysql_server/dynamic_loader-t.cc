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

#include <example_services.h>
#include <gtest/gtest.h>
#include <mysql/components/minimal_chassis.h>
#include <mysql/components/my_service.h>
#include <string>

#include "m_ctype.h"
#include "my_io.h"
#include "my_sys.h"
#include "scope_guard.h"
#include "unit_test_common.h"

using registry_type_t = SERVICE_TYPE_NO_CONST(registry);
using loader_type_t = SERVICE_TYPE_NO_CONST(dynamic_loader);

class dynamic_loader : public ::testing::Test {
 protected:
  virtual void SetUp() {
    reg = NULL;
    loader = NULL;
    ASSERT_FALSE(minimal_chassis_init(&reg, NULL));
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

TEST_F(dynamic_loader, bootstrap) { ASSERT_TRUE(loader != nullptr); }

TEST_F(dynamic_loader, try_load_component) {
  static const char *urns[] = {"file://component_example_component1"};
  std::string path;
  const char *absolute_urns;
  make_absolute_urn(*urns, &path);
  absolute_urns = path.c_str();
  ASSERT_FALSE(loader->load(&absolute_urns, 1));
  ASSERT_FALSE(loader->unload(&absolute_urns, 1));
}

TEST_F(dynamic_loader, try_unload_the_same_component_in_group) {
  static const char *urns[] = {"file://component_example_component1"};
  std::string path;
  const char *absolute_urns[2];
  for (int i = 0; i < 2; i++)
    absolute_urns[i] = (char *)malloc(2046 * sizeof(char));
  make_absolute_urn(*urns, &path);
  strcpy(const_cast<char *>(absolute_urns[0]), path.c_str());
  ASSERT_FALSE(loader->load(absolute_urns, 1));
  static const char *urns_bad[] = {"file://component_example_component1",
                                   "file://component_example_component1"};
  make_absolute_urn(urns_bad[1], &path);
  strcpy(const_cast<char *>(absolute_urns[1]), path.c_str());
  ASSERT_TRUE(loader->unload((const char **)absolute_urns, 2));
  ASSERT_FALSE(loader->unload((const char **)&absolute_urns[0], 1));
  for (int i = 0; i < 2; i++) free(const_cast<char *>(absolute_urns[i]));
}

TEST_F(dynamic_loader, try_load_twice) {
  static const char *urns[] = {"file://component_example_component1"};
  std::string path;
  const char *absolute_urns;
  make_absolute_urn(*urns, &path);
  absolute_urns = path.c_str();
  ASSERT_FALSE(loader->load(&absolute_urns, 1));
  ASSERT_TRUE(loader->load(&absolute_urns, 1));
  {
    my_service<SERVICE_TYPE(example_math)> service("example_math", reg);
    ASSERT_FALSE((bool)service);
  }

  ASSERT_FALSE(loader->unload(&absolute_urns, 1));
}

TEST_F(dynamic_loader, try_load_not_existing) {
  static const char *urns[] = {"file://component_example_component0"};
  std::string path;
  const char *absolute_urns;
  make_absolute_urn(*urns, &path);
  absolute_urns = path.c_str();
  ASSERT_TRUE(loader->load(&absolute_urns, 1));
}

TEST_F(dynamic_loader, try_load_with_unsatisfied_dependencies) {
  static const char *urns[] = {"file://component_example_component3"};
  std::string path;
  const char *absolute_urns;
  make_absolute_urn(*urns, &path);
  absolute_urns = path.c_str();
  ASSERT_TRUE(loader->load(&absolute_urns, 1));
}

TEST_F(dynamic_loader, try_load_and_forget) {
  static const char *urns[] = {"file://component_example_component1"};
  std::string path;
  const char *absolute_urns;
  make_absolute_urn(*urns, &path);
  absolute_urns = path.c_str();
  ASSERT_FALSE(loader->load(&absolute_urns, 1));
}

TEST_F(dynamic_loader, try_unload_not_existing) {
  static const char *urns[] = {"file://component_example_component0"};
  std::string path;
  const char *absolute_urns;
  make_absolute_urn(*urns, &path);
  absolute_urns = path.c_str();
  ASSERT_TRUE(loader->unload(&absolute_urns, 1));
}

TEST_F(dynamic_loader, load_different_components) {
  static const char *urns1[] = {"file://component_example_component1"};
  static const char *urns2[] = {"file://component_example_component2",
                                "file://component_example_component3"};
  std::string path1, path2, path3;
  const char *absolute_urn;
  const char *absolute_urns[2];
  for (int i = 0; i < 2; i++)
    absolute_urns[i] = (char *)malloc(2046 * sizeof(char));
  make_absolute_urn(*urns1, &path1);
  absolute_urn = path1.c_str();
  {
    my_service<SERVICE_TYPE(example_math)> service("example_math", reg);
    ASSERT_TRUE((bool)service);
  }
  ASSERT_FALSE(loader->load(&absolute_urn, 1));
  {
    my_service<SERVICE_TYPE(example_math)> service("example_math", reg);
    ASSERT_FALSE((bool)service);
  }
  ASSERT_FALSE(loader->unload(&absolute_urn, 1));
  make_absolute_urn(urns2[0], &path2);
  strcpy(const_cast<char *>(absolute_urns[0]), path2.c_str());
  make_absolute_urn(urns2[1], &path3);
  strcpy(const_cast<char *>(absolute_urns[1]), path3.c_str());
  ASSERT_FALSE(loader->load(absolute_urns, 2));
  {
    my_service<SERVICE_TYPE(example_math)> service("example_math", reg);
    ASSERT_FALSE((bool)service);
  }
  ASSERT_FALSE(loader->unload(absolute_urns, 2));
  {
    my_service<SERVICE_TYPE(example_math)> service("example_math", reg);
    ASSERT_TRUE((bool)service);
  }
  for (int i = 0; i < 2; i++) free(const_cast<char *>(absolute_urns[i]));
}

TEST_F(dynamic_loader, dependencies) {
  static const char *urns1[] = {"file://component_example_component3"};
  static const char *urns2[] = {"file://component_example_component1",
                                "file://component_example_component3"};
  std::string path1, path2;
  const char *absolute_urn;
  const char *absolute_urns[2];
  for (int i = 0; i < 2; i++)
    absolute_urns[i] = (char *)malloc(2046 * sizeof(char));
  make_absolute_urn(*urns1, &path1);
  absolute_urn = path1.c_str();
  {
    my_service<SERVICE_TYPE(example_math)> service("example_math", reg);
    ASSERT_TRUE((bool)service);
  }
  ASSERT_TRUE(loader->load(&absolute_urn, 1));
  {
    my_service<SERVICE_TYPE(example_math)> service("example_math", reg);
    ASSERT_TRUE((bool)service);
  }
  make_absolute_urn(urns2[0], &path2);
  strcpy(const_cast<char *>(absolute_urns[0]), path2.c_str());
  make_absolute_urn(urns2[1], &path1);
  strcpy(const_cast<char *>(absolute_urns[1]), path1.c_str());
  ASSERT_FALSE(loader->load(absolute_urns, 2));
  ASSERT_FALSE(loader->unload(absolute_urns, 2));
  {
    my_service<SERVICE_TYPE(example_math)> service("example_math", reg);
    ASSERT_TRUE((bool)service);
  }
  for (int i = 0; i < 2; i++) free(const_cast<char *>(absolute_urns[i]));
}

TEST_F(dynamic_loader, cyclic_dependencies) {
  static const char *urns_self_depends[] = {
      "file://component_self_required_test_component"};
  static const char *urns_cyclic_depends_broken1[] = {
      "file://component_cyclic_dependency_test_component_1"};
  static const char *urns_cyclic_depends_broken2[] = {
      "file://component_cyclic_dependency_test_component_2"};
  static const char *urns_cyclic_depends[] = {
      "file://component_cyclic_dependency_test_component_1",
      "file://component_cyclic_dependency_test_component_2"};

  std::string path1, path2, path3;
  const char *absolute_self_depends;
  const char *absolute_cyclic_depends_broken1;
  const char *absolute_cyclic_depends_broken2;
  const char *absolute_cyclic_depends[2];
  make_absolute_urn(*urns_self_depends, &path1);
  absolute_self_depends = path1.c_str();

  /* Self-provided requirements should pass. */
  ASSERT_FALSE(loader->load(&absolute_self_depends, 1));
  ASSERT_FALSE(loader->unload(&absolute_self_depends, 1));

  /* Broken cyclic dependency. */
  make_absolute_urn(*urns_cyclic_depends_broken1, &path2);
  absolute_cyclic_depends_broken1 = path2.c_str();
  ASSERT_TRUE(loader->load(&absolute_cyclic_depends_broken1, 1));
  make_absolute_urn(*urns_cyclic_depends_broken2, &path3);
  absolute_cyclic_depends_broken2 = path3.c_str();
  ASSERT_TRUE(loader->load(&absolute_cyclic_depends_broken2, 1));

  /* Correct cyclic dependency.*/
  for (int i = 0; i < 2; i++)
    absolute_cyclic_depends[i] = (char *)malloc(2046 * sizeof(char));
  make_absolute_urn(urns_cyclic_depends[0], &path2);
  strcpy(const_cast<char *>(absolute_cyclic_depends[0]), path2.c_str());
  make_absolute_urn(urns_cyclic_depends[1], &path3);
  strcpy(const_cast<char *>(absolute_cyclic_depends[1]), path3.c_str());
  ASSERT_FALSE(loader->load(absolute_cyclic_depends, 2));
  ASSERT_FALSE(loader->unload(absolute_cyclic_depends, 2));
  for (int i = 0; i < 2; i++)
    free(const_cast<char *>(absolute_cyclic_depends[i]));
}

TEST_F(dynamic_loader, first_dependency) {
  static const char *urn1[] = {"file://component_example_component1"};
  static const char *urn2[] = {"file://component_example_component2"};
  static const char *urn3[] = {"file://component_example_component3"};
  std::string path1, path2, path3;
  const char *absolute_urn1;
  const char *absolute_urn2;
  const char *absolute_urn3;
  make_absolute_urn(*urn1, &path1);
  absolute_urn1 = path1.c_str();
  make_absolute_urn(*urn2, &path2);
  absolute_urn2 = path2.c_str();
  make_absolute_urn(*urn3, &path3);
  absolute_urn3 = path3.c_str();
  ASSERT_TRUE(loader->load(&absolute_urn3, 1));
  ASSERT_FALSE(loader->load(&absolute_urn1, 1));
  ASSERT_FALSE(loader->load(&absolute_urn3, 1));
  ASSERT_FALSE(loader->load(&absolute_urn2, 1));
  /*
    lib2 would be sufficient for lib3 to satisfy its dependencies, but lib3 is
    already using actively dependency on lib1, so we can't unload it here.
  */
  ASSERT_TRUE(loader->unload(&absolute_urn1, 1));
}

TEST_F(dynamic_loader, iteration) {
  my_service<SERVICE_TYPE(dynamic_loader_query)> service("dynamic_loader_query",
                                                         reg);
  ASSERT_FALSE(service);

  my_h_component_iterator iterator;
  const char *name;
  const char *urn;
  int count = 0;
  bool test_library_found = false;

  /* No components to iterate over. */
  ASSERT_TRUE(service->create(&iterator));

  static const char *urns[] = {"file://component_example_component1",
                               "file://component_example_component2",
                               "file://component_example_component3"};
  std::string path;
  const char *absolute_urns[3];
  for (int i = 0; i < 3; i++)
    absolute_urns[i] = (char *)malloc(2046 * sizeof(char));
  make_absolute_urn(urns[0], &path);
  strcpy(const_cast<char *>(absolute_urns[0]), path.c_str());
  make_absolute_urn(urns[1], &path);
  strcpy(const_cast<char *>(absolute_urns[1]), path.c_str());
  make_absolute_urn(urns[2], &path);
  strcpy(const_cast<char *>(absolute_urns[2]), path.c_str());

  ASSERT_FALSE(loader->load(absolute_urns, 3));

  ASSERT_FALSE(service->create(&iterator));

  auto guard = create_scope_guard(
      [&service, &iterator]() { service->release(iterator); });

  service->release(my_h_component_iterator{});
  ASSERT_TRUE(service->get(my_h_component_iterator{}, &name, &urn));
  ASSERT_TRUE(service->next(my_h_component_iterator{}));
  ASSERT_TRUE(service->is_valid(my_h_component_iterator{}));
  for (; !service->is_valid(iterator); service->next(iterator)) {
    ASSERT_FALSE(service->get(iterator, &name, &urn));

    count++;
    test_library_found |= !strcmp(name, "mysql:example_component1");
  }
  ASSERT_TRUE(service->get(iterator, &name, &urn));
  ASSERT_TRUE(service->next(iterator));
  ASSERT_TRUE(service->is_valid(iterator));

  /* there should be at least 3 test components loaded. */
  ASSERT_GE(count, 3);
  ASSERT_TRUE(test_library_found);
  for (int i = 0; i < 3; i++) free(const_cast<char *>(absolute_urns[i]));
}

TEST_F(dynamic_loader, metadata) {
  my_service<SERVICE_TYPE(dynamic_loader_query)> query_service(
      "dynamic_loader_query", reg);
  ASSERT_FALSE(query_service);

  my_service<SERVICE_TYPE(dynamic_loader_metadata_enumerate)> metadata_service(
      "dynamic_loader_metadata_enumerate", reg);
  ASSERT_FALSE(metadata_service);

  my_service<SERVICE_TYPE(dynamic_loader_metadata_query)>
      metadata_query_service("dynamic_loader_metadata_query", reg);
  ASSERT_FALSE(metadata_query_service);

  static const char *urns[] = {"file://component_example_component1",
                               "file://component_example_component2",
                               "file://component_example_component3"};

  std::string path;
  const char *absolute_urns[3];
  for (int i = 0; i < 3; i++)
    absolute_urns[i] = (char *)malloc(2046 * sizeof(char));
  make_absolute_urn(urns[0], &path);
  strcpy(const_cast<char *>(absolute_urns[0]), path.c_str());
  make_absolute_urn(urns[1], &path);
  strcpy(const_cast<char *>(absolute_urns[1]), path.c_str());
  make_absolute_urn(urns[2], &path);
  strcpy(const_cast<char *>(absolute_urns[2]), path.c_str());

  ASSERT_FALSE(loader->load(absolute_urns, 3));

  my_h_component_iterator iterator;
  const char *name;
  const char *urn;
  const char *value;
  int count = 0;
  bool property_found = false;

  ASSERT_FALSE(query_service->create(&iterator));

  auto query_service_guard = create_scope_guard(
      [&query_service, &iterator]() { query_service->release(iterator); });

  for (; !query_service->is_valid(iterator); query_service->next(iterator)) {
    ASSERT_FALSE(query_service->get(iterator, &name, &urn));

    if (!strcmp(urn, "file://component_example_component1")) {
      ASSERT_FALSE(
          metadata_query_service->get_value(iterator, "mysql.author", &value));
      ASSERT_STREQ(value, "Oracle Corporation");
      ASSERT_FALSE(
          metadata_query_service->get_value(iterator, "mysql.license", &value));
      ASSERT_STREQ(value, "GPL");
      ASSERT_FALSE(
          metadata_query_service->get_value(iterator, "test_property", &value));
      ASSERT_TRUE(metadata_query_service->get_value(
          iterator, "non_existing_test_property", &value));

      my_h_component_metadata_iterator metadata_iterator;

      ASSERT_FALSE(metadata_service->create(iterator, &metadata_iterator));

      auto guard =
          create_scope_guard([&metadata_service, &metadata_iterator]() {
            metadata_service->release(metadata_iterator);
          });

      metadata_service->release(my_h_component_metadata_iterator{});
      ASSERT_TRUE(metadata_service->get(my_h_component_metadata_iterator{},
                                        &name, &value));
      ASSERT_TRUE(metadata_service->next(my_h_component_metadata_iterator{}));
      ASSERT_TRUE(
          metadata_service->is_valid(my_h_component_metadata_iterator{}));
      for (; !metadata_service->is_valid(metadata_iterator);
           metadata_service->next(metadata_iterator)) {
        ASSERT_FALSE(metadata_service->get(metadata_iterator, &name, &value));

        count++;
        property_found |= strcmp(name, "test_property");
      }
      ASSERT_TRUE(metadata_service->get(metadata_iterator, &name, &value));
      ASSERT_TRUE(metadata_service->next(metadata_iterator));
      ASSERT_TRUE(metadata_service->is_valid(metadata_iterator));

      /* there should be at least 3 properties. */
      ASSERT_GE(count, 3);
      ASSERT_TRUE(property_found);
    }
  }
  for (int i = 0; i < 3; i++) free(const_cast<char *>(absolute_urns[i]));
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
  my_end(0);
  return retval;
}
