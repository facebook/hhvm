/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sys/stat.h>
#include <chrono>
#include <thread>

#include <boost/filesystem.hpp>
#include <folly/FileUtil.h>
#include <folly/json/dynamic.h>
#include <folly/testing/TestUtil.h>

#include "mcrouter/CarbonRouterInstanceBase.h"
#include "mcrouter/ConfigApi.h"

namespace facebook::memcache::mcrouter {

namespace {

// Mock ConfigApi for testing
class MockConfigApi : public ConfigApi {
 public:
  MockConfigApi() : ConfigApi(McrouterOptions()) {}

  MOCK_METHOD(
      bool,
      getConfigFile,
      (std::string & confFile, std::string& path),
      (override));
  MOCK_METHOD(folly::dynamic, getConfigSourcesInfo, (), (override));
  MOCK_METHOD(void, startObserving, (), (override));
  MOCK_METHOD(void, stopObserving, (pid_t), (noexcept, override));
  MOCK_METHOD(void, subscribeToTrackedSources, (), (override));
  MOCK_METHOD(void, abandonTrackedSources, (), (override));
};

// Test helper class that inherits from CarbonRouterInstanceBase to allow
// testing
class TestableRouterInstance : public CarbonRouterInstanceBase {
 public:
  TestableRouterInstance(
      McrouterOptions opts,
      std::unique_ptr<MockConfigApi> mockConfig)
      : CarbonRouterInstanceBase(std::move(opts)),
        mockConfigApi_(std::move(mockConfig)) {
    // Replace the default config API with our mock
    const_cast<std::unique_ptr<ConfigApi>&>(configApi_) =
        std::move(mockConfigApi_);
  }

  // Get the mock config API for setting expectations
  // MockConfigApi* getMockConfigApi() {
  //   return static_cast<MockConfigApi*>(configApi_.get());
  // }

  // Mock implementations of pure virtual methods
  ProxyBase* getProxyBase(size_t /* index */) const override {
    return nullptr;
  }

  size_t getProxyCpu() const override {
    return 0;
  }

  bool proxyCpuEnabled() const override {
    return false;
  }

  void scheduleStatsCpuWorker(bool /* enable */) override {}

  folly::StringPiece routerInfoName() const override {
    return "TestRouter";
  }

 private:
  std::unique_ptr<MockConfigApi> mockConfigApi_;
};

} // anonymous namespace

// Test fixture for end-to-end preprocessed config dump tests
class PreprocessedConfigDumpTest : public ::testing::Test {
 protected:
  void SetUp() override {
    tempDir_ = std::make_unique<folly::test::TemporaryDirectory>(
        "dump_config_e2e_test");
  }

  // Helper to create McrouterOptions for testing
  McrouterOptions createTestOptions() {
    McrouterOptions opts;
    opts.service_name = "test_service";
    opts.router_name = "test_router";
    opts.flavor_name = "test_flavor";
    opts.config_dump_root = tempDir_->path().string();
    opts.dump_preprocessed_config_interval_sec = 300; // 5 minutes in seconds
    opts.config_str = R"({
      "pools": {
        "test_pool": {
          "servers": ["test:11211", "test2:11211"]
        }
      },
      "route": "PoolRoute|test_pool"
    })";
    return opts;
  }

  boost::filesystem::path getExpectedFilePath() const {
    return boost::filesystem::path(tempDir_->path().string()) / "test_service" /
        "test_router" / "libmcrouter.test_service.test_flavor.ppc.json";
  }

  std::unique_ptr<folly::test::TemporaryDirectory> tempDir_;
};

// Test successful preprocessed config dump with valid config
TEST_F(PreprocessedConfigDumpTest, EndToEndSuccessfulConfigDump) {
  auto mockConfigApi = std::make_unique<MockConfigApi>();
  auto* mockPtr = mockConfigApi.get();

  auto opts = createTestOptions();

  // Set up expectations for the mock
  EXPECT_CALL(*mockPtr, getConfigFile(::testing::_, ::testing::_))
      .WillOnce(
          ::testing::DoAll(
              ::testing::SetArgReferee<0>(opts.config_str),
              ::testing::SetArgReferee<1>("test_config_path"),
              ::testing::Return(true)));

  TestableRouterInstance instance(std::move(opts), std::move(mockConfigApi));

  // Call the actual method under test
  instance.dumpPreprocessedConfigToDiskForTesting();

  // Verify the file was created in the expected location
  auto expectedFile = getExpectedFilePath();
  EXPECT_TRUE(boost::filesystem::exists(expectedFile));

  // Verify the file contains valid JSON
  std::string content;
  EXPECT_TRUE(folly::readFile(expectedFile.string().c_str(), content));

  folly::dynamic jsonData;
  EXPECT_NO_THROW(jsonData = folly::parseJson(content));

  // Verify the content has been preprocessed (should contain routing info)
  EXPECT_NE(content.find("test_pool"), std::string::npos);

  // Verify file permissions are set correctly (0664)
  struct stat fileStat{};
  EXPECT_EQ(0, stat(expectedFile.string().c_str(), &fileStat));
  EXPECT_EQ(
      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, fileStat.st_mode & 0777);
}

// Test that no file is created when config loading fails
TEST_F(PreprocessedConfigDumpTest, EndToEndConfigLoadFailure) {
  auto mockConfigApi = std::make_unique<MockConfigApi>();
  auto* mockPtr = mockConfigApi.get();

  auto opts = createTestOptions();

  // Mock failed config file retrieval
  EXPECT_CALL(*mockPtr, getConfigFile(::testing::_, ::testing::_))
      .WillOnce(::testing::Return(false));

  TestableRouterInstance instance(std::move(opts), std::move(mockConfigApi));

  // Call the method under test - should not throw
  EXPECT_NO_THROW(instance.dumpPreprocessedConfigToDiskForTesting());

  // Verify no file was created
  auto expectedFile = getExpectedFilePath();
  EXPECT_FALSE(boost::filesystem::exists(expectedFile));
}

// Test that subsequent dumps with same content only touch the file
TEST_F(PreprocessedConfigDumpTest, EndToEndSameContentOnlyTouchesFile) {
  auto mockConfigApi = std::make_unique<MockConfigApi>();
  auto* mockPtr = mockConfigApi.get();

  auto opts = createTestOptions();

  // Mock successful config file retrieval for both calls
  EXPECT_CALL(*mockPtr, getConfigFile(::testing::_, ::testing::_))
      .Times(2)
      .WillRepeatedly(
          ::testing::DoAll(
              ::testing::SetArgReferee<0>(opts.config_str),
              ::testing::SetArgReferee<1>("test_config_path"),
              ::testing::Return(true)));

  TestableRouterInstance instance(std::move(opts), std::move(mockConfigApi));

  // First call should create the file
  instance.dumpPreprocessedConfigToDiskForTesting();

  auto expectedFile = getExpectedFilePath();
  EXPECT_TRUE(boost::filesystem::exists(expectedFile));

  // Read initial content and get timestamp
  std::string initialContent;
  EXPECT_TRUE(folly::readFile(expectedFile.string().c_str(), initialContent));
  auto initialTime = boost::filesystem::last_write_time(expectedFile);

  // Wait a bit to ensure timestamp difference
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // Second call with same content should only touch the file
  instance.dumpPreprocessedConfigToDiskForTesting();

  // File should still exist
  EXPECT_TRUE(boost::filesystem::exists(expectedFile));

  // Content should remain unchanged
  std::string finalContent;
  EXPECT_TRUE(folly::readFile(expectedFile.string().c_str(), finalContent));
  EXPECT_EQ(initialContent, finalContent);

  // File modification time should be updated
  auto finalTime = boost::filesystem::last_write_time(expectedFile);
  EXPECT_GE(finalTime, initialTime);
}

// Test that different content causes file rewrite
TEST_F(PreprocessedConfigDumpTest, EndToEndDifferentContentCausesRewrite) {
  auto mockConfigApi = std::make_unique<MockConfigApi>();
  auto* mockPtr = mockConfigApi.get();

  auto opts = createTestOptions();

  std::string secondConfig = R"({
    "pools": {
      "different_pool": {
        "servers": ["different:11211"]
      }
    },
    "route": "PoolRoute|different_pool"
  })";

  // First call with initial config
  EXPECT_CALL(*mockPtr, getConfigFile(::testing::_, ::testing::_))
      .WillOnce(
          ::testing::DoAll(
              ::testing::SetArgReferee<0>(opts.config_str),
              ::testing::SetArgReferee<1>("test_config_path"),
              ::testing::Return(true)));

  TestableRouterInstance instance(std::move(opts), std::move(mockConfigApi));
  instance.dumpPreprocessedConfigToDiskForTesting();

  auto expectedFile = getExpectedFilePath();
  EXPECT_TRUE(boost::filesystem::exists(expectedFile));

  // Read first content
  std::string firstContent;
  EXPECT_TRUE(folly::readFile(expectedFile.string().c_str(), firstContent));

  // Second call with different config
  EXPECT_CALL(*mockPtr, getConfigFile(::testing::_, ::testing::_))
      .WillOnce(
          ::testing::DoAll(
              ::testing::SetArgReferee<0>(secondConfig),
              ::testing::SetArgReferee<1>("test_config_path"),
              ::testing::Return(true)));

  instance.dumpPreprocessedConfigToDiskForTesting();

  // Read second content
  std::string secondContent;
  EXPECT_TRUE(folly::readFile(expectedFile.string().c_str(), secondContent));

  // Content should be different
  EXPECT_NE(firstContent, secondContent);

  // Both should be valid JSON
  EXPECT_NO_THROW(folly::parseJson(firstContent));
  EXPECT_NO_THROW(folly::parseJson(secondContent));
}

// Test exception handling during config processing with invalid config
TEST_F(PreprocessedConfigDumpTest, EndToEndHandlesInvalidConfig) {
  auto mockConfigApi = std::make_unique<MockConfigApi>();
  auto* mockPtr = mockConfigApi.get();

  auto opts = createTestOptions();

  // Mock config with invalid JSON syntax
  std::string invalidConfig = "{ invalid json syntax [";

  EXPECT_CALL(*mockPtr, getConfigFile(::testing::_, ::testing::_))
      .WillOnce(
          ::testing::DoAll(
              ::testing::SetArgReferee<0>(invalidConfig),
              ::testing::SetArgReferee<1>("test_config_path"),
              ::testing::Return(true)));

  TestableRouterInstance instance(std::move(opts), std::move(mockConfigApi));

  // Call should not throw despite invalid config
  EXPECT_NO_THROW(instance.dumpPreprocessedConfigToDiskForTesting());

  // Should not create a file when config processing fails
  auto expectedFile = getExpectedFilePath();
  EXPECT_FALSE(boost::filesystem::exists(expectedFile));
}

// Test that output filename follows correct format
TEST_F(PreprocessedConfigDumpTest, EndToEndCorrectFilenameFormat) {
  auto mockConfigApi = std::make_unique<MockConfigApi>();
  auto* mockPtr = mockConfigApi.get();

  auto opts = createTestOptions();

  EXPECT_CALL(*mockPtr, getConfigFile(::testing::_, ::testing::_))
      .WillOnce(
          ::testing::DoAll(
              ::testing::SetArgReferee<0>(opts.config_str),
              ::testing::SetArgReferee<1>("test_config_path"),
              ::testing::Return(true)));

  TestableRouterInstance instance(std::move(opts), std::move(mockConfigApi));
  instance.dumpPreprocessedConfigToDiskForTesting();

  // Verify the filename follows the expected pattern:
  // libmcrouter.{service_name}.{flavor_name}.ppc.json
  auto expectedFile = getExpectedFilePath();
  EXPECT_TRUE(boost::filesystem::exists(expectedFile));

  std::string expectedFilename =
      "libmcrouter.test_service.test_flavor.ppc.json";
  EXPECT_EQ(expectedFile.filename().string(), expectedFilename);
}

// Test that missing config_dump_root prevents dumping
TEST_F(PreprocessedConfigDumpTest, EndToEndMissingConfigDumpRoot) {
  auto mockConfigApi = std::make_unique<MockConfigApi>();
  auto* mockPtr = mockConfigApi.get();

  auto opts = createTestOptions();
  opts.config_dump_root = ""; // Empty config dump root

  // Function still attempts to load config but should fail early
  EXPECT_CALL(*mockPtr, getConfigFile(::testing::_, ::testing::_))
      .WillOnce(::testing::Return(false));

  TestableRouterInstance instance(std::move(opts), std::move(mockConfigApi));

  // Call should not do anything when config_dump_root is empty
  instance.dumpPreprocessedConfigToDiskForTesting();
}

} // namespace facebook::memcache::mcrouter
