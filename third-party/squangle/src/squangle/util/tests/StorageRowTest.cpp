/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Conv.h>
#include <folly/Random.h>
#include <folly/Range.h>
#include <gtest/gtest.h>
#include <type_traits>

#include "squangle/util/StorageRow.h"

using namespace facebook::common::mysql_client;

using namespace ::testing;

TEST(StorageRowTest, Simple) {
  StorageRow row(6);

  row.appendNull();
  row.appendValue("foo");
  row.appendValue((int64_t)1);
  row.appendValue((uint64_t)1);
  row.appendValue(1.0);
  row.appendValue(true);

  EXPECT_EQ(row.count(), 6);

  // Column 0
  EXPECT_TRUE(row.isNull(0));

  // Column 1
  auto expectStringPiece = [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, folly::StringPiece>) {
      return arg;
    }

    throw std::runtime_error("not a folly::StringPiece");
    return folly::StringPiece();
  };

  EXPECT_FALSE(row.isNull(1));
  EXPECT_EQ(row.as<folly::StringPiece>(1, expectStringPiece), "foo");

  // Column 2
  auto expectInt64 = [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, int64_t>) {
      return arg;
    }

    throw std::runtime_error("not an int64_t");
    return (int64_t)0;
  };

  EXPECT_FALSE(row.isNull(2));
  EXPECT_EQ(row.as<int64_t>(2, expectInt64), 1);

  // Column 3
  auto expectUInt64 = [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, uint64_t>) {
      return arg;
    }

    throw std::runtime_error("not a uint64_t");
    return (uint64_t)0;
  };

  EXPECT_FALSE(row.isNull(3));
  EXPECT_EQ(row.as<uint64_t>(3, expectUInt64), 1);

  // Column 4
  auto expectDouble = [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, double>) {
      return arg;
    }

    throw std::runtime_error("not a double");
    return 0.0;
  };

  EXPECT_FALSE(row.isNull(4));
  EXPECT_EQ(row.as<double>(4, expectDouble), 1.0);

  // Column 5
  auto expectBool = [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, bool>) {
      return arg;
    }

    throw std::runtime_error("not a bool");
    return false;
  };

  EXPECT_FALSE(row.isNull(5));
  EXPECT_EQ(row.as<bool>(5, expectBool), true);
}

TEST(StorageRowTest, SimpleBool) {
  StorageRow row(3);

  row.appendValue(true);
  row.appendValue(false);
  row.appendNull();

  auto expectBool = [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, bool>) {
      return arg;
    }

    throw std::runtime_error("not a bool");
    return false;
  };

  ASSERT_FALSE(row.isNull(0));
  EXPECT_EQ(row.as<bool>(0, expectBool), true);

  ASSERT_FALSE(row.isNull(1));
  EXPECT_EQ(row.as<bool>(1, expectBool), false);

  EXPECT_TRUE(row.isNull(2));
}

TEST(StorageRowTest, SimpleInt64) {
  std::vector<int64_t> values = {
      0,
      1,
      std::numeric_limits<int8_t>::max(),
      ((int64_t)std::numeric_limits<int8_t>::max()) + 1,
      std::numeric_limits<int16_t>::max(),
      ((int64_t)std::numeric_limits<int16_t>::max()) + 1,
      std::numeric_limits<int32_t>::max(),
      ((int64_t)std::numeric_limits<int32_t>::max()) + 1,
      std::numeric_limits<int64_t>::max(),
      -1,
      std::numeric_limits<int8_t>::min(),
      ((int64_t)std::numeric_limits<int8_t>::min()) - 1,
      std::numeric_limits<int16_t>::min(),
      ((int64_t)std::numeric_limits<int16_t>::min()) - 1,
      std::numeric_limits<int32_t>::min(),
      ((int64_t)std::numeric_limits<int32_t>::min()) - 1,
      std::numeric_limits<int64_t>::min(),
  };

  StorageRow row(values.size());

  for (auto value : values) {
    row.appendValue(value);
  }

  auto expectInt64_t = [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, int64_t>) {
      return arg;
    }

    throw std::runtime_error("not an int64_t");
    return (int64_t)0;
  };

  size_t column = 0;
  for (auto value : values) {
    ASSERT_FALSE(row.isNull(column));
    EXPECT_EQ(row.as<int64_t>(column, expectInt64_t), value);
    column++;
  }
}

TEST(StorageRowTest, SimpleUInt64) {
  std::vector<uint64_t> values = {
      0,
      1,
      std::numeric_limits<uint8_t>::max(),
      ((uint64_t)std::numeric_limits<uint8_t>::max()) + 1,
      std::numeric_limits<uint16_t>::max(),
      ((uint64_t)std::numeric_limits<uint16_t>::max()) + 1,
      std::numeric_limits<uint32_t>::max(),
      ((uint64_t)std::numeric_limits<uint32_t>::max()) + 1,
      std::numeric_limits<uint64_t>::max(),
  };

  StorageRow row(values.size());

  for (auto value : values) {
    row.appendValue(value);
  }

  auto expectUInt64_t = [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, uint64_t>) {
      return arg;
    }

    throw std::runtime_error("not a uint64_t");
    return (uint64_t)0;
  };

  size_t column = 0;
  for (auto value : values) {
    ASSERT_FALSE(row.isNull(column));
    EXPECT_EQ(row.as<int64_t>(column, expectUInt64_t), value);
    column++;
  }
}

TEST(StorageRowTest, SimpleDouble) {
  std::vector<double> values = {
      0.0,
      0.1,
      std::numeric_limits<float>::max(),
      ((double)std::numeric_limits<float>::max()) + 0.000001,
      std::numeric_limits<double>::max(),
  };

  StorageRow row(values.size());

  for (auto value : values) {
    row.appendValue(value);
  }

  auto expectDouble = [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, double>) {
      return arg;
    }

    throw std::runtime_error("not a double");
    return 0.0;
  };

  size_t column = 0;
  for (auto value : values) {
    ASSERT_FALSE(row.isNull(column));
    EXPECT_EQ(row.as<double>(column, expectDouble), value);
    column++;
  }
}

TEST(StorageRowTest, SimpleStrings) {
  std::vector<std::string> values = {
      "short",
      "",
      "this is a medium length string holding a somewhere north of one hundred characters used to test medium length strings",
      "Thanks 😊", // Unicode
      "语言处理", // more Unicode
      std::string(4096, 'a'), // a string with 4096 'a's.
      std::string(4097, 'b'), // a string longer than 4096
      std::string(1000000, 'c'), // a huge string
  };

  StorageRow row(values.size());

  for (auto value : values) {
    row.appendValue(value);
  }

  auto expectString = [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, folly::StringPiece>) {
      return arg;
    }

    throw std::runtime_error("not a string");
    return folly::StringPiece();
  };

  size_t column = 0;
  for (auto value : values) {
    ASSERT_FALSE(row.isNull(column));
    EXPECT_EQ(row.as<folly::StringPiece>(column, expectString), value);
    column++;
  }
}

void randomBytes(void* val, size_t length) {
  for (size_t ii = 0; ii < length;) {
    auto rnd = folly::Random::rand64();
    auto bytes_to_copy = std::min(sizeof(rnd), length - ii);
    memcpy(((uint8_t*)val) + ii, &rnd, bytes_to_copy);
    ii += bytes_to_copy;
  }
}

TEST(StorageRowTest, RandomValues) {
  static constexpr size_t kNumColumns = 1000000;

  using TypeUnion = std::
      variant<std::monostate, bool, int64_t, uint64_t, double, std::string>;
  std::vector<TypeUnion> values;
  values.reserve(kNumColumns);

  StorageRow row(kNumColumns);

  for (size_t i = 0; i < kNumColumns; i++) {
    switch (folly::Random::rand32(6)) {
      case 0: {
        // Null values
        row.appendNull();
        values.push_back({});
        break;
      }

      case 1: {
        // Boolean values
        auto val = folly::Random::randBool(0.5);
        row.appendValue(val);
        values.push_back(val);
        break;
      }

      case 2: {
        // One byte signed integer values
        int8_t val;
        randomBytes(&val, sizeof(val));
        row.appendValue((int64_t)val);
        values.push_back((int64_t)val);
        break;
      }

      case 3: {
        // Two byte signed integer values
        int16_t val;
        randomBytes(&val, sizeof(val));
        row.appendValue((int64_t)val);
        values.push_back((int64_t)val);
        break;
      }

      case 4: {
        // Four byte signed integer values
        int32_t val;
        randomBytes(&val, sizeof(val));
        row.appendValue((int64_t)val);
        values.push_back((int64_t)val);
        break;
      }

      case 5: {
        // Eight byte signed integer values
        int64_t val;
        randomBytes(&val, sizeof(val));
        row.appendValue(val);
        values.push_back(val);
        break;
      }

      case 6: {
        // One byte unsigned integer values
        uint8_t val;
        randomBytes(&val, sizeof(val));
        row.appendValue((uint64_t)val);
        values.push_back((uint64_t)val);
        break;
      }

      case 7: {
        // Two byte unsigned integer values
        uint16_t val;
        randomBytes(&val, sizeof(val));
        row.appendValue((uint64_t)val);
        values.push_back((uint64_t)val);
        break;
      }

      case 8: {
        // Four byte unsigned integer values
        uint32_t val;
        randomBytes(&val, sizeof(val));
        row.appendValue((uint64_t)val);
        values.push_back((uint64_t)val);
        break;
      }

      case 9: {
        // Eight byte unsigned integer values
        uint64_t val;
        randomBytes(&val, sizeof(val));
        row.appendValue(val);
        values.push_back(val);
        break;
      }

      case 10: {
        // Double values
        double val;
        randomBytes(&val, sizeof(val));
        row.appendValue(val);
        values.push_back(val);
        break;
      }

      case 11: {
        // Strings less <= 4096 bytes
        auto size = folly::Random::rand32(4096);
        std::string val(size, ' ');
        randomBytes(val.data(), val.size());
        row.appendValue(val);
        values.push_back(val);
        break;
      }

      case 12: {
        // Strings less <= 4096 bytes
        auto size = folly::Random::rand32(1000000) + 4096;
        std::string val(size, ' ');
        randomBytes(val.data(), val.size());
        row.appendValue(val);
        values.push_back(val);
        break;
      }
    }
  }

  // Now check each entry and compare it to what we expect
  size_t column = 0;
  for (const auto& value : values) {
    std::visit(
        [&](auto&& arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, std::monostate>) {
            EXPECT_TRUE(row.isNull(column));
          } else if constexpr (std::is_same_v<T, bool>) {
            EXPECT_FALSE(row.isNull(column));
            EXPECT_EQ(arg, row.as<bool>(column, [](const auto& arg) {
              using U = std::decay_t<decltype(arg)>;
              if constexpr (std::is_same_v<U, bool>) {
                return arg;
              }

              throw std::runtime_error("not a bool");
              return false;
            }));
          } else if constexpr (std::is_same_v<T, int64_t>) {
            EXPECT_FALSE(row.isNull(column));
            EXPECT_EQ(arg, row.as<int64_t>(column, [](const auto& arg) {
              using U = std::decay_t<decltype(arg)>;
              if constexpr (std::is_same_v<U, int64_t>) {
                return arg;
              }

              throw std::runtime_error("not an int64_t");
              return (int64_t)0;
            }));
          } else if constexpr (std::is_same_v<T, uint64_t>) {
            EXPECT_FALSE(row.isNull(column));
            EXPECT_EQ(arg, row.as<uint64_t>(column, [](const auto& arg) {
              using U = std::decay_t<decltype(arg)>;
              if constexpr (std::is_same_v<U, uint64_t>) {
                return arg;
              }

              throw std::runtime_error("not a uint64_t");
              return (uint64_t)0;
            }));
          } else if constexpr (std::is_same_v<T, double>) {
            EXPECT_FALSE(row.isNull(column));
            EXPECT_EQ(arg, row.as<double>(column, [](const auto& arg) {
              using U = std::decay_t<decltype(arg)>;
              if constexpr (std::is_same_v<U, double>) {
                return arg;
              }

              throw std::runtime_error("not a double");
              return 0.0;
            }));
          } else if constexpr (std::is_same_v<T, folly::StringPiece>) {
            EXPECT_FALSE(row.isNull(column));
            EXPECT_EQ(
                arg, row.as<folly::StringPiece>(column, [](const auto& arg) {
                  using U = std::decay_t<decltype(arg)>;
                  if constexpr (std::is_same_v<U, folly::StringPiece>) {
                    return arg;
                  }

                  throw std::runtime_error("not a string");
                  return folly::StringPiece();
                }));
          }
        },
        value);
    column++;
  }
}
