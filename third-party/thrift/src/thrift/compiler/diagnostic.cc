/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

#include <thrift/compiler/diagnostic.h>

#include <ostream>
#include <string>

using apache::thrift::compiler::diagnostic;
using apache::thrift::compiler::diagnostic_level;

namespace {
const char* level_to_string(diagnostic_level level) {
  switch (level) {
    case diagnostic_level::error:
      return "ERROR";
    case diagnostic_level::warning:
      return "WARNING";
    case diagnostic_level::info:
      return "INFO";
    case diagnostic_level::debug:
      return "DEBUG";
  }
  return "??";
}
} // namespace

fmt::format_context::iterator fmt::formatter<diagnostic>::format(
    const diagnostic& d, format_context& ctx) const {
  auto out = ctx.out();
  fmt::format_to(out, "[{}:{}", level_to_string(d.level()), d.file());
  if (d.lineno() > 0) {
    fmt::format_to(out, ":{}", d.lineno());
  }
  fmt::format_to(out, "] {}", d.message());
  if (!d.name().empty()) {
    fmt::format_to(out, " [{}]", d.name());
  }
  if (d.fixit_hint()) {
    fmt::format_to(
        out,
        R"( fix: [original: "{}"][replacement: "{}"])",
        d.fixit_hint()->original(),
        d.fixit_hint()->replacement());
  }
  return out;
}

namespace apache::thrift::compiler {

std::string diagnostic::str() const {
  return fmt::format("{}", *this);
}

void diagnostic_results::add(diagnostic diag) {
  increment(diag.level());
  diagnostics_.emplace_back(std::move(diag));
}

diagnostic_results::diagnostic_results(
    std::vector<diagnostic> initial_diagnostics)
    : diagnostics_(std::move(initial_diagnostics)) {
  for (const auto& diag : diagnostics_) {
    increment(diag.level());
  }
}

void diagnostics_engine::do_report(
    source_location loc,
    std::string name,
    std::optional<fixit> fixit_hint,
    diagnostic_level level,
    std::string msg) {
  if (level == diagnostic_level::error) {
    has_errors_ = true;
  }
  if (!params_.should_report(level)) {
    return;
  }
  std::string file_name;
  unsigned line = 0;
  if (loc != source_location()) {
    auto resolved_loc = resolved_location(loc, *source_mgr_);
    file_name = resolved_loc.file_name();
    line = resolved_loc.line();
  }
  if (fixit_hint) {
    fixit_hint->resolve_location(*source_mgr_, loc);
  }
  report_cb_(
      {level,
       std::move(msg),
       std::move(file_name),
       static_cast<int>(line),
       std::move(name),
       std::move(fixit_hint)});
}

std::ostream& operator<<(std::ostream& out, const diagnostic& diag) {
  return out << diag.str();
}

} // namespace apache::thrift::compiler
