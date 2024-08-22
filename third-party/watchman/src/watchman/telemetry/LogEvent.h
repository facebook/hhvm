/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/SysTypes.h>
#include <folly/stop_watch.h>

#include <atomic>
#include <chrono>
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

struct BaseEventData {
  // TODO: add system and user time for Unix systems
  std::chrono::time_point<std::chrono::system_clock> start_time =
      std::chrono::system_clock::now();
  std::string root;
  std::string error;
  int64_t event_count = 1;

  void populate(DynamicEvent& event) const {
    std::chrono::duration<int64_t, std::nano> elapsed_time =
        std::chrono::system_clock::now() - start_time;
    event.addInt(
        "start_time", std::chrono::system_clock::to_time_t(start_time));
    event.addInt(
        "elapsed_time",
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time)
            .count());
    if (!root.empty()) {
      event.addString("root", root);
    }
    if (!error.empty()) {
      event.addString("error", error);
    }
    event.addInt("event_count", event_count);
  }
};

struct MetadataEventData {
  BaseEventData base;
  int64_t recrawl = 0;
  bool case_sensitive = false;
  std::string watcher;

  void populate(DynamicEvent& event) const {
    base.populate(event);
    event.addInt("recrawl", recrawl);
    event.addBool("case_sensitive", case_sensitive);
    if (!watcher.empty()) {
      event.addString("watcher", watcher);
    }
  }
};

struct DispatchCommand {
  static constexpr const char* type = "dispatch_command";

  MetadataEventData meta;
  std::string command;
  std::string args;
  pid_t client_pid = 0;
  std::string client_name;

  void populate(DynamicEvent& event) const {
    meta.populate(event);
    event.addString("command", command);
    if (!args.empty()) {
      event.addString("args", args);
    }
    if (client_pid != 0) {
      event.addInt("client_pid", client_pid);
    }
    if (!client_name.empty()) {
      event.addString("client", client_name);
    }
  }
};

struct ClockTest {
  static constexpr const char* type = "clock_test";

  BaseEventData base;

  void populate(DynamicEvent& event) const {
    base.populate(event);
  }
};

struct AgeOut {
  static constexpr const char* type = "age_out";

  MetadataEventData meta;
  int64_t walked = 0;
  int64_t files = 0;
  int64_t dirs = 0;

  void populate(DynamicEvent& event) const {
    meta.populate(event);
    event.addInt("walked", walked);
    event.addInt("files", files);
    event.addInt("dirs", dirs);
  }
};

struct SyncToNow {
  static constexpr const char* type = "sync_to_now";

  MetadataEventData meta;
  bool success = true;
  int64_t timeoutms = 0;

  void populate(DynamicEvent& event) const {
    meta.populate(event);
    event.addBool("success", success);
    event.addInt("timeoutms", timeoutms);
  }
};

struct SavedState {
  enum Target { Manifold = 1, Xdb = 2 };
  enum Action { GetProperties = 1, Connect = 2, Query = 3 };

  static constexpr const char* type = "saved_state";

  MetadataEventData meta;
  Target target = Manifold;
  Action action = GetProperties;
  std::string project;
  std::string path;
  int64_t commit_date = 0;
  std::string metadata;
  std::string properties;
  bool success = false;

  void populate(DynamicEvent& event) const {
    meta.populate(event);
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

struct QueryExecute {
  static constexpr const char* type = "query_execute";

  MetadataEventData meta;
  std::string request_id;
  int64_t num_special_files = 0;
  std::string special_files;
  bool fresh_instance = false;
  int64_t deduped = 0;
  int64_t results = 0;
  int64_t walked = 0;
  std::string query;

  void populate(DynamicEvent& event) const {
    meta.populate(event);
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

struct FullCrawl {
  static constexpr const char* type = "full_crawl";

  MetadataEventData meta;

  void populate(DynamicEvent& event) const {
    meta.populate(event);
  }
};

struct Dropped {
  static constexpr const char* type = "dropped";

  MetadataEventData meta;
  bool isKernel = false;

  void populate(DynamicEvent& event) const {
    meta.populate(event);
    event.addBool("isKernel", isKernel);
  }
};

} // namespace watchman
