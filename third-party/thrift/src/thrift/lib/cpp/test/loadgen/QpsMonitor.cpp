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

#define __STDC_FORMAT_MACROS

#include <thrift/lib/cpp/test/loadgen/QpsMonitor.h>

#include <thrift/lib/cpp/concurrency/Util.h>
#include <thrift/lib/cpp/test/loadgen/LoadConfig.h>

#include <stdio.h>
#include <string.h>
#include <chrono>

using namespace boost;
using namespace apache::thrift::concurrency;

namespace apache {
namespace thrift {
namespace loadgen {

enum {
  US_PER_S = concurrency::Util::US_PER_S,
};

QpsMonitor::QpsMonitor(const std::shared_ptr<LoadConfig>& config)
    : initialTime_(),
      initialSum_(0),
      printAllTime_(true),
      enabledState_(config->getNumOpTypes()),
      aggregateScoreBoard_(config->getNumOpTypes()),
      config_(config),
      currentQps_(0) {}

std::shared_ptr<ScoreBoard> QpsMonitor::newScoreBoard(int /* id */) {
  std::shared_ptr<QpsScoreBoard> scoreboard(
      new QpsScoreBoard(config_->getNumOpTypes()));
  scoreboards_.push_back(scoreboard);
  return scoreboard;
}

void QpsMonitor::initializeInfo() {
  // The worker threads may have already started and performed
  // some operations by the time we start.  Record the current totals rather
  // than starting from 0, so that the QPS rates reported in the first interval
  // are accurate.
  computeAggregate(&aggregateScoreBoard_);

  // Record the start time and initial totals,
  // so we can use it for printing an all-time QPS rate.
  initialTime_ = std::chrono::steady_clock::now();
  initialSum_ = aggregateScoreBoard_.computeTotalCount();

  // Call our parent's initializeInfo() method
  TerminalMonitor::initializeInfo();
}

uint32_t QpsMonitor::printHeader() {
  printf("\n");

  bool printTotal = (enabledState_.getNumEnabled() > 1);

  uint32_t numOps = config_->getNumOpTypes();
  uint32_t numEnabled = 0;
  for (uint32_t op = 0; op < numOps; ++op) {
    if (enabledState_.isEnabled(op)) {
      printf("%17s ", config_->getOpName(op).c_str());
      ++numEnabled;
    }
  }
  if (printTotal) {
    printf("%15s", "Totals");
    ++numEnabled;
  }
  if (printAllTime_) {
    printf("%10s", "All Time");
  }
  printf("\n");

  for (uint32_t op = 0; op < numOps; ++op) {
    if (enabledState_.isEnabled(op)) {
      printf("%10s %6s ", "Total", "QPS");
    }
  }
  if (printTotal) {
    printf("%10s %6s", "Total", "QPS");
  }
  if (printAllTime_) {
    printf("%10s", "QPS");
  }
  printf("\n");

  uint32_t separatorLength = numEnabled * 18;
  if (printAllTime_) {
    separatorLength += 10;
  }
  char sep[separatorLength + 1];
  memset(sep, '-', separatorLength);
  sep[separatorLength] = '\0';
  printf("%s\n", sep);

  fflush(stdout);

  // we printed 3 lines
  return 3;
}

uint32_t QpsMonitor::printInfo(uint64_t intervalUsec) {
  // Store a snapshot of the data from the previous interval
  QpsScoreBoard prevScoreBoard = aggregateScoreBoard_;
  // Aggregate all of the worker's scoreboards into new data for this interval
  computeAggregate(&aggregateScoreBoard_);

  uint64_t currentSum = aggregateScoreBoard_.computeTotalCount();

  // Print the queries per second, broken down by operation
  for (uint32_t op = 0; op < config_->getNumOpTypes(); ++op) {
    if (enabledState_.isEnabled(op)) {
      uint64_t currentCount = aggregateScoreBoard_.getCount(op);
      uint64_t delta = currentCount - prevScoreBoard.getCount(op);
      uint64_t qps = (US_PER_S * delta) / intervalUsec;
      printf("%10" PRIu64 " %6" PRIu64 " ", currentCount, qps);
    }
  }

  // Print the aggregated queries per second
  uint64_t delta = currentSum - prevScoreBoard.computeTotalCount();
  uint64_t sumQps = (US_PER_S * delta) / intervalUsec;
  bool printTotal = (enabledState_.getNumEnabled() > 1);
  if (printTotal) {
    printf("%10" PRIu64 " %6" PRIu64, currentSum, sumQps);
  }
  currentQps_ = sumQps;

  // Print the all-time queries per second
  if (printAllTime_) {
    auto now = std::chrono::steady_clock::now();
    auto allTimeQps =
        ((currentSum - initialSum_) /
         std::chrono::duration<double>(now - initialTime_).count());
    printf("%10lf", allTimeQps);
  }

  printf("\n");

  fflush(stdout);

  // we printed 1 line
  return 1;
}

uint64_t QpsMonitor::getCurrentQps() {
  return currentQps_;
}

void QpsMonitor::computeAggregate(QpsScoreBoard* scoreboard) {
  // Zero out this scoreboard
  scoreboard->zero();

  // Sum the information in each Worker's scoreboard into
  // this aggregate scoreboard
  for (ScoreBoardVector::const_iterator it = scoreboards_.begin();
       it != scoreboards_.end();
       ++it) {
    scoreboard->accumulate(it->get());
  }
}

} // namespace loadgen
} // namespace thrift
} // namespace apache
