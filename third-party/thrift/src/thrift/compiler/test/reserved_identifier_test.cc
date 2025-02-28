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

#include <thrift/compiler/sema/reserved_identifier.h>

#include <gtest/gtest.h>

using apache::thrift::compiler::is_reserved_identifier;

TEST(ReservedIdentifierTest, is_reserved_identifier) {
  EXPECT_TRUE(is_reserved_identifier("fbthriftIsReserved"));
  EXPECT_TRUE(is_reserved_identifier("__fbThriftIsAlsoReserved"));
  EXPECT_TRUE(is_reserved_identifier("FBThriftEvenThisIsReserved"));
  EXPECT_TRUE(is_reserved_identifier("FBThrift"));

  EXPECT_FALSE(is_reserved_identifier("ThisIsNotReserved"));
  EXPECT_FALSE(is_reserved_identifier("NetherIsFbThriftReserved"));
  EXPECT_FALSE(is_reserved_identifier("FB_THRIFT_IS_NOT_RESERVED"));
  EXPECT_FALSE(is_reserved_identifier("___________________"));
  EXPECT_FALSE(is_reserved_identifier("___________________fbthrif"));
  EXPECT_FALSE(is_reserved_identifier("NotIt"));
}
