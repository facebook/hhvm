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

#include <gtest/gtest.h>

#include <thrift/compiler/codemod/package_generator.h>

namespace apache::thrift::compiler {

TEST(PackageGeneratorTest, file_path) {
  std::string path = "/foo/bar/baz.thrift";
  EXPECT_EQ(
      codemod::package_name_generator::from_file_path(path),
      "meta.com/foo/bar/baz");
  path = "baz.thrift";
  EXPECT_EQ(
      codemod::package_name_generator::from_file_path(path), "meta.com/baz");
  path = "python/foo/bar.thrift";
  EXPECT_EQ(
      codemod::package_name_generator::from_file_path(path),
      "meta.com/foo/bar");
}

TEST(PackageGeneratorTest, namespace) {
  codemod::package_name_generator gen("test.foo.bar");
  EXPECT_EQ(gen.generate("apache.org"), "apache.org/test/foo/bar");

  codemod::package_name_generator gen_with_domain("facebook.foo.bar");
  EXPECT_EQ(gen_with_domain.generate("apache.org"), "facebook.com/foo/bar");

  codemod::package_name_generator gen_with_default_domain("test.foo.bar");
  EXPECT_EQ(gen_with_default_domain.generate(), "meta.com/test/foo/bar");
}
} // namespace apache::thrift::compiler
