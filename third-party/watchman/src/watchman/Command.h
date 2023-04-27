/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include "watchman/PDU.h"
#include "watchman/Result.h"
#include "watchman/thirdparty/jansson/jansson.h"

namespace watchman {

struct CommandDefinition;
class Stream;

enum class Pretty {
  Yes,
  IfTty,
  No,
};

class Command {
 public:
  /**
   * Constructs a null command used only to start the Watchman server.
   */
  /* implicit */ Command(std::nullptr_t) : args_{json_null()} {}

  Command(w_string name, json_ref args);

  /**
   * Parses a command from arbitrary JSON.
   *
   * Throws CommandValidationError if the JSON is invalid.
   */
  static Command parse(const json_ref& pdu);

  /**
   * Renders into a JSON (or BSER) PDU.
   */
  json_ref render() const;

  /**
   * The null command is used solely to start the server, and never actually
   * executed.
   */
  bool isNullCommand() const {
    return name_ == nullptr;
  }

  std::string_view name() const {
    return name_.view();
  }

  json_ref& args() {
    return args_;
  }

  const json_ref& args() const {
    return args_;
  }

  /**
   * Returns an unowned reference to the command definition for `name_`. May be
   * null if there is no corresponding command.
   */
  const CommandDefinition* getCommandDefinition() const {
    return commandDefinition_;
  }

  /**
   * Perform some client-side validation of this Command and its arguments. If
   * validation fails, print an error PDU to stdout in the format specified by
   * `output_pdu` and `output_capabilities` and exit(1).
   */
  void validateOrExit(PduFormat error_format);

  /**
   * Called by the client. Sends a command to the daemon and prints the output
   * response to stdout.
   *
   * If persistent is true, this function continuously loops until there is an
   * error reading from the connection stream.
   */
  ResultErrno<folly::Unit> run(
      Stream& stream,
      bool persistent,
      PduFormat server_format,
      PduFormat output_format,
      Pretty pretty) const;

 private:
  /**
   * Read a PDU from `stream`, blocking if necessary, and encode it into
   * stdout through `output_pdu_buf`.
   */
  ResultErrno<folly::Unit> passPduToStdout(
      Stream& stream,
      PduBuffer& input_buffer,
      PduFormat output_format,
      PduBuffer& output_pdu_buf,
      Pretty pretty) const;

  w_string name_;
  json_ref args_;

  const CommandDefinition* commandDefinition_ = nullptr;
};

} // namespace watchman
