/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <thrift/lib/cpp/test/loadgen/QpsScoreBoard.h>

namespace apache {
namespace thrift {
namespace loadgen {

void QpsScoreBoard::opStarted(uint32_t /* opType */) {}

void QpsScoreBoard::opSucceeded(uint32_t opType) {
  OpData* data = opData_.getOpData(opType);
  ++data->count;
}

void QpsScoreBoard::opFailed(uint32_t /* opType */) {}

uint64_t QpsScoreBoard::getCount(uint32_t opType) const {
  const OpData* data = opData_.getOpDataOrNull(opType);
  return data ? data->count : 0;
}

uint64_t QpsScoreBoard::computeTotalCount() const {
  OpData accumulated;
  opData_.accumulateOverOps(&accumulated);
  return accumulated.count;
}

void QpsScoreBoard::zero() {
  opData_.zero();
}

void QpsScoreBoard::accumulate(const QpsScoreBoard* other) {
  opData_.accumulate(&other->opData_);
}

} // namespace loadgen
} // namespace thrift
} // namespace apache
