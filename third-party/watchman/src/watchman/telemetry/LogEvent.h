/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/SysTypes.h>
#include <atomic>
#include <cstdint>
#include <optional>
#include <string>

#include "eden/common/telemetry/DynamicEvent.h"

namespace watchman {

using DynamicEvent = facebook::eden::DynamicEvent;

enum LogEventType : uint8_t {
  DispatchCommandType,
  ClockTestType,
  AgeOutType,
  SyncToNowType,
  SavedStateType,
  QueryExecuteType,
  FullCrawlType,
  DroppedType
};

// Returns samplingRate and eventCount
std::pair<int64_t, int64_t> getLogEventCounters(const LogEventType& type);

struct BaseEvent {
  std::string root;
  std::string error;
  int64_t event_count = 1;

  void populate(DynamicEvent& event) const {
    if (!root.empty()) {
      event.addString("root", root);
    }
    if (!error.empty()) {
      event.addString("error", error);
    }
    event.addInt("event_count", event_count);
  }
};

struct MetadataEvent : public BaseEvent {
  int64_t recrawl = 0;
  bool case_sensitive = false;
  std::string watcher;

  void populate(DynamicEvent& event) const {
    BaseEvent::populate(event);

    event.addInt("recrawl", recrawl);
    event.addBool("case_sensitive", case_sensitive);
    if (!watcher.empty()) {
      event.addString("watcher", watcher);
    }
  }
};

struct DispatchCommand : public MetadataEvent {
  static constexpr const char* type = "dispatch_command";

  double duration = 0.0;
  std::string command;
  std::string args;
  pid_t client_pid = 0;

  void populate(DynamicEvent& event) const {
    MetadataEvent::populate(event);

    event.addDouble("duration", duration);
    event.addString("command", command);
    if (!args.empty()) {
      event.addString("args", args);
    }
    if (client_pid != 0) {
      event.addInt("client_pid", client_pid);
    }
  }
};

struct ClockTest : public BaseEvent {
  static constexpr const char* type = "clock_test";

  void populate(DynamicEvent& event) const {
    BaseEvent::populate(event);
  }
};

struct AgeOut : public MetadataEvent {
  static constexpr const char* type = "age_out";

  int64_t walked = 0;
  int64_t files = 0;
  int64_t dirs = 0;

  void populate(DynamicEvent& event) const {
    MetadataEvent::populate(event);

    event.addInt("walked", walked);
    event.addInt("files", files);
    event.addInt("dirs", dirs);
  }
};

struct SyncToNow : public MetadataEvent {
  static constexpr const char* type = "sync_to_now";

  bool success = false;
  int64_t timeoutms = 0;

  void populate(DynamicEvent& event) const {
    MetadataEvent::populate(event);

    event.addBool("success", success);
    event.addInt("timeoutms", timeoutms);
  }
};

struct SavedState : public MetadataEvent {
  enum Target { Manifold = 1, Xdb = 2 };
  enum Action { GetProperties = 1, Connect = 2, Query = 3 };

  static constexpr const char* type = "saved_state";

  Target target = Manifold;
  Action action = GetProperties;
  std::string project;
  std::string path;
  int64_t commit_date = 0;
  std::string metadata;
  std::string properties;
  bool success = false;

  void populate(DynamicEvent& event) const {
    MetadataEvent::populate(event);

    event.addInt("target", target);
    event.addInt("action", action);
    event.addString("project", project);
    if (!path.empty()) {
      event.addString("path", path);
    }
    event.addInt("commit_date", commit_date);
    if (!metadata.empty()) {
      event.addString("metadata", metadata);
    }
    if (!properties.empty()) {
      event.addString("properties", properties);
    }
    event.addBool("success", success);
  }
};

struct QueryExecute : public MetadataEvent {
  static constexpr const char* type = "query_execute";

  std::string request_id;
  int64_t num_special_files = 0;
  std::string special_files;
  bool fresh_instance = false;
  int64_t deduped = 0;
  int64_t results = 0;
  int64_t walked = 0;
  std::string query;

  void populate(DynamicEvent& event) const {
    MetadataEvent::populate(event);

    if (!request_id.empty()) {
      event.addString("request_id", request_id);
    }
    event.addInt("num_special_files", num_special_files);
    if (!special_files.empty()) {
      event.addString("special_files", special_files);
    }
    event.addBool("fresh_instance", fresh_instance);
    event.addInt("deduped", deduped);
    event.addInt("results", results);
    event.addInt("walked", walked);
    if (!query.empty()) {
      event.addString("query", query);
    }
  }
};

struct FullCrawl : public MetadataEvent {
  static constexpr const char* type = "full_crawl";
};

struct Dropped : public MetadataEvent {
  static constexpr const char* type = "dropped";

  bool isKernel = false;

  void populate(DynamicEvent& event) const {
    MetadataEvent::populate(event);

    event.addBool("isKernel", isKernel);
  }
};

} // namespace watchman
