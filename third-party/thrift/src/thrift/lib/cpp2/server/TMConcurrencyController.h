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

#pragma once

#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp2/server/ParallelConcurrencyController.h>

namespace apache::thrift {
class TMConcurrencyController : public ParallelConcurrencyControllerBase {
 public:
  TMConcurrencyController(
      RequestPileInterface& pile, concurrency::ThreadManager& tm)
      : ParallelConcurrencyControllerBase(pile), tm_(tm) {}
  std::string describe() const override;

  serverdbginfo::ConcurrencyControllerDbgInfo getDbgInfo() const override;

 private:
  concurrency::ThreadManager& tm_;

  void scheduleOnExecutor() override;
};
} // namespace apache::thrift
