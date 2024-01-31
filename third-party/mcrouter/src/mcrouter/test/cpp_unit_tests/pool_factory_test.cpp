/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdexcept>

#include <gtest/gtest.h>

#include <folly/Range.h>
#include <folly/experimental/StringKeyedUnorderedMap.h>
#include <folly/json.h>

#include "mcrouter/ConfigApiIf.h"
#include "mcrouter/PoolFactory.h"

using namespace facebook::memcache::mcrouter;

namespace {

class MockConfigApi : public ConfigApiIf {
 public:
  MockConfigApi() = default;

  explicit MockConfigApi(folly::F14NodeMap<std::string, std::string> pools)
      : pools_(std::move(pools)) {}

  bool partialReconfigurableSource(const std::string&, std::string&) override {
    return false;
  }

  bool get(ConfigType type, const std::string& path, std::string& contents)
      final {
    ++getCalls_;
    if (type != ConfigType::Pool) {
      return false;
    }
    auto it = pools_.find(path);
    if (it != pools_.end()) {
      contents = it->second;
      return true;
    }
    return false;
  }

  bool getConfigFile(std::string& config, std::string& path) final {
    config = "{}";
    path = "{}";
    return true;
  }

  size_t getCalls() const {
    return getCalls_;
  }

 private:
  folly::F14NodeMap<std::string, std::string> pools_;
  size_t getCalls_{0};
};

} // namespace

TEST(PoolFactory, inherit_loop) {
  MockConfigApi api;
  PoolFactory factory(
      folly::parseJson(R"({
    "pools": {
      "A": {
        "inherit": "B"
      },
      "B": {
        "inherit": "C"
      },
      "C": {
        "inherit": "A"
      }
    }
  })"),
      api,
      folly::json::metadata_map{});
  try {
    factory.parsePool("A");
  } catch (const std::logic_error& e) {
    EXPECT_TRUE(folly::StringPiece(e.what()).contains("Cycle")) << e.what();
    return;
  }
  FAIL() << "No exception thrown on inherit cycle";
}

TEST(PoolFactory, inherit_cache) {
  MockConfigApi api(folly::F14NodeMap<std::string, std::string>{
      {"api_pool", "{ \"servers\": [ \"localhost:1234\" ] }"}});
  PoolFactory factory(
      folly::parseJson(R"({
    "pools": {
      "A": {
        "inherit": "api_pool",
        "server_timeout": 5
      },
      "B": {
        "inherit": "api_pool",
        "server_timeout": 10
      },
      "C": {
        "inherit": "A",
        "server_timeout": 15
      }
    }
  })"),
      api,
      folly::json::metadata_map{});
  auto poolA = factory.parsePool("A");
  EXPECT_EQ("A", poolA.name.str());
  EXPECT_EQ(5, poolA.json["server_timeout"].getInt());
  auto poolB = factory.parsePool("B");
  EXPECT_EQ("B", poolB.name.str());
  EXPECT_EQ(10, poolB.json["server_timeout"].getInt());
  auto poolC = factory.parsePool("C");
  EXPECT_EQ("C", poolC.name.str());
  EXPECT_EQ(15, poolC.json["server_timeout"].getInt());
  EXPECT_EQ(api.getCalls(), 1);
}
