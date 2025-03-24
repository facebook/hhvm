/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <map>
#include <gtest/gtest.h>

#include <thrift/compiler/codemod/package_generator.h>

namespace apache::thrift::compiler {

TEST(PackageGeneratorTest, file_path) {
  std::string path = "/foo/Bar123FOO/Baz.thrift";
  EXPECT_EQ(
      codemod::package_name_generator::from_file_path(path),
      "meta.com/foo/bar123_foo/baz");
  path = "baz.thrift";
  EXPECT_EQ(
      codemod::package_name_generator::from_file_path(path), "meta.com/baz");
  path = "python/foo/bar.thrift";
  EXPECT_EQ(
      codemod::package_name_generator::from_file_path(path),
      "meta.com/foo/bar");
}

TEST(PackageGeneratorTest, namespace) {
  codemod::package_name_generator gen("cpp2", "test.BarFOO.BarBazB");
  EXPECT_EQ(gen.generate("apache.org"), "apache.org/test/bar_foo/bar_baz_b");

  codemod::package_name_generator gen_with_domain("cpp2", "facebook.foo.bar");
  EXPECT_EQ(gen_with_domain.generate("apache.org"), "facebook.com/foo/bar");

  codemod::package_name_generator gen_with_default_domain(
      "cpp2", "test.foo.bar");
  EXPECT_EQ(gen_with_default_domain.generate(), "meta.com/test/foo/bar");
}

TEST(PackageGeneratorTest, common_package) {
  std::map<std::string, std::string> namespaces = {
      {"cpp2", "foo.bar.baz"}, {"hack", "baz"}};

  auto get_common_pkg = [&]() {
    return codemod::package_name_generator_util::from_namespaces(namespaces)
        .find_common_package();
  };

  // No common package found
  EXPECT_EQ(get_common_pkg(), "");

  // Common Package with no domain in common namespaces
  namespaces["python"] = "foo.bar.BAZ";
  EXPECT_EQ(get_common_pkg(), "meta.com/foo/bar/baz");

  /*
   * Since domain is not present in any of the common namespaces,
   * Any available domain should be used.
   */
  namespaces["java"] = "org.apache.foobar";
  EXPECT_EQ(get_common_pkg(), "apache.org/foo/bar/baz");

  // Common namespace with domain
  namespaces["cpp2"] = "facebook.foo.bar.baz";
  EXPECT_EQ(get_common_pkg(), "facebook.com/foo/bar/baz");

  /*
   * Packages from cpp2 and python namespaces have different domains but
   * same path.
   * Use the common path of cpp2 and python namespaces
   * and domain from either one of them
   */
  namespaces["python"] = "meta.foo.bar.baz";
  EXPECT_EQ(get_common_pkg(), "meta.com/foo/bar/baz");

  // cpp and python ns have same path => foo.bar.baz
  // java and hack ns have same path => foobar
  // since "foo.bar.baz" is longer than "foobar", choose the longer one
  namespaces["hack"] = "apache.foobar";
  EXPECT_EQ(get_common_pkg(), "meta.com/foo/bar/baz");
}

TEST(PackageGeneratorTest, ns_with_language_identifiers) {
  std::map<std::string, std::string> namespaces = {
      {"cpp2", "foo.test_cpp2.baz_cpp2"}, {"hack", "baz"}};

  auto get_common_pkg = [&]() {
    return codemod::package_name_generator_util::from_namespaces(namespaces)
        .find_common_package();
  };

  // No common package found
  EXPECT_EQ(get_common_pkg(), "");

  namespaces["python"] = "foo.python_test.baz";
  /*
   * After removing language identifiers,
   * cpp2 and python namespaces respectively become,
   * foo.test_cpp2.baz_cpp2 => foo.test.baz
   * foo.python_test.baz => foo.test.baz
   */
  EXPECT_EQ(get_common_pkg(), "meta.com/foo/test/baz");

  // Only language specific identifiers should be removed
  // So for python namespace, "cpp2" should be retained.
  namespaces["python"] = "foo.test_cpp2.baz_python_cpp2.python";
  EXPECT_EQ(get_common_pkg(), "meta.com/foo/test_cpp2/baz_cpp2");
}

TEST(PackageGeneratorTest, common_identifiers) {
  std::map<std::string, std::string> namespaces = {
      {"cpp2", "apache.xyz.foo.abc"},
      {"hack", "abc.apache.xyz.foo"},
      {"py3", "apache.foo"}};
  auto get_pkg_from_common_identifiers = [&]() {
    return codemod::package_name_generator_util::from_namespaces(namespaces)
        .get_package_from_common_identifiers();
  };

  // For above namespaces, common identifiers = {"foo"}
  // which doesn't meet minimum length requirement
  EXPECT_EQ(get_pkg_from_common_identifiers(), "");

  namespaces["py3"] = "meta.foo.abc.xyz";
  // common identifiers = {"foo", "xyz"}
  // Since the order of common identifiers is different in all three namespaces,
  // choosing the order from first namespace.

  // Since there are 2 different domains in cpp2 and py3 ns, choosing the first
  // domain
  EXPECT_EQ(get_pkg_from_common_identifiers(), "apache.org/xyz/foo");
}

TEST(PackageGeneratorTest, longest_package) {
  std::map<std::string, std::string> namespaces = {
      {"cpp2", "apache.abcdef.ghij"}, {"hack", "xyz.abc.foo"}, {"py3", "foo"}};

  auto get_longest_pkg = [&]() {
    return codemod::package_name_generator_util::from_namespaces(namespaces)
        .get_longest_package();
  };

  // Since domain is present in one of the namespaces,
  // the domain from that namespace is used for generating the package.
  // Package from cpp2 namespace is the longest one.
  EXPECT_EQ(get_longest_pkg(), "apache.org/abcdef/ghij");

  // Since py3 namespaces has a domain, that domain will be used for generating
  // the package from py3 namespace.
  // "facebook.com/abcdef.ghi" (len = 23) > "apache.org/abcdef/ghij" (len = 22)
  namespaces["py3"] = "facebook.abcdef.ghi";
  EXPECT_EQ(get_longest_pkg(), "facebook.com/abcdef/ghi");
}

TEST(PackageGeneratorTest, to_snake_case) {
  std::map<std::string, std::string> input_output = {
      {"FOOBAR", "foobar"},
      {"FOOBar", "foo_bar"},
      {"FOO_Bar", "foo_bar"},
      {"FooBar", "foo_bar"},
      {"FooV2Bar", "foo_v2_bar"},
      {"FOO1232Bar", "foo1232_bar"},
      {"BazFOO2Bar", "baz_foo2_bar"}};
  for (auto const& [input, output] : input_output) {
    codemod::package_name_generator gen("cpp2", fmt::format("test.{}", input));
    EXPECT_EQ(gen.generate(), fmt::format("meta.com/test/{}", output));
  }
}
} // namespace apache::thrift::compiler
