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

#include <glog/logging.h>
#include <thrift/lib/cpp2/server/ParallelConcurrencyController.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>
#include <thrift/lib/cpp2/server/TokenBucketConcurrencyController.h>

namespace apache::thrift {

std::unique_ptr<ConcurrencyControllerInterface>
makeStandardConcurrencyController(
    RequestPileInterface& pile, folly::Executor& ex) {
  if (FLAGS_thrift_use_token_bucket_concurrency_controller) {
    LOG(INFO)
        << "Flag is set to use TokenBucketConcurrencyController as a stanard concurrency controller";
    return std::make_unique<TokenBucketConcurrencyController>(pile, ex);
  }

  LOG(INFO)
      << "ParallelConcurrencyController will be used as a standard concurrency controller";
  return std::make_unique<ParallelConcurrencyController>(pile, ex);
}

} // namespace apache::thrift
