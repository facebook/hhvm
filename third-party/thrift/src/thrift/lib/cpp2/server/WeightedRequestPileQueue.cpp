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

#include <thrift/lib/cpp2/server/WeightedRequestPileQueue.h>

namespace apache::thrift::server {

RequestPileQueueAcceptResult OneDimensionalControlBlock::accept(
    Weights weight) {
  DCHECK(weight);
  for (;;) {
    auto oldCounter = counter_.load();
    auto counter = oldCounter;

    counter += weight;

    if (counter > limit_.load()) {
      return RequestPileQueueAcceptResult::Failed;
    }

    if (counter_.compare_exchange_weak(oldCounter, counter)) {
      if (oldCounter == 0) {
        return RequestPileQueueAcceptResult::FirstSuccess;
      } else {
        return RequestPileQueueAcceptResult::Success;
      }
    }
  }
}

RequestPileQueueAcceptResult TwoDimensionalControlBlock::accept(
    Weights weights) {
  auto [w1, w2] = weights;
  DCHECK(w1);
  DCHECK(w2);

  for (;;) {
    auto oldCounter1 = counter1_.load();
    auto oldCounter2 = counter2_.load();

    auto counter1 = oldCounter1;
    auto counter2 = oldCounter2;

    counter1 += w1;
    counter2 += w2;

    if (counter1 > limit1_.load() || counter2 > limit2_.load()) {
      return RequestPileQueueAcceptResult::Failed;
    }

    if (!counter1_.compare_exchange_weak(oldCounter1, counter1)) {
      continue;
    }

    if (!counter2_.compare_exchange_weak(oldCounter2, counter2)) {
      // roll back counter 1
      counter1_ -= w1;
      continue;
    }

    // Here we only check oldCounter2 because we change counter1 then
    // counter2, when counter2 is set it's guarantee that counter1
    // has been set. But the other way is not necessarily true
    if (oldCounter2 == 0) {
      return RequestPileQueueAcceptResult::FirstSuccess;
    }

    return RequestPileQueueAcceptResult::Success;
  }
}

} // namespace apache::thrift::server
