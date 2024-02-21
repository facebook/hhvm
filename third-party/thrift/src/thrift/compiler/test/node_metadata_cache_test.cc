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

#include <thrift/compiler/sema/diagnostic_context.h>

#include <memory>
#include <folly/portability/GTest.h>

namespace apache::thrift::compiler {
namespace {

TEST(NodeMetadataCacheTest, Cache) {
  node_metadata_cache cache;
  EXPECT_EQ(cache.get<int>(t_base_type::t_bool()), 0);
  EXPECT_EQ(
      &cache.get<int>(t_base_type::t_bool()),
      &cache.get<int>(t_base_type::t_bool()));
  EXPECT_EQ(
      cache.get(
          t_base_type::t_i32(), []() { return std::make_unique<int>(1); }),
      1);
  EXPECT_NE(
      &cache.get<int>(t_base_type::t_bool()),
      &cache.get<int>(t_base_type::t_i32()));
  cache.get<int>(t_base_type::t_bool()) = 2;
  EXPECT_EQ(cache.get<int>(t_base_type::t_bool()), 2);
  EXPECT_EQ(cache.get<int>(t_base_type::t_i32()), 1);
  EXPECT_EQ(cache.get<float>(t_base_type::t_i32()), 0.0);
}

} // namespace
} // namespace apache::thrift::compiler
