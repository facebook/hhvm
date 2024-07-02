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

#include <thrift/compiler/lib/reserved_identifier_name.h>

#include <folly/portability/GTest.h>

namespace apache::thrift::compiler {

TEST(ReservedIdentifierTest, IsReservedIdentifierName) {
  EXPECT_TRUE(is_reserved_identifier_name("fbthriftIsReserved"));
  EXPECT_TRUE(is_reserved_identifier_name("__fbThriftIsAlsoReserved"));
  EXPECT_TRUE(is_reserved_identifier_name("FBThriftEvenThisIsReserved"));
  EXPECT_TRUE(is_reserved_identifier_name("FBThrift"));

  EXPECT_FALSE(is_reserved_identifier_name("ThisIsNotReserved"));
  EXPECT_FALSE(is_reserved_identifier_name("NetherIsFbThriftReserved"));
  EXPECT_FALSE(is_reserved_identifier_name("FB_THRIFT_IS_NOT_RESERVED"));
  EXPECT_FALSE(is_reserved_identifier_name("___________________"));
  EXPECT_FALSE(is_reserved_identifier_name("___________________fbthrif"));
  EXPECT_FALSE(is_reserved_identifier_name("NotIt"));
}

} // namespace apache::thrift::compiler
