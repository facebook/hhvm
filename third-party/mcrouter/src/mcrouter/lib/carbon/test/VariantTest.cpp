/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>

#include <gtest/gtest.h>

#include "mcrouter/lib/carbon/Variant.h"

TEST(Variant, basic) {
  carbon::Variant<int, std::string> var;

  var.emplace<std::string>("abc");
  EXPECT_EQ("abc", var.get<std::string>());
  EXPECT_EQ(std::type_index(typeid(std::string)), var.which());
  EXPECT_TRUE(var.is<std::string>());
  EXPECT_FALSE(var.is<int>());

  var.emplace<int>(123);
  EXPECT_EQ(123, var.get<int>());
  EXPECT_EQ(std::type_index(typeid(int)), var.which());
  EXPECT_TRUE(var.is<int>());
  EXPECT_FALSE(var.is<std::string>());
}
