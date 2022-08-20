/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <fatal/type/array.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/reflection/reflection.h>
#include <thrift/test/reflection/gen-cpp2/troublesome_fatal_all.h>

namespace apache::thrift::test::reflection {
namespace {

TEST(TroublesomeTest, FatalReflection) {
  using module = reflect_module<troublesome_tags::module>;
  EXPECT_STREQ(fatal::z_data<module::name>(), "troublesome");
  EXPECT_STREQ(fatal::z_data<troublesome_tags::strings::strings>(), "strings");
  EXPECT_STREQ(fatal::z_data<troublesome_tags::structs::structs>(), "structs");
  EXPECT_STREQ(fatal::z_data<troublesome_tags::enums::enums>(), "enums");
  EXPECT_STREQ(
      fatal::z_data<troublesome_tags::constants::constants>(), "constants");
  EXPECT_STREQ(
      fatal::z_data<troublesome_tags::services::services>(), "services");
}

} // namespace
} // namespace apache::thrift::test::reflection
