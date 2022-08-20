/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/TraceEvent.h>

#include <atomic>
#include <folly/json.h>
#include <sstream>
#include <string>

namespace proxygen {

// Helpers used to make TraceEventType/TraceFieldType can be used with GLOG
std::ostream& operator<<(std::ostream& os, TraceEventType eventType) {
  os << getTraceEventTypeString(eventType);
  return os;
}

std::ostream& operator<<(std::ostream& os, TraceFieldType fieldType) {
  os << getTraceFieldTypeString(fieldType);
  return os;
}

TraceEvent::TraceEvent(TraceEventType type, uint32_t parentID)
    : type_(type), parentID_(parentID) {
  static std::atomic<uint32_t> counter(0);
  id_ = counter++;
}

void TraceEvent::start(const TimeUtil& tm) {
  stateFlags_ |= State::STARTED;
  start_ = tm.now();
}

void TraceEvent::start(TimePoint startTime) {
  stateFlags_ |= State::STARTED;
  start_ = startTime;
}

void TraceEvent::end(const TimeUtil& tm) {
  stateFlags_ |= State::ENDED;
  end_ = tm.now();
}

void TraceEvent::end(TimePoint endTime) {
  stateFlags_ |= State::ENDED;
  end_ = endTime;
}

bool TraceEvent::hasStarted() const {
  return stateFlags_ & State::STARTED;
}

bool TraceEvent::hasEnded() const {
  return stateFlags_ & State::ENDED;
}

bool TraceEvent::readBoolMeta(TraceFieldType key, bool& dest) const {
  return readMeta(key, dest);
}

bool TraceEvent::readStrMeta(TraceFieldType key, std::string& dest) const {
  return readMeta(key, dest);
}
bool TraceEvent::addMetaInternal(TraceFieldType key, MetaData&& value) {
  auto rc = metaData_.emplace(key, value);

  // replace if key already exist
  if (!rc.second) {
    rc.first->second = std::move(value);
  }

  return rc.second;
}

std::string TraceEvent::toString() const {
  std::ostringstream out;
  int startSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(
                            start_.time_since_epoch())
                            .count();
  int endSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(
                          end_.time_since_epoch())
                          .count();
  out << "TraceEvent(";
  out << "type='" << getTraceEventTypeString(type_) << "', ";
  out << "id='" << id_ << "', ";
  out << "parentID='" << parentID_ << "', ";
  out << "start='" << startSinceEpoch << "', ";
  out << "end='" << endSinceEpoch << "', ";
  out << "metaData='{";
  auto itr = getMetaDataItr();
  while (itr.isValid()) {
    out << getTraceFieldTypeString(itr.getKey()) << ": "
        << itr.getValueAs<std::string>() << ", ";
    itr.next();
  }
  out << "}')";
  return out.str();
}

std::string TraceEvent::MetaData::ConvVisitor<std::string>::operator()(
    const std::vector<std::string>& operand) const {
  // parse string vector to json string.
  folly::dynamic data = folly::dynamic::array;
  for (auto item : operand) {
    data.push_back(item);
  }
  return folly::toJson(data);
}

std::ostream& operator<<(std::ostream& out, const TraceEvent& event) {
  out << event.toString();
  return out;
}
} // namespace proxygen
