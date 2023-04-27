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

#include <wangle/ssl/TLSCredProcessor.h>

#include <boost/filesystem.hpp>
#include <folly/File.h>
#include <folly/FileUtil.h>
#include <folly/Range.h>
#include <folly/portability/GTest.h>
#include <folly/portability/Stdlib.h>
#include <folly/synchronization/Baton.h>
#include <wangle/ssl/test/TicketUtil.h>

using namespace folly;
using namespace wangle;

namespace fs = boost::filesystem;

class ProcessTicketTest : public testing::Test {
 public:
  void SetUp() override {
    char ticketTemp[] = {"/tmp/ticketFile-XXXXXX"};
    File(mkstemp(ticketTemp), true);
    ticketFile = ticketTemp;
    char certTemp[] = {"/tmp/certFile-XXXXXX"};
    File(mkstemp(certTemp), true);
    certFile = certTemp;
  }

  void TearDown() override {
    remove(ticketFile.c_str());
    remove(certFile.c_str());
  }

  std::string ticketFile;
  std::string certFile;
};

void expectValidData(folly::Optional<wangle::TLSTicketKeySeeds> seeds) {
  ASSERT_TRUE(seeds);
  ASSERT_EQ(2, seeds->newSeeds.size());
  ASSERT_EQ(1, seeds->currentSeeds.size());
  ASSERT_EQ(0, seeds->oldSeeds.size());
  ASSERT_EQ(
      "0000111122223333444455556666777788889999aaaabbbbccccddddeeeeffff",
      seeds->newSeeds[0]);
  ASSERT_EQ(
      "111122223333444455556666777788889999aaaabbbbccccddddeeeeffff0000",
      seeds->newSeeds[1]);
}

TEST_F(ProcessTicketTest, ParseTicketFile) {
  CHECK(writeFile(validTicketData, ticketFile.c_str()));
  auto seeds = TLSCredProcessor::processTLSTickets(ticketFile);
  expectValidData(seeds);
}

TEST_F(ProcessTicketTest, ParseTicketFileWithPassword) {
  CHECK(writeFile(encryptedTicketString, ticketFile.c_str()));
  auto seeds =
      TLSCredProcessor::processTLSTickets(ticketFile, ticketPasswordString);
  expectValidData(seeds);
}

TEST_F(ProcessTicketTest, ParseInvalidFile) {
  CHECK(writeFile(invalidTicketData, ticketFile.c_str()));
  auto seeds = TLSCredProcessor::processTLSTickets(ticketFile);
  ASSERT_FALSE(seeds);
}

TEST_F(ProcessTicketTest, handleAbsentFile) {
  auto seeds = TLSCredProcessor::processTLSTickets("/path/does/not/exist");
  ASSERT_FALSE(seeds);
}

void updateModifiedTime(const std::string& fileName, int elapsed) {
  auto previous = fs::last_write_time(fileName);
  auto newTime = std::chrono::system_clock::to_time_t(
      std::chrono::system_clock::from_time_t(previous) +
      std::chrono::seconds(elapsed));
  fs::last_write_time(fileName, newTime);
}

TEST_F(ProcessTicketTest, TestUpdateTicketFile) {
  Baton<> ticketBaton;
  Baton<> certBaton;
  TLSCredProcessor processor;
  processor.setTicketPathToWatch(ticketFile);
  processor.setCertPathsToWatch({certFile});
  processor.addTicketCallback([&](TLSTicketKeySeeds) { ticketBaton.post(); });
  processor.addCertCallback([&]() { certBaton.post(); });
  CHECK(writeFile(validTicketData, ticketFile.c_str()));
  updateModifiedTime(ticketFile, 1);
  EXPECT_TRUE(ticketBaton.try_wait_for(std::chrono::seconds(15)));
  ticketBaton.reset();
  updateModifiedTime(ticketFile, 10);
  EXPECT_TRUE(ticketBaton.try_wait_for(std::chrono::seconds(15)));
  ticketBaton.reset();
  ASSERT_FALSE(certBaton.ready());
  CHECK(writeFile(validTicketData, certFile.c_str()));
  updateModifiedTime(certFile, 1);
  EXPECT_TRUE(certBaton.try_wait_for(std::chrono::seconds(15)));
  certBaton.reset();
  updateModifiedTime(certFile, 10);
  EXPECT_TRUE(certBaton.try_wait_for(std::chrono::seconds(15)));
  ASSERT_FALSE(ticketBaton.ready());
}

TEST_F(ProcessTicketTest, TestUpdateTicketFileWithPassword) {
  Baton<> ticketBaton;
  TLSCredProcessor processor;
  processor.setTicketPathToWatch(ticketFile, ticketPasswordString);
  processor.addTicketCallback([&](TLSTicketKeySeeds) { ticketBaton.post(); });

  CHECK(writeFile(encryptedTicketString, ticketFile.c_str()));
  updateModifiedTime(ticketFile, 1);
  EXPECT_TRUE(ticketBaton.try_wait_for(std::chrono::seconds(30)));
  ticketBaton.reset();
  updateModifiedTime(ticketFile, 10);
  EXPECT_TRUE(ticketBaton.try_wait_for(std::chrono::seconds(30)));
}

TEST_F(ProcessTicketTest, TestMultipleCerts) {
  Baton<> certBaton;
  TLSCredProcessor processor{std::chrono::milliseconds(250)};
  processor.addCertCallback([&]() { certBaton.post(); });
  processor.setCertPathsToWatch({certFile, ticketFile});

  CHECK(writeFile(validTicketData, ticketFile.c_str()));
  updateModifiedTime(ticketFile, 1);
  EXPECT_TRUE(certBaton.try_wait_for(std::chrono::seconds(1)));
  certBaton.reset();
  updateModifiedTime(ticketFile, 10);
  EXPECT_TRUE(certBaton.try_wait_for(std::chrono::seconds(1)));
  certBaton.reset();

  CHECK(writeFile(validTicketData, certFile.c_str()));
  updateModifiedTime(certFile, 1);
  EXPECT_TRUE(certBaton.try_wait_for(std::chrono::seconds(1)));
  certBaton.reset();
  updateModifiedTime(certFile, 10);
  EXPECT_TRUE(certBaton.try_wait_for(std::chrono::seconds(1)));
  certBaton.reset();
}

TEST_F(ProcessTicketTest, TestSetPullInterval) {
  Baton<> ticketBaton;
  Baton<> certBaton;
  TLSCredProcessor processor;
  processor.setTicketPathToWatch(ticketFile);
  processor.setCertPathsToWatch({certFile});
  processor.setPollInterval(std::chrono::seconds(3));
  processor.addTicketCallback([&](TLSTicketKeySeeds) { ticketBaton.post(); });
  processor.addCertCallback([&]() { certBaton.post(); });
  CHECK(writeFile(validTicketData, ticketFile.c_str()));
  updateModifiedTime(ticketFile, 1);
  EXPECT_TRUE(ticketBaton.try_wait_for(std::chrono::seconds(5)));
  ticketBaton.reset();
  updateModifiedTime(ticketFile, 3);
  EXPECT_TRUE(ticketBaton.try_wait_for(std::chrono::seconds(5)));
  ticketBaton.reset();
  ASSERT_FALSE(certBaton.ready());
  CHECK(writeFile(validTicketData, certFile.c_str()));
  updateModifiedTime(certFile, 1);
  EXPECT_TRUE(certBaton.try_wait_for(std::chrono::seconds(5)));
  certBaton.reset();
  updateModifiedTime(certFile, 3);
  EXPECT_TRUE(certBaton.try_wait_for(std::chrono::seconds(5)));
  certBaton.reset();
  ASSERT_FALSE(ticketBaton.ready());
}
