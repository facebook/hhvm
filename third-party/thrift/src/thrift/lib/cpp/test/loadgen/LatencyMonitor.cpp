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

#include <thrift/lib/cpp/test/loadgen/LatencyMonitor.h>

#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp/concurrency/Util.h>
#include <thrift/lib/cpp/test/loadgen/WeightedLoadConfig.h>

DEFINE_double(
    thriftLatencyMonPct,
    99,
    "x-th percentail latencies to show in stats: (0 - 100)");

using std::shared_ptr;

namespace apache {
namespace thrift {
namespace loadgen {

enum {
  US_PER_S = concurrency::Util::US_PER_S,
};

LatencyMonitor::LatencyMonitor(const shared_ptr<LoadConfig>& config)
    : numOpTypes_(config->getNumOpTypes()),
      opFields_(),
      initialTime_(),
      initialScoreBoard_(numOpTypes_),
      aggregateScoreBoard_(numOpTypes_),
      config_(config),
      currentQps_(0) {
  setDefaultOpFields();
  CHECK(FLAGS_thriftLatencyMonPct > 0 && FLAGS_thriftLatencyMonPct < 100);
}

void LatencyMonitor::setFields(uint32_t opType, const FieldInfoVector* fields) {
  if (opType >= opFields_.size()) {
    assert(false);
    return;
  }
  opFields_[opType] = *fields;
}

void LatencyMonitor::setTotalFields(const FieldInfoVector* fields) {
  totalFields_ = *fields;
}

shared_ptr<ScoreBoard> LatencyMonitor::newScoreBoard(int /* id */) {
  shared_ptr<LatencyScoreBoard> scoreboard(new LatencyScoreBoard(numOpTypes_));
  scoreboards_.push_back(scoreboard);
  return scoreboard;
}

void LatencyMonitor::initializeInfo() {
  // The worker threads may have already started and performed
  // some operations by the time we start.  Record the current totals rather
  // than starting from 0, so that the QPS rates reported in the first interval
  // are accurate.
  aggregateWorkerScorboards(&initialScoreBoard_);
  aggregateScoreBoard_ = initialScoreBoard_;

  // Record the start time
  initialTime_ = std::chrono::steady_clock::now();

  // Print information about how to read the output
  printLegend();

  // Call our parent's initializeInfo() method
  TerminalMonitor::initializeInfo();
}

uint32_t LatencyMonitor::printHeader() {
  printf("\n");
  uint32_t linesPrinted = 1;

  // Print a line containing the operation names
  for (uint32_t op = 0; op < numOpTypes_; ++op) {
    FieldInfoVector* fields = &opFields_[op];
    if (!fields->empty()) {
      int opWidth = getFieldVectorWidth(fields);
      printField(config_->getOpName(op).c_str(), opWidth);
      printf("| ");
    }
  }
  if (!totalFields_.empty()) {
    int totalWidth = getFieldVectorWidth(&totalFields_);
    printField("Total", totalWidth);
  }
  printf("\n");
  ++linesPrinted;

  // Print a line with the field labels
  for (uint32_t op = 0; op < numOpTypes_; ++op) {
    FieldInfoVector* fields = &opFields_[op];
    for (FieldInfoVector::iterator it = fields->begin(); it != fields->end();
         ++it) {
      printField(getFieldName(it->field), &(*it));
    }
    if (!fields->empty()) {
      printf("| ");
    }
  }
  for (FieldInfoVector::iterator it = totalFields_.begin();
       it != totalFields_.end();
       ++it) {
    printField(getFieldName(it->field), &(*it));
  }
  printf("\n");
  ++linesPrinted;

  fflush(stdout);

  return linesPrinted;
}

uint32_t LatencyMonitor::printInfo(uint64_t intervalUsec) {
  // Store a snapshot of the data from the previous interval
  LatencyScoreBoard prevScoreBoard = aggregateScoreBoard_;
  // Aggregate all of the worker's scoreboards into new data for this interval
  aggregateWorkerScorboards(&aggregateScoreBoard_);

  auto now = std::chrono::steady_clock::now();
  auto allTimeUsec =
      std::chrono::duration_cast<std::chrono::microseconds>(now - initialTime_);

  uint64_t totalQueries = 0;

  // Print per-operation statistics
  for (uint32_t op = 0; op < numOpTypes_; ++op) {
    FieldInfoVector* fields = &opFields_[op];
    if (fields->empty()) {
      continue;
    }

    const LatencyScoreBoard::OpData* current =
        aggregateScoreBoard_.getOpData(op);
    const LatencyScoreBoard::OpData* prev = prevScoreBoard.getOpData(op);
    const LatencyScoreBoard::OpData* initial = initialScoreBoard_.getOpData(op);

    printOpInfo(
        fields, current, prev, initial, intervalUsec, allTimeUsec.count());
    printf("| ");

    totalQueries += current->getCountSince(prev);
  }
  currentQps_ = US_PER_S * totalQueries / intervalUsec;

  // Print the aggregated statistics across all operatiosn
  if (!totalFields_.empty()) {
    LatencyScoreBoard::OpData current;
    LatencyScoreBoard::OpData prev;
    LatencyScoreBoard::OpData initial;

    aggregateScoreBoard_.computeOpAggregate(&current);
    prevScoreBoard.computeOpAggregate(&prev);
    initialScoreBoard_.computeOpAggregate(&initial);

    printOpInfo(
        &totalFields_,
        &current,
        &prev,
        &initial,
        intervalUsec,
        allTimeUsec.count());
  }

  printf("\n");

  fflush(stdout);

  // we printed 1 line
  return 1;
}

uint64_t LatencyMonitor::getCurrentQps() {
  return currentQps_;
}

void LatencyMonitor::printOpHeader(FieldInfoVector* fields) {
  for (FieldInfoVector::const_iterator it = fields->begin();
       it != fields->end();
       ++it) {
    printf("%*s ", it->width - 1, getFieldName(it->field));
  }
}

void LatencyMonitor::printOpInfo(
    FieldInfoVector* fields,
    const LatencyScoreBoard::OpData* current,
    const LatencyScoreBoard::OpData* prev,
    const LatencyScoreBoard::OpData* initial,
    uint64_t intervalUsec,
    uint64_t allTimeUsec) {
  for (FieldInfoVector::iterator it = fields->begin(); it != fields->end();
       ++it) {
    char buf[128];
    formatFieldValue(
        it->field,
        buf,
        sizeof(buf),
        current,
        prev,
        initial,
        intervalUsec,
        allTimeUsec);

    printField(buf, &(*it));
  }
}

uint32_t LatencyMonitor::getFieldVectorWidth(
    const FieldInfoVector* fields) const {
  int width = 0;
  for (FieldInfoVector::const_iterator it = fields->begin();
       it != fields->end();
       ++it) {
    if (it->width <= 0) {
      width += getDefaultFieldWidth(it->field);
    } else {
      width += it->width;
    }
  }

  return width;
}

const char* LatencyMonitor::getFieldName(FieldEnum field) const {
  switch (field) {
    case FIELD_COUNT:
      return "Count";
    case FIELD_QPS:
      return "QPS";
    case FIELD_LATENCY:
      return "Latency";
    case FIELD_PCT_LATENCY:
      return "PCT Latency";
    case FIELD_ALL_TIME_COUNT:
      return "Tot. Count";
    case FIELD_ALL_TIME_QPS:
      return "Tot. QPS";
    case FIELD_ALL_TIME_LATENCY:
      return "Tot. Latency";
    case FIELD_ALL_TIME_PCT_LATENCY:
      return "Tot. PCT Latency";
  }

  assert(false);
  throw TLibraryException("unknown field type");
}

uint32_t LatencyMonitor::getDefaultFieldWidth(FieldEnum field) const {
  switch (field) {
    case FIELD_COUNT:
      return 10;
    case FIELD_QPS:
      return 8;
    case FIELD_LATENCY:
      return 12;
    case FIELD_PCT_LATENCY:
      return 8;
    case FIELD_ALL_TIME_COUNT:
      return 12;
    case FIELD_ALL_TIME_QPS:
      return 10;
    case FIELD_ALL_TIME_LATENCY:
      return 14;
    case FIELD_ALL_TIME_PCT_LATENCY:
      return 10;
  }

  assert(false);
  throw TLibraryException("unknown field type");
}

void LatencyMonitor::formatFieldValue(
    FieldEnum field,
    char* buf,
    size_t buflen,
    const LatencyScoreBoard::OpData* current,
    const LatencyScoreBoard::OpData* prev,
    const LatencyScoreBoard::OpData* initial,
    uint64_t intervalUsec,
    uint64_t allTimeUsec) {
  switch (field) {
    case FIELD_COUNT:
      snprintf(buf, buflen, "%" PRIu64, current->getCountSince(prev));
      return;
    case FIELD_QPS:
      snprintf(
          buf,
          buflen,
          "%" PRIu64,
          (US_PER_S * current->getCountSince(prev)) / intervalUsec);
      return;
    case FIELD_LATENCY:
      formatLatency(
          buf,
          buflen,
          current->getLatencyAvgSince(prev),
          current->getLatencyStdDevSince(prev));
      return;
    case FIELD_PCT_LATENCY:
      formatLatency(
          buf,
          buflen,
          current->getLatencyPctSince(FLAGS_thriftLatencyMonPct / 100, prev));
      return;
    case FIELD_ALL_TIME_COUNT:
      snprintf(buf, buflen, "%" PRIu64, current->getCountSince(initial));
      return;
    case FIELD_ALL_TIME_QPS:
      snprintf(
          buf,
          buflen,
          "%" PRIu64,
          (US_PER_S * current->getCountSince(initial)) / allTimeUsec);
      return;
    case FIELD_ALL_TIME_LATENCY:
      formatLatency(
          buf,
          buflen,
          current->getLatencyAvgSince(initial),
          current->getLatencyStdDevSince(initial));
      return;
    case FIELD_ALL_TIME_PCT_LATENCY:
      formatLatency(
          buf,
          buflen,
          current->getLatencyPctSince(
              FLAGS_thriftLatencyMonPct / 100, initial));
      return;
  }

  assert(false);
  throw TLibraryException("unknown field type");
}

void LatencyMonitor::formatLatency(char* buf, size_t buflen, double pct) {
  int pctPrecision = 0;
  if (pct < 1) {
    pctPrecision = 2;
  } else if (pct < 10) {
    pctPrecision = 1;
  }

  snprintf(buf, buflen, "%.*f", pctPrecision, pct);
}

void LatencyMonitor::formatLatency(
    char* buf, size_t buflen, double avg, double stddev) {
  int avgPrecision = 0;
  if (avg < 1) {
    avgPrecision = 2;
  } else if (avg < 10) {
    avgPrecision = 1;
  }

  int stddevPrecision = 0;
  if (stddev < 1) {
    stddevPrecision = 2;
  } else if (stddev < 10) {
    stddevPrecision = 1;
  }

  snprintf(
      buf, buflen, "%.*f/%.*f", avgPrecision, avg, stddevPrecision, stddev);
}

void LatencyMonitor::aggregateWorkerScorboards(LatencyScoreBoard* scoreboard) {
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

void LatencyMonitor::printField(const char* value, int width) {
  // We add an extra space in the format string, so there is always at least 1
  // space between fields, even if one of the values is too large.  Therefore,
  // decrement width by 1 when passing it in to printf().
  printf("%*s ", width - 1, value);
}

void LatencyMonitor::printField(const char* value, FieldInfo* fieldInfo) {
  int actualWidth = fieldInfo->width;
  if (actualWidth <= 0) {
    actualWidth = getDefaultFieldWidth(fieldInfo->field);
  }

  int widthPrinted = printf("%*s ", actualWidth - 1, value);

  // If dynamicWidth is true, and the value was too wide for the width,
  // increase the width to use on subsequent rows
  if (fieldInfo->dynamicWidth && widthPrinted > fieldInfo->width) {
    fieldInfo->width = widthPrinted;
  }
}

/**
 * Initialize opFields_ and totalFields_ to some reasonable default values
 */
void LatencyMonitor::setDefaultOpFields() {
  opFields_.resize(numOpTypes_);

  // Special case handling for WeightedLoadConfig.
  // We want to avoid printing statistics for fields that have a 0 weight.
  shared_ptr<WeightedLoadConfig> weightedConfig =
      std::dynamic_pointer_cast<WeightedLoadConfig>(config_);

  uint32_t numEnabledOps = 0;
  if (weightedConfig) {
    for (uint32_t op = 0; op < numOpTypes_; ++op) {
      if (weightedConfig->getOpWeight(op) > 0) {
        ++numEnabledOps;
      }
    }
  } else {
    numEnabledOps = numOpTypes_;
  }

  FieldInfoVector defaultFields;

  if (numEnabledOps == 1) {
    // If there is just 1 operation, print all statistics for it
    defaultFields.push_back(FieldInfo(FIELD_QPS));
    defaultFields.push_back(FieldInfo(FIELD_LATENCY));
    defaultFields.push_back(FieldInfo(FIELD_PCT_LATENCY));
    defaultFields.push_back(FieldInfo(FIELD_ALL_TIME_QPS));
    defaultFields.push_back(FieldInfo(FIELD_ALL_TIME_LATENCY));
    defaultFields.push_back(FieldInfo(FIELD_ALL_TIME_PCT_LATENCY));
  } else {
    // Otherwise, print the QPS and latency for each operation
    defaultFields.push_back(FieldInfo(FIELD_QPS));
    defaultFields.push_back(FieldInfo(FIELD_LATENCY));
    defaultFields.push_back(FieldInfo(FIELD_PCT_LATENCY));
    // And the print the all-time QPS summed across all operations
    totalFields_.push_back(FieldInfo(FIELD_ALL_TIME_QPS));
  }

  for (uint32_t op = 0; op < numOpTypes_; ++op) {
    if (weightedConfig && weightedConfig->getOpWeight(op) <= 0) {
      continue;
    }
    opFields_[op] = defaultFields;
  }
}

void LatencyMonitor::printLegend() {
  printf("Field Labels:\n");

  if (isFieldInUse(FIELD_COUNT)) {
    printf(
        "  %10s: number of operations in the last interval\n",
        getFieldName(FIELD_COUNT));
  }
  if (isFieldInUse(FIELD_QPS)) {
    printf(
        "  %10s: queries per second in the last interval\n",
        getFieldName(FIELD_QPS));
  }
  if (isFieldInUse(FIELD_LATENCY)) {
    printf(
        "  %10s: average microseconds per operation over the last interval\n"
        "  %10s  displayed value is (average/standard deviation)\n",
        getFieldName(FIELD_LATENCY),
        "");
  }
  if (isFieldInUse(FIELD_PCT_LATENCY)) {
    printf(
        "  %10s: %2.1fth percentile microseconds per operation over\
        the last interval\n",
        getFieldName(FIELD_PCT_LATENCY),
        FLAGS_thriftLatencyMonPct);
  }
  if (isFieldInUse(FIELD_ALL_TIME_COUNT)) {
    printf(
        "  %10s: number of operations since the test started\n",
        getFieldName(FIELD_ALL_TIME_COUNT));
  }
  if (isFieldInUse(FIELD_ALL_TIME_QPS)) {
    printf(
        "  %10s: average queries per second since the test started\n",
        getFieldName(FIELD_ALL_TIME_QPS));
  }
  if (isFieldInUse(FIELD_ALL_TIME_LATENCY)) {
    printf(
        "  %10s: average microseconds per operation since the test started\n"
        "  %10s  displayed value is (average/standard deviation)\n",
        getFieldName(FIELD_ALL_TIME_LATENCY),
        "");
  }
  if (isFieldInUse(FIELD_ALL_TIME_PCT_LATENCY)) {
    printf(
        "  %10s: %2.1fth percentile microseconds per operation since\
        the test started\n",
        getFieldName(FIELD_ALL_TIME_PCT_LATENCY),
        FLAGS_thriftLatencyMonPct);
  }

  fflush(stdout);
}

bool LatencyMonitor::isFieldInUse(FieldEnum field) {
  for (uint32_t op = 0; op < numOpTypes_; ++op) {
    const FieldInfoVector* fields = &opFields_[op];
    for (FieldInfoVector::const_iterator it = fields->begin();
         it != fields->end();
         ++it) {
      if (it->field == field) {
        return true;
      }
    }
  }

  for (FieldInfoVector::const_iterator it = totalFields_.begin();
       it != totalFields_.end();
       ++it) {
    if (it->field == field) {
      return true;
    }
  }

  return false;
}

} // namespace loadgen
} // namespace thrift
} // namespace apache
