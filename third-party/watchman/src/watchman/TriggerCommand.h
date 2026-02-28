/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <thread>

#include "watchman/ChildProcess.h"
#include "watchman/PubSub.h"
#include "watchman/saved_state/SavedStateInterface.h"

namespace watchman {

class Event;
class Root;
struct Query;

enum trigger_input_style { input_dev_null, input_json, input_name_list };

struct TriggerCommand {
  w_string triggername;
  std::shared_ptr<Query> query;
  json_ref definition;
  std::optional<json_ref> command;
  ChildProcess::Environment env;

  bool append_files;
  enum trigger_input_style stdin_style;
  uint32_t max_files_stdin;

  int stdout_flags;
  int stderr_flags;
  std::string stdout_name;
  std::string stderr_name;

  /* While we are running, this holds the pid
   * of the running process */
  std::unique_ptr<ChildProcess> current_proc;

  TriggerCommand(
      SavedStateFactory savedStateFactory,
      const std::shared_ptr<Root>& root,
      const json_ref& trig);
  ~TriggerCommand();

  void stop();
  void start(const std::shared_ptr<Root>& root);

 private:
  TriggerCommand(const TriggerCommand&) = delete;
  TriggerCommand(TriggerCommand&&) = delete;

  TriggerCommand& operator=(const TriggerCommand&) = delete;
  TriggerCommand& operator=(TriggerCommand&&) = delete;

  void run(const std::shared_ptr<Root>& root);
  bool maybeSpawn(const std::shared_ptr<Root>& root);
  bool waitNoIntr();

  const SavedStateFactory savedStateFactory_;
  std::thread triggerThread_;
  std::shared_ptr<Publisher::Subscriber> subscriber_;
  std::unique_ptr<Event> ping_;
  bool stopTrigger_{false};
};

} // namespace watchman
