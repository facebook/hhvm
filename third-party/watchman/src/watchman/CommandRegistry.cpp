/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/CommandRegistry.h"
#include <unordered_map>
#include <unordered_set>
#include "watchman/Errors.h"
#include "watchman/thirdparty/jansson/jansson.h"

namespace watchman {

namespace {

CommandDefinition* commandsList = nullptr;

struct CommandRegistry {
  std::unordered_set<std::string> capabilities;

  CommandRegistry() {
    capabilities.reserve(128);
  }

  static CommandRegistry& get() {
    // Meyers singleton to avoid SIOF problems
    static auto* s = new CommandRegistry;
    return *s;
  }
};

} // namespace

ErrorResponse::ErrorResponse(const char* message)
    : std::runtime_error{message} {}

UntypedResponse::UntypedResponse() {
  set("version", typed_string_to_json(PACKAGE_VERSION, W_STRING_UNICODE));
}

UntypedResponse::UntypedResponse(std::unordered_map<w_string, json_ref> map)
    : std::unordered_map<w_string, json_ref>{std::move(map)} {}

json_ref UntypedResponse::toJson() && {
  return json_object(std::move(*this));
}

void UntypedResponse::set(const char* key, json_ref value) {
  insert_or_assign(w_string{key, W_STRING_UNICODE}, std::move(value));
}

void UntypedResponse::set(
    std::initializer_list<std::pair<const char*, json_ref>> values) {
  for (auto& [key, value] : values) {
    set(key, value);
  }
}

CommandDefinition::CommandDefinition(
    std::string_view name,
    std::string_view capname,
    CommandHandler handler,
    CommandFlags flags,
    CommandValidator validator,
    ResultPrinter result_printer)
    : name{name},
      flags{flags},
      validator{validator},
      handler{handler},
      result_printer{result_printer} {
  next_ = commandsList;
  commandsList = this;

  capability_register(capname);
}

const CommandDefinition* CommandDefinition::lookup(std::string_view name) {
  // You can imagine optimizing this into a sublinear lookup but the command
  // list is small and constant.
  for (const auto* def = commandsList; def; def = def->next_) {
    if (name == def->name) {
      return def;
    }
  }
  return nullptr;
}

std::vector<const CommandDefinition*> CommandDefinition::getAll() {
  size_t n = 0;
  for (const auto* p = commandsList; p; p = p->next_) {
    ++n;
  }

  std::vector<const CommandDefinition*> defs;
  defs.reserve(n);
  for (auto* p = commandsList; p; p = p->next_) {
    defs.push_back(p);
  }
  return defs;
}

BaseResponse::BaseResponse() : version{PACKAGE_VERSION, W_STRING_UNICODE} {}

void capability_register(std::string_view name) {
  CommandRegistry::get().capabilities.emplace(std::string{name});
}

bool capability_supported(std::string_view name) {
  auto& reg = CommandRegistry::get();
  // TODO: Eliminate this copy.
  return reg.capabilities.find(std::string{name}) != reg.capabilities.end();
}

json_ref capability_get_list() {
  auto& caps = CommandRegistry::get().capabilities;

  std::vector<json_ref> arr;
  arr.reserve(caps.size());
  for (auto& name : caps) {
    arr.push_back(typed_string_to_json(name.c_str()));
  }

  return json_array(std::move(arr));
}

} // namespace watchman
