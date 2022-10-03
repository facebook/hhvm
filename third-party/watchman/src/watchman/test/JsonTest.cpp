/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include "watchman/thirdparty/jansson/jansson_private.h"

namespace {

std::string json_dtostr(double value) {
  char buf[30];
  int length = jsonp_dtostr(buf, std::size(buf), value);
  if (length >= 0) {
    return std::string{buf, buf + length};
  } else {
    throw std::domain_error(
        fmt::format("error converting value to buffer, length = {}", length));
  }
}

TEST(JsonTest, dtostr) {
  EXPECT_EQ("0.0", json_dtostr(0));
  EXPECT_EQ(
      "0.33333333333333331", json_dtostr(0.3333333333333333333333333333333333));
  EXPECT_EQ(
      "0.66666666666666663", json_dtostr(0.6666666666666666666666666666666666));
  EXPECT_EQ(
      "3.3333333333333333e+33",
      json_dtostr(3333333333333333333333333333333333.0));
  EXPECT_EQ(
      "6.6666666666666667e+33",
      json_dtostr(6666666666666666666666666666666666.0));
  EXPECT_EQ("1.0", json_dtostr(1.0));
  EXPECT_EQ("1e+20", json_dtostr(100000000000000000000.0));
}

TEST(JsonTest, double_round_trip) {
  double values[] = {
      0.3333333333333333333333333333333333,
      3333333333333333333333333333333333.0,
      1.0,
      100000000000000000000.0,
  };
  for (auto d : values) {
    SCOPED_TRACE(fmt::format("value = {}", d));
    auto encoded = json_dumps(json_real(d), JSON_ENCODE_ANY | JSON_COMPACT);
    json_error_t err{};
    auto value = json_loads(encoded.c_str(), JSON_DECODE_ANY, &err);

    EXPECT_EQ(JSON_REAL, value.value().type());
    EXPECT_EQ(d, json_real_value(value.value())) << " encoded = " << encoded;
  }
}

TEST(JsonTest, too_deep_parse_tree) {
  std::string document(10000, '[');

  json_error_t err;
  json_loads(document.c_str(), JSON_DECODE_ANY, &err);
}

} // namespace
