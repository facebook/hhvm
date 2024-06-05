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

#include <folly/Synchronized.h>
#include <folly/executors/FunctionScheduler.h>
#include <wangle/ssl/TLSTicketKeySeeds.h>
#include <memory>

#pragma once

namespace wangle {

/**
 * A class that updates in memory ticket seeds and fires callbacks periodically
 * based on the updateInterval duration, which defaults to 4 hours.
 */
class TLSInMemoryTicketProcessor {
  static constexpr std::chrono::milliseconds kDefaultUpdateInterval =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::hours(2));

 public:
  TLSInMemoryTicketProcessor();
  explicit TLSInMemoryTicketProcessor(
      std::vector<std::function<void(wangle::TLSTicketKeySeeds)>> callbacks,
      std::chrono::milliseconds updateInterval = kDefaultUpdateInterval);

  TLSInMemoryTicketProcessor(TLSInMemoryTicketProcessor&&) = default;
  TLSInMemoryTicketProcessor& operator=(TLSInMemoryTicketProcessor&&) = default;

  virtual ~TLSInMemoryTicketProcessor();
  TLSTicketKeySeeds initInMemoryTicketSeeds();

  /* Add a callback fucntion to be fired periodically. */

 private:
  void initScheduler();
  void updateTicketSeeds() noexcept;

  std::unique_ptr<folly::FunctionScheduler> scheduler_;
  std::chrono::milliseconds updateInterval_{kDefaultUpdateInterval};
  std::vector<std::function<void(wangle::TLSTicketKeySeeds)>> ticketCallbacks_;
  TLSTicketKeySeeds ticketSeeds_;
};

} // namespace wangle
