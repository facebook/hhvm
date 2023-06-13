/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/utils/Exception.h>
#include <proxygen/lib/utils/Export.h>
#include <proxygen/lib/utils/Time.h>
#include <proxygen/lib/utils/TraceEventType.h>
#include <proxygen/lib/utils/TraceFieldType.h>

#include <boost/variant.hpp>
#include <glog/logging.h>

#include <folly/Conv.h>
#include <folly/lang/Exception.h>

#include <map>
#include <string>
#include <vector>

namespace proxygen {
// Helpers used to make TraceEventType/TraceFieldType can be used with GLOG
FB_EXPORT std::ostream& operator<<(std::ostream& os, TraceEventType eventType);
FB_EXPORT std::ostream& operator<<(std::ostream& os, TraceFieldType fieldType);

/**
 * Simple structure to track timing of event in request flow then we can
 * report back to the application.
 */
class TraceEvent {
 public:
  struct MetaData {
   public:
    using MetaDataType =
        boost::variant<int64_t, std::string, std::vector<std::string>>;

    template <typename T,
              typename = typename std::enable_if<std::is_integral<T>::value,
                                                 void>::type>
    /* implicit */ MetaData(T value) : value_(folly::to<int64_t>(value)) {
    }

    /* implicit */ MetaData(const std::string& value) : value_(value) {
    }

    /* implicit */ MetaData(std::string&& value) : value_(std::move(value)) {
    }

    /* implicit */ MetaData(const char* value) : value_(std::string(value)) {
    }

    /* implicit */ MetaData(const folly::fbstring& value)
        : value_(value.toStdString()) {
    }

    /* implicit */ MetaData(const std::vector<std::string>& value)
        : value_(value) {
    }

    /* implicit */ MetaData(std::vector<std::string>&& value)
        : value_(std::move(value)) {
    }

    template <typename T>
    T getValueAs() const {
      ConvVisitor<T> visitor;
      return boost::apply_visitor(visitor, value_);
    }

    const std::type_info& type() const {
      return value_.type();
    }

    template <typename T>
    struct ConvVisitor : boost::static_visitor<T> {
      T operator()(const std::vector<std::string>& /* Unused */) const {
        folly::throw_exception<Exception>("Not supported for type");
      }

      template <typename U>
      T operator()(U& operand) const {
        return folly::to<T>(operand);
      }
    };

    MetaDataType value_;
  };

  using MetaDataMap = std::map<TraceFieldType, MetaData>;

  class Iterator {
   public:
    explicit Iterator(const TraceEvent& event)
        : event_(event), itr_(event.metaData_.begin()) {
    }

    ~Iterator() {
    }

    void next() {
      ++itr_;
    }

    bool isValid() const {
      return itr_ != event_.metaData_.end();
    }

    TraceFieldType getKey() const {
      return itr_->first;
    }

    template <typename T>
    T getValueAs() const {
      return itr_->second.getValueAs<T>();
    }

    const std::type_info& type() const {
      return itr_->second.type();
    }

   private:
    const TraceEvent& event_;
    MetaDataMap::const_iterator itr_;
  };

  FB_EXPORT explicit TraceEvent(TraceEventType type, uint32_t parentID = 0);

  /**
   * Sets the start time to the current time according to the TimeUtil.
   */
  void start(const TimeUtil& tm);

  /**
   * Sets the start time to the given TimePoint.
   */
  void start(TimePoint startTime);

  /**
   * Sets the end time to the current time according to the TimeUtil.
   */
  void end(const TimeUtil& tm);

  /**
   * Sets the end time to the given TimePoint.
   */
  void end(TimePoint endTime);

  /**
   * @Returns true iff start() has been called on this TraceEvent.
   */
  bool hasStarted() const;

  /**
   * @Returns true iff end() has been called on this TraceEvent.
   */
  bool hasEnded() const;

  TimePoint getStartTime() const {
    return start_;
  }

  TimePoint getEndTime() const {
    return end_;
  }

  TraceEventType getType() const {
    return type_;
  }

  uint32_t getID() const {
    return id_;
  }

  void setParentID(uint32_t parent) {
    parentID_ = parent;
  }

  uint32_t getParentID() const {
    return parentID_;
  }

  bool hasTraceField(TraceFieldType field) const {
    return metaData_.count(field);
  }

  template <typename T>
  T getTraceFieldDataAs(TraceFieldType field) const {
    const auto itr = metaData_.find(field);
    CHECK(itr != metaData_.end());
    return itr->second.getValueAs<T>();
  }

  void setMetaData(MetaDataMap&& input) {
    metaData_ = std::move(input);
  }

  const MetaDataMap& getMetaData() const {
    return metaData_;
  }

  Iterator getMetaDataItr() const {
    return Iterator(*this);
  }

  template <typename T>
  bool addMeta(TraceFieldType key, T&& value) {
    MetaData val(std::forward<T>(value));
    return addMetaInternal(key, std::move(val));
  }

  template <typename T>
  bool addMeta(TraceFieldType key, const T& value) {
    MetaData val(value);
    return addMetaInternal(key, std::move(val));
  }

  template <typename T>
  bool readIntMeta(TraceFieldType key, T& dest) const {
    static_assert(std::is_integral<T>::value && !std::is_same<T, bool>::value,
                  "readIntMeta should take an intergral type of paremeter");
    return readMeta(key, dest);
  }

  bool readBoolMeta(TraceFieldType key, bool& dest) const;

  bool readStrMeta(TraceFieldType key, std::string& dest) const;

  std::string toString() const;

  friend std::ostream& operator<<(std::ostream& out, const TraceEvent& event);

  friend class Iterator;

 private:
  template <typename T>
  bool readMeta(TraceFieldType key, T& dest) const {
    const auto itr = metaData_.find(key);
    if (itr != metaData_.end()) {
      return folly::catch_exception(
          [&]() -> bool {
            dest = itr->second.getValueAs<T>();
            return true;
          },
          +[]() -> bool { return false; });
    }
    return false;
  }

  FB_EXPORT bool addMetaInternal(TraceFieldType key, MetaData&& val);

  enum State {
    NOT_STARTED = 0,
    STARTED = 1,
    ENDED = 2,
  };

  uint8_t stateFlags_{0};
  TraceEventType type_;
  uint32_t id_;
  uint32_t parentID_;
  TimePoint start_;
  TimePoint end_;
  MetaDataMap metaData_;
};

template <>
struct TraceEvent::MetaData::ConvVisitor<std::vector<std::string>>
    : boost::static_visitor<std::vector<std::string>> {
  std::vector<std::string> operator()(
      const std::vector<std::string>& operand) const {
    return operand;
  }

  template <typename U>
  std::vector<std::string> operator()(U& /* Unused */) const {
    folly::throw_exception<Exception>("Not supported for type");
  }
};

template <>
struct TraceEvent::MetaData::ConvVisitor<std::string>
    : boost::static_visitor<std::string> {
  std::string operator()(const std::vector<std::string>& operand) const;

  template <typename U>
  std::string operator()(U& operand) const {
    return folly::to<std::string>(operand);
  }
};
} // namespace proxygen
