/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/SysTypes.h>
#include <cstdint>
#include <optional>
#include <string>

#include "eden/common/telemetry/DynamicEvent.h"

namespace watchman {

using DynamicEvent = facebook::eden::DynamicEvent;

struct BaseEvent {
  std::string root;
  std::string error;

  void populate(DynamicEvent& event) const {
    if (!root.empty()) {
      event.addString("root", root);
    }
    if (!error.empty()) {
      event.addString("error", error);
    }
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

} // namespace watchman
