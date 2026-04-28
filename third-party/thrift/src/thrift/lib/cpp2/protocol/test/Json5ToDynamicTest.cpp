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

#include <cmath>
#include <filesystem>
#include <gtest/gtest.h>
#include <folly/FileUtil.h>
#include <folly/json.h>
#include <folly/json/json.h>
#include <folly/testing/TestUtil.h>
#include <thrift/lib/cpp2/protocol/test/Json5ToDynamic.h>

namespace apache::thrift::json5::detail {
namespace {

// Recursively replaces NaN double values with a sentinel string so that
// folly::dynamic equality comparisons work (since NaN != NaN).
void maskNan(folly::dynamic& a) {
  if (a.isDouble() && std::isnan(a.asDouble())) {
    a = "__fbthrift_masked_nan";
  }
  if (a.isArray()) {
    for (folly::dynamic& i : a) {
      maskNan(i);
    }
  }
  if (a.isObject()) {
    for (auto& [key, val] : a.items()) {
      maskNan(val);
    }
  }
}

TEST(Json5ToDynamicTest, CompareWithFollyDynamic) {
  auto json = R"({
    "primitives": {
      "boolTrue": true,
      "boolFalse": false,
      "null": null,
      "int": 42,
      "negativeInt": -42,
      "zero": 0,
      "negativeZero": -0.0,
      "float": 3.14,
      "int": Infinity,
      "-int": -Infinity,
      "nan": NaN,
      "largeInt": 9223372036854775807,
      "smallInt": -9223372036854775807,
      "smallFloat": 0.000001,
      "largeFloat": 9999999.999999
    },
    "strings": {
      "empty": "",
      "whitespace": "   ",
      "escapes": "tab:\t newline:\n quote:\" backslash:\\",
      "specialChars": "!@#$%^&*()_+-=[]{}|;':\",./<>?",
      "multiline": "line1\nline2\nline3"
    },
    "emptyContainers": {
      "emptyObj": {},
      "emptyArr": [],
      "nested": {"deep": {}}
    },
    "arrays": {
      "booleans": [true, false, true, true, false],
      "mixedTypes": [1, "two", 3.0, true, null, {"key": "value"}, [1, 2, 3]],
      "deeplyNested": [[[[[[1]]]]]],
      "numbers": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20],
      "withNulls": [null, 1, null, 2, null]
    },
    "nestedObjects": {
      "level1": {
        "level2": {
          "level3": {
            "level4": {
              "value": "deep"
            }
          }
        }
      }
    },
    "arrayOfObjects": [
      {"name": "Alice", "age": 30, "active": true},
      {"name": "Bob", "age": 25, "active": false},
      {"name": "Charlie", "age": 35, "active": true}
    ],
    "complexNested": {
      "users": [
        {"id": 1, "tags": ["admin", "user"], "metadata": {"created": 1234567890}},
        {"id": 2, "tags": ["user"], "metadata": {"created": 1234567891}}
      ],
      "settings": {"theme": "dark", "notifications": {"email": true, "sms": false}}
    },
    "numericKeys": {
      "123": "numeric key",
      "45.67": "float key",
      "0": "zero key"
    }
  })";

  auto v1 = json5ToDynamic(json);
  auto v2 = folly::parseJson(json);
  maskNan(v1), maskNan(v2);
  EXPECT_EQ(v1, v2);
}

constexpr auto kTestRootPath = "thrift/lib/cpp2/protocol/test/json5-tests";

TEST(Json5ToDynamicTest, Parse) {
  const std::filesystem::path root{
      (folly::test::find_resource(kTestRootPath) / "src").string()};
  std::unordered_set<std::string> processed;
  for (const auto& entry :
       std::filesystem::recursive_directory_iterator(root)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    SCOPED_TRACE(entry);
    auto relPath = std::filesystem::relative(entry.path(), root);
    if (*relPath.begin() == "todo") {
      // Skip `todo` directory
      continue;
    }
    std::string ext = entry.path().extension();
    if (ext != ".txt" && ext != ".js" && ext != ".json" && ext != ".json5") {
      // skip non-test files (.md, .editorconfig, .gitattributes, .errorSpec)
      continue;
    }

    std::string buffer;
    folly::readFile(entry.path().c_str(), buffer);

    if (ext == ".txt" || ext == ".js") {
      // Not valid json, it should fail to parse.
      EXPECT_THROW(json5ToDynamic(buffer), std::exception);
    } else {
      // Parse `buffer` with Json5Reader and folly::parseJson.
      // Both functions should return equivalent folly::dynamic.
      // Note that folly json5 support is experimental, though we only use in
      // unit-test so it's fine.
      auto v1 = json5ToDynamic(buffer);
      auto v2 = folly::parseJson(buffer, {.allow_json5_experimental = true});
      maskNan(v1), maskNan(v2);
      EXPECT_EQ(v1, v2);
    }
    processed.emplace(relPath);
  }

  // Sanity check
  EXPECT_TRUE(processed.contains("arrays/regular-array.json"));
  EXPECT_TRUE(processed.contains("arrays/trailing-comma-array.json5"));
  EXPECT_TRUE(processed.contains("arrays/leading-comma-array.js"));
  EXPECT_TRUE(processed.contains("arrays/no-comma-array.txt"));
}

} // namespace
} // namespace apache::thrift::json5::detail
