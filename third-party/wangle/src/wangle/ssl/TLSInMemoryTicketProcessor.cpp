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

#include <folly/Random.h>
#include <wangle/ssl/SSLUtil.h>
#include <wangle/ssl/TLSInMemoryTicketProcessor.h>

namespace {
std::string generateRandomSeed() {
  uint8_t seed[32] = {0};
  folly::Random::secureRandom(seed, sizeof(seed));
  return wangle::SSLUtil::hexlify(std::string((char*)seed, sizeof(seed)));
}
} // namespace

namespace wangle {
TLSInMemoryTicketProcessor::TLSInMemoryTicketProcessor() = default;

TLSInMemoryTicketProcessor::TLSInMemoryTicketProcessor(
    std::vector<std::function<void(wangle::TLSTicketKeySeeds)>> callbacks,
    std::chrono::milliseconds updateInterval)
    : updateInterval_(updateInterval), ticketCallbacks_(callbacks) {}

TLSInMemoryTicketProcessor::~TLSInMemoryTicketProcessor() {
  if (scheduler_) {
    scheduler_->cancelAllFunctionsAndWait();
  }
}

TLSTicketKeySeeds TLSInMemoryTicketProcessor::initInMemoryTicketSeeds() {
  TLSTicketKeySeeds seedData;
  seedData.currentSeeds.push_back(generateRandomSeed());
  seedData.newSeeds.push_back(generateRandomSeed());
  ticketSeeds_ = seedData;
  initScheduler();
  return seedData;
}

void TLSInMemoryTicketProcessor::initScheduler() {
  scheduler_ = std::make_unique<folly::FunctionScheduler>();
  scheduler_->setThreadName("TLSInMemoryTicketProcessor");
  scheduler_->addFunction(
      [this] { this->updateTicketSeeds(); },
      updateInterval_,
      "TLSInMemoryTicketProcessor",
      updateInterval_);
  scheduler_->start();
}

void TLSInMemoryTicketProcessor::updateTicketSeeds() noexcept {
  TLSTicketKeySeeds updatedSeeds = {
      ticketSeeds_.currentSeeds, ticketSeeds_.newSeeds, {generateRandomSeed()}};
  ticketSeeds_ = updatedSeeds;
  for (auto& callback : ticketCallbacks_) {
    callback(updatedSeeds);
  }
}
} // namespace wangle
