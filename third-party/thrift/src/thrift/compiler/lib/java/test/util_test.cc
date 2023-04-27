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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/compiler/lib/java/util.h>

using namespace apache::thrift::compiler;

class JavaUtilTest : public testing::Test {};

TEST_F(JavaUtilTest, test_mangle_java_name) {
  EXPECT_EQ(java::mangle_java_name("foo", false), "foo");
  EXPECT_EQ(java::mangle_java_name("Apple", false), "apple");
  EXPECT_EQ(java::mangle_java_name("HBase", false), "HBase");
  EXPECT_EQ(java::mangle_java_name("foo_bar_baz", false), "fooBarBaz");
  EXPECT_EQ(java::mangle_java_name("foo_bar_BAz", false), "fooBarBAz");
  EXPECT_EQ(java::mangle_java_name("foo", true), "Foo");
  EXPECT_EQ(java::mangle_java_name("Apple", true), "Apple");
  EXPECT_EQ(java::mangle_java_name("HBase", true), "HBase");
  EXPECT_EQ(java::mangle_java_name("foo_bar_baz", true), "FooBarBaz");
  EXPECT_EQ(java::mangle_java_name("foo_bar_BAz", true), "FooBarBAz");
  EXPECT_EQ(java::mangle_java_name("foo_", true), "Foo_");
}

TEST_F(JavaUtilTest, test_mangle_java_constant_name) {
  EXPECT_EQ(java::mangle_java_constant_name("foo"), "FOO");
  EXPECT_EQ(java::mangle_java_constant_name("Apple"), "APPLE");
  EXPECT_EQ(java::mangle_java_constant_name("HBase"), "HBASE");
  EXPECT_EQ(java::mangle_java_constant_name("AppleTree"), "APPLE_TREE");
  EXPECT_EQ(java::mangle_java_constant_name("foo_bar_baz"), "FOO_BAR_BAZ");
}

TEST_F(JavaUtilTest, test_escape_java_string) {
  EXPECT_EQ(java::quote_java_string("foo"), "\"foo\"");
  EXPECT_EQ(java::quote_java_string("\""), "\"\\\"\"");
  EXPECT_EQ(java::quote_java_string("\\"), "\"\\\\\"");
}

TEST_F(JavaUtilTest, test_package_to_path) {
  EXPECT_EQ(java::package_to_path("foo"), "foo");
  EXPECT_EQ(java::package_to_path("foo.bar"), "foo/bar");
  EXPECT_EQ(java::package_to_path("foo.bar.baz"), "foo/bar/baz");
}
