/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>
#include <vector>

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <folly/FileUtil.h>
#include <folly/Range.h>
#include <folly/json.h>

#include "mcrouter/lib/config/ConfigPreprocessor.h"
#include "mcrouter/lib/config/ImportResolverIf.h"
#include "mcrouter/lib/fbi/cpp/util.h"

using facebook::memcache::ConfigPreprocessor;
using facebook::memcache::ImportResolverIf;
using facebook::memcache::parseJsonString;

const std::string kTestFile =
    "mcrouter/lib/config/test/config_preprocessor_test_file.json";
const std::string kTestFileErrors =
    "mcrouter/lib/config/test/config_preprocessor_test_errors.json";
const std::string kTestFileComments =
    "mcrouter/lib/config/test/config_preprocessor_test_comments.json";

const folly::F14NodeMap<std::string, folly::dynamic> kGlobalParams = {
    {"testGlobal", "test"},
    {"templGlobal", "templ"}};

class MockImportResolver : public ImportResolverIf {
  std::string import(folly::StringPiece path) final {
    if (path == "test") {
      return "\"mock_test\"";
    }
    if (path == "templ") {
      return "{ \"type\": \"macroDef\", \"result\": \"imported_macro\" }";
    }
    if (path == "do_not_exist") {
      throw std::runtime_error(std::string("Can not import") + path.str());
    }
    return "";
  }
};

void runCase(
    const folly::dynamic& consts,
    const folly::dynamic& macros,
    const std::string& caseName,
    const folly::dynamic& caseObj) {
  LOG(INFO) << "  Case: " << caseName;
  auto orig = caseObj["orig"];
  auto expand = caseObj["expand"];

  folly::dynamic obj =
      folly::dynamic::object("consts", consts)("macros", macros)("orig", orig);
  MockImportResolver resolver;
  folly::json::metadata_map configMetadataMap;
  auto result = ConfigPreprocessor::getConfigWithoutMacros(
      folly::toJson(obj), resolver, kGlobalParams, &configMetadataMap);

  auto origExpand = result["orig"];

  folly::json::serialization_opts opts;
  opts.sort_keys = true;
  opts.pretty_formatting = false;

  auto jsonExpand = folly::json::serialize(expand, opts);
  auto jsonOrigExpand = folly::json::serialize(origExpand, opts);

  ASSERT_EQ(jsonExpand, jsonOrigExpand) << "Case: " << caseName << " fail";
  LOG(INFO) << "  success";
}

void runTest(const std::string& testName, const folly::dynamic& testObj) {
  LOG(INFO) << "Test: " << testName;

  if (!testObj.count("macros")) {
    throw std::logic_error("TestObj without macros");
  }
  folly::dynamic consts = folly::dynamic::array;
  if (testObj.count("consts")) {
    consts = testObj["consts"];
  }
  for (auto& it : testObj["cases"].items()) {
    auto caseName = it.first.asString();
    auto caseObj = it.second;
    runCase(consts, testObj["macros"], testName + ":" + caseName, it.second);
  }
}

TEST(ConfigPreprocessorTest, macros) {
  std::string jsonStr;
  if (!folly::readFile(kTestFile.data(), jsonStr)) {
    FAIL() << "can not read test file " << kTestFile;
  }

  auto json = parseJsonString(jsonStr);

  for (auto& test : json.items()) {
    auto testName = test.first.asString();
    auto testObj = test.second;

    runTest(testName, testObj);
  }
}

TEST(ConfigPreprocessorTest, errors) {
  MockImportResolver resolver;

  std::string jsonStr;
  if (!folly::readFile(kTestFileErrors.data(), jsonStr)) {
    FAIL() << "can not read test file " << kTestFileErrors;
  }

  auto json = parseJsonString(jsonStr);

  for (auto testCase : json.items()) {
    LOG(INFO) << "Case: " << testCase.first.asString();
    auto caseStr = folly::toJson(testCase.second);
    try {
      folly::json::metadata_map configMetadataMap;
      ConfigPreprocessor::getConfigWithoutMacros(
          caseStr, resolver, kGlobalParams, &configMetadataMap);
    } catch (const std::logic_error& e) {
      LOG(INFO) << "success " << e.what();
      continue;
    }
    FAIL() << "Case " << testCase.first.asString() << ": No error thrown";
  }
}

TEST(ConfigPreprocessorTest, comments) {
  MockImportResolver resolver;

  std::string jsonStr;
  if (!folly::readFile(kTestFileComments.data(), jsonStr)) {
    FAIL() << "can not read test file " << kTestFileErrors;
  }

  folly::json::metadata_map configMetadataMap;
  auto json = ConfigPreprocessor::getConfigWithoutMacros(
      jsonStr, resolver, kGlobalParams, &configMetadataMap);

  folly::json::serialization_opts opts;
  opts.sort_keys = true;
  opts.pretty_formatting = false;

  auto orig = folly::json::serialize(json["orig"], opts);
  auto expand = folly::json::serialize(json["expand"], opts);

  EXPECT_EQ(orig, expand);
}
