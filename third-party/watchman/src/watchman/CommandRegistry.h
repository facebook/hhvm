/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdexcept>
#include <vector>

// TODO: We could avoid the Client dependency fi CommandHandler returned a
// json_ref response or error rather than taking a Client.
#include "eden/common/utils/OptionSet.h"
#include "watchman/Serde.h"
#include "watchman/thirdparty/jansson/jansson.h"
#include "watchman/watchman_preprocessor.h"

namespace watchman {

class Client;
class Command;

/**
 * Thrown by command handlers to indicate a potentially expected error, where
 * the given message is used directly in the Watchman error response.
 */
class ErrorResponse : public std::runtime_error {
 public:
  ErrorResponse(const char* message);

  template <typename First, typename... Rest>
  ErrorResponse(
      fmt::format_string<First, Rest...> fmt,
      First&& first,
      Rest&&... rest)
      : ErrorResponse(fmt::format(
                          std::move(fmt),
                          std::forward<First>(first),
                          std::forward<Rest>(rest)...)
                          .c_str()) {}
};

/**
 * Thrown by command handlers that manually enqueue a response.
 */
class ResponseWasHandledManually : public std::exception {};

/**
 * Represents an old-style, untyped JSON command response.
 *
 * Since JSON ASTs are (or will be) immutable, the response objects are built in
 * an UntypedResponse before being converted to a JSON object as late as
 * possible.
 */
class UntypedResponse : public std::unordered_map<w_string, json_ref> {
 public:
  /**
   * Automatically sets the "version" field.
   */
  UntypedResponse();

  /**
   * Construct from a previously-composed JSON unordered_map.
   */
  explicit UntypedResponse(std::unordered_map<w_string, json_ref> map);

  /**
   * Consumes the UntypedResponse and converts it into a JSON object.
   */
  json_ref toJson() &&;

  void set(const char* key, json_ref value);

  void set(std::initializer_list<std::pair<const char*, json_ref>> values);
};

/**
 * Validates a command's arguments. Runs on the client. May modify the given
 * command. Should throw an exception (ideally CommandValidationError) if
 * validation fails.
 */
using CommandValidator = void (*)(Command& command);

/**
 * Executes a command's primary action. Usually runs on the server, but there
 * are client-only commands.
 *
 * Returns a success response, or throws ErrorResponse.
 */
using CommandHandler =
    UntypedResponse (*)(Client* client, const json_ref& args);

/**
 * For commands that support pretty, human-readable output, this function is
 * called, on the client, with a result PDU. It should print its output to
 * stdout.
 *
 * Only called when the output is a tty.
 */
using ResultPrinter = void (*)(const json_ref& result);

struct CommandFlags : facebook::eden::OptionSet<CommandFlags, uint8_t> {};

inline constexpr auto CMD_DAEMON = CommandFlags::raw(1);
inline constexpr auto CMD_CLIENT = CommandFlags::raw(2);
inline constexpr auto CMD_POISON_IMMUNE = CommandFlags::raw(4);
inline constexpr auto CMD_ALLOW_ANY_USER = CommandFlags::raw(8);

struct CommandDefinition {
  const std::string_view name;
  const CommandFlags flags;
  const CommandValidator validator;
  const CommandHandler handler;
  const ResultPrinter result_printer;

  CommandDefinition(
      std::string_view name,
      std::string_view capname,
      CommandHandler handler,
      CommandFlags flags,
      CommandValidator validator,
      ResultPrinter result_printer);

  /**
   * Provide a way to query (and eventually modify) command line arguments
   *
   * This is not thread-safe and should only be invoked from main()
   */
  static const CommandDefinition* lookup(std::string_view name);

  static std::vector<const CommandDefinition*> getAll();

 private:
  // registration linkage
  CommandDefinition* next_ = nullptr;
};

/**
 * Response types should extend BaseResponse by default, since every Watchman
 * response includes a version string.
 */
struct BaseResponse : serde::Object {
  w_string version;

  BaseResponse();

  template <typename X>
  void map(X& x) {
    x.required("version", version);
  }
};

/**
 * For commands that have no request parameters, write:
 * `using Request = NullRequest`
 */
using NullRequest = serde::Nothing;

/**
 * Provides a typed interface for CommandDefinition that can optionally handle
 * validation, result-printing, request decoding, and response encoding.
 */
template <typename T>
class TypedCommand : public CommandDefinition {
 public:
  /// Override to implement a validator.
  static constexpr CommandValidator validate = nullptr;

  explicit TypedCommand(ResultPrinter resultPrinter = nullptr)
      : CommandDefinition{
            T::name,
            // TODO: eliminate this allocation
            std::string{"cmd-"} + std::string{T::name},
            T::handleRaw,
            T::flags,
            T::validate,
            resultPrinter} {}

  static UntypedResponse handleRaw(Client* client, const json_ref& args) {
    // In advance of having individual handlers take a Command struct directly,
    // let's shift off the first entry `args`, since we know it's the command
    // name.
    auto& arr = args.array();
    std::vector<json_ref> adjusted_args;
    for (size_t i = 1; i < arr.size(); ++i) {
      adjusted_args.push_back(arr[i]);
    }

    using Request = typename T::Request;
    auto encodedResponse = serde::encode(T::handle(
        client, serde::decode<Request>(json_array(std::move(adjusted_args)))));
    return UntypedResponse{encodedResponse.object()};
  }
};

/**
 * A subclass of TypedCommand that also provides can print a human-readable
 * representation of the response.
 *
 * TODO: It might be possible to morge this into TypedCommand by using SFINAE to
 * detect the existence of a printResult method.
 */
template <typename T>
class PrettyCommand : public TypedCommand<T> {
 public:
  PrettyCommand() : TypedCommand<T>{&printResultRaw} {}

  static void printResultRaw(const json_ref& result) {
    using Response = typename T::Response;
    return T::printResult(serde::decode<Response>(result));
  }
};

static_assert(
    std::is_trivially_destructible_v<CommandDefinition>,
    "CommandDefinition should remain unchanged until process exit");

void capability_register(std::string_view name);
bool capability_supported(std::string_view name);
json_ref capability_get_list();

#define W_CMD_REG(name, func, flags, clivalidate)                       \
  static const ::watchman::CommandDefinition w_gen_symbol(w_cmd_def_) { \
    (name), "cmd-" name, (func), (flags), (clivalidate), nullptr        \
  }

#define WATCHMAN_COMMAND(name, class_) static class_ reg_##name

#define W_CAP_REG1(symbol, name)           \
  static w_ctor_fn_type(symbol) {          \
    ::watchman::capability_register(name); \
  }                                        \
  w_ctor_fn_reg(symbol)

#define W_CAP_REG(name) W_CAP_REG1(w_gen_symbol(w_cap_reg_), name)

} // namespace watchman

/* vim:ts=2:sw=2:et:
 */
