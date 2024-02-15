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
#include <thrift/compiler/lib/go/util.h>

using namespace apache::thrift::compiler;

class GoUtilTest : public testing::Test {};

TEST_F(GoUtilTest, test_munge_ident) {
  // All upper case
  EXPECT_EQ(go::munge_ident("FOO", true), "FOO");
  EXPECT_EQ(go::munge_ident("FOO_BAR", true), "FOO_BAR");

  // Snake case
  EXPECT_EQ(go::munge_ident("foo", true), "Foo");
  EXPECT_EQ(go::munge_ident("foo_", true), "Foo_");
  EXPECT_EQ(go::munge_ident("foo_bar", true), "FooBar");

  // Camel case
  EXPECT_EQ(go::munge_ident("Foo", true), "Foo");
  EXPECT_EQ(go::munge_ident("FooBar", true), "FooBar");
  EXPECT_EQ(go::munge_ident("fooBar", true), "FooBar");
  EXPECT_EQ(go::munge_ident("FOOBar", true), "FOOBar");
  EXPECT_EQ(go::munge_ident("FooBar_", true), "FooBar_");

  // Initialisms
  EXPECT_EQ(go::munge_ident("url", true, false), "URL");
  EXPECT_EQ(go::munge_ident("Url", true, false), "URL");
  EXPECT_EQ(go::munge_ident("URL", true, false), "URL");
  EXPECT_EQ(go::munge_ident("url", false, false), "url");
  EXPECT_EQ(go::munge_ident("Url", false, false), "url");
  EXPECT_EQ(go::munge_ident("URL", false, false), "url");
  EXPECT_EQ(go::munge_ident("FooUrl", true, false), "FooURL");
  EXPECT_EQ(go::munge_ident("FooUrlBar", true, false), "FooURLBar");
  EXPECT_EQ(go::munge_ident("Foo_Url", true, false), "Foo_URL");
  EXPECT_EQ(go::munge_ident("Foo_Url_Bar", true, false), "Foo_URL_Bar");
  EXPECT_EQ(go::munge_ident("foo_url", true, false), "FooURL");
  EXPECT_EQ(go::munge_ident("foo_url_bar", true, false), "FooURLBar");
  EXPECT_EQ(go::munge_ident("UrlBar", true, false), "URLBar");
  EXPECT_EQ(go::munge_ident("Url_Bar", true, false), "URL_Bar");
  EXPECT_EQ(go::munge_ident("url_bar", true, false), "URLBar");
  // Legacy substr bug
  EXPECT_EQ(
      go::munge_ident("cluster_id_to_name", true, true), "ClusterIdToName");
  EXPECT_EQ(go::munge_ident("service_id", true, true), "ServiceID");

  // Reserved idents
  EXPECT_EQ(go::munge_ident("type", false, true), "type_");
  EXPECT_EQ(go::munge_ident("go", false, true), "go_");

  // Compat cases
  EXPECT_EQ(go::munge_ident("foo_args", true, true), "FooArgs_");
  EXPECT_EQ(go::munge_ident("FooArgs", true, true), "FooArgs_");
  EXPECT_EQ(go::munge_ident("foo_result", true, true), "FooResult_");
  EXPECT_EQ(go::munge_ident("FooResult", true, true), "FooResult_");
  EXPECT_EQ(go::munge_ident("new_bar", true, true), "NewBar_");
  EXPECT_EQ(go::munge_ident("NewBar", true, true), "NewBar_");
}
