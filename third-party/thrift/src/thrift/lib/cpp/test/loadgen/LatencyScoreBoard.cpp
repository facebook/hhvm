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

#include <thrift/lib/cpp/test/loadgen/LatencyScoreBoard.h>

#include <thrift/lib/cpp/concurrency/Util.h>

#include <math.h>

DEFINE_int64(thriftLatencyBucketMax, 5000, "Maximum latency bucket in ms.");

namespace apache {
namespace thrift {
namespace loadgen {

/*
 * LatencyScoreBoard::OpData methods
 */

LatencyScoreBoard::OpData::OpData()
    : latDistHist_(50, 0, FLAGS_thriftLatencyBucketMax * 1000) {
  zero();
}

void LatencyScoreBoard::OpData::addDataPoint(uint64_t latency) {
  ++count_;
  usecSum_ += latency;
  sumOfSquares_ += latency * latency;
  latDistHist_.addValue(latency);
}

void LatencyScoreBoard::OpData::zero() {
  count_ = 0;
  usecSum_ = 0;
  sumOfSquares_ = 0;
  latDistHist_.clear();
}

void LatencyScoreBoard::OpData::accumulate(const OpData* other) {
  count_ += other->count_;
  usecSum_ += other->usecSum_;
  sumOfSquares_ += other->sumOfSquares_;
  latDistHist_.merge(other->latDistHist_);
}

uint64_t LatencyScoreBoard::OpData::getCount() const {
  return count_;
}

uint64_t LatencyScoreBoard::OpData::getCountSince(const OpData* other) const {
  return count_ - other->count_;
}

double LatencyScoreBoard::OpData::getLatencyAvg() const {
  if (count_ == 0) {
    return 0;
  }
  return static_cast<double>(usecSum_) / count_;
}

double LatencyScoreBoard::OpData::getLatencyPct(double pct) const {
  if (count_ == 0) {
    return 0;
  }
  uint64_t pct_lat = latDistHist_.getPercentileEstimate(pct);
  if (pct_lat > size_t(FLAGS_thriftLatencyBucketMax) * 1000) {
    LOG(WARNING) << "Estimated percentile latency " << pct_lat / 1000
                 << " ms is greater than the maximum bucket value "
                 << FLAGS_thriftLatencyBucketMax << " ms.";
  }
  return pct_lat;
}

double LatencyScoreBoard::OpData::getLatencyPctSince(
    double pct, const OpData* other) const {
  if (other->count_ >= count_) {
    return 0;
  }
  folly::Histogram<uint64_t> tmp = latDistHist_;
  tmp.subtract(other->latDistHist_);
  uint64_t pct_lat = tmp.getPercentileEstimate(pct);
  if (pct_lat > size_t(FLAGS_thriftLatencyBucketMax) * 1000) {
    LOG(WARNING) << "Estimated percentile latency " << pct_lat / 1000
                 << " ms is greater than the maximum bucket value "
                 << FLAGS_thriftLatencyBucketMax << " ms.";
  }
  return pct_lat;
}

double LatencyScoreBoard::OpData::getLatencyAvgSince(
    const OpData* other) const {
  if (other->count_ >= count_) {
    return 0;
  }
  return (
      static_cast<double>(usecSum_ - other->usecSum_) /
      (count_ - other->count_));
}

double LatencyScoreBoard::OpData::getLatencyStdDev() const {
  if (count_ == 0) {
    return 0;
  }
  return sqrt((sumOfSquares_ - usecSum_ * (usecSum_ / count_)) / count_);
}

double LatencyScoreBoard::OpData::getLatencyStdDevSince(
    const OpData* other) const {
  if (other->count_ >= count_) {
    return 0;
  }

  uint64_t deltaSumOfSquares = sumOfSquares_ - other->sumOfSquares_;
  uint64_t deltaCount = count_ - other->count_;
  uint64_t deltaSum = usecSum_ - other->usecSum_;
  return sqrt(
      (deltaSumOfSquares - deltaSum * (deltaSum / deltaCount)) / deltaCount);
}

/*
 * LatencyScoreBoard methods
 */

void LatencyScoreBoard::opStarted(uint32_t /* opType */) {
  startTime_ = std::chrono::steady_clock::now();
}

void LatencyScoreBoard::opSucceeded(uint32_t opType) {
  OpData* data = opData_.getOpData(opType);

  auto now = std::chrono::steady_clock::now();
  auto latency =
      std::chrono::duration_cast<std::chrono::microseconds>(now - startTime_);
  data->addDataPoint(latency.count());
}

void LatencyScoreBoard::opFailed(uint32_t /* opType */) {}

const LatencyScoreBoard::OpData* LatencyScoreBoard::getOpData(uint32_t opType) {
  return opData_.getOpData(opType);
}

void LatencyScoreBoard::computeOpAggregate(OpData* result) const {
  opData_.accumulateOverOps(result);
}

void LatencyScoreBoard::zero() {
  opData_.zero();
}

void LatencyScoreBoard::accumulate(const LatencyScoreBoard* other) {
  opData_.accumulate(&other->opData_);
}

} // namespace loadgen
} // namespace thrift
} // namespace apache
