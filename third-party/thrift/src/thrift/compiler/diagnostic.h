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

#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <iosfwd>
#include <iostream>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <thrift/compiler/detail/pluggable_functions.h>
#include <thrift/compiler/metrics/metrics.h>
#include <thrift/compiler/source_location.h>

namespace apache::thrift::compiler {

// A diagnostic level.
enum class diagnostic_level {
  error,
  warning,
  info,
  debug,
};

const char* level_to_string(diagnostic_level level);

class fixit {
 public:
  /**
   * @param original   - original text to be fixed
   * @param replacement - replacement text to replace the original
   * @param loc         - location of the fixit in the source code, if left
   * blank will be populated with the location of the diagnostic
   */
  fixit(std::string original, std::string replacement, source_location loc = {})
      : original_(std::move(original)),
        replacement_(std::move(replacement)),
        loc_(std::move(loc)) {}

  fixit(std::string original, std::string replacement, int line, int column)
      : original_(std::move(original)),
        replacement_(std::move(replacement)),
        line_(line),
        column_(column) {}

  const std::string& original() const { return original_; }
  const std::string& replacement() const { return replacement_; }
  int lineno() const { return line_; }
  int column() const { return column_; }

  // Resolve the location of the fixit in the source code, or use the provided
  // diagonstic location
  void resolve_location(
      source_manager& sm, source_location diagnostic_location);

 private:
  std::string original_;
  std::string replacement_;
  int line_ = 0;
  int column_ = 0;
  source_location loc_;

  // Comparing two fixit hints, not including the source_location since that
  // is an optional param that ultimately just gets resolved into line and
  // column
  friend bool operator==(const fixit& lhs, const fixit& rhs) {
    return std::tie(lhs.original_, lhs.replacement_, lhs.line_, lhs.column_) ==
        std::tie(rhs.original_, rhs.replacement_, rhs.line_, rhs.column_);
  }
};

/**
 * A diagnostic message.
 */
class diagnostic {
 public:
  /**
   * Creates a diagnostic.
   *
   * @param level      - diagnostic level
   * @param message    - detailed diagnostic message
   * @param file       - file path location of diagnostic
   * @param line       - line location of diagnostic in the file, if known
   * @param name       - name given to this diagnostic, if any
   * @param fixit      - an optional fix to apply to the source code
   */
  diagnostic(
      diagnostic_level level,
      std::string message,
      std::string file,
      int line = 0,
      std::string name = "",
      std::optional<fixit> fixit_hint = {})
      : level_(level),
        message_(std::move(message)),
        file_(std::move(file)),
        line_(line),
        name_(std::move(name)),
        fixit_hint_(std::move(fixit_hint)) {}

  diagnostic_level level() const { return level_; }
  const std::string& message() const { return message_; }
  const std::string& file() const { return file_; }
  int lineno() const { return line_; }
  const std::string& name() const { return name_; }
  const std::optional<fixit>& fixit_hint() const { return fixit_hint_; }
  std::string str() const;

  void set_name(std::string&& name) { name_ = std::move(name); }

 private:
  diagnostic_level level_;
  std::string message_;
  std::string file_;
  int line_;
  std::string name_;
  std::optional<fixit> fixit_hint_;

  friend bool operator==(const diagnostic& lhs, const diagnostic& rhs) {
    return std::tie(
               lhs.level_,
               lhs.line_,
               lhs.message_,
               lhs.file_,
               lhs.name_,
               lhs.fixit_hint_) ==
        std::tie(
               rhs.level_,
               rhs.line_,
               rhs.message_,
               rhs.file_,
               rhs.name_,
               rhs.fixit_hint_);
  }
  friend bool operator!=(const diagnostic& lhs, const diagnostic& rhs) {
    return !(lhs == rhs);
  }
};

// A container of diagnostic results.
class diagnostic_results {
 public:
  explicit diagnostic_results(std::vector<diagnostic> initial_diagnostics);
  diagnostic_results() = default;
  diagnostic_results(const diagnostic_results&) = default;
  diagnostic_results(diagnostic_results&&) noexcept = default;

  diagnostic_results& operator=(diagnostic_results&&) noexcept = default;
  diagnostic_results& operator=(const diagnostic_results&) = default;

  const std::vector<diagnostic>& diagnostics() const& { return diagnostics_; }
  std::vector<diagnostic>&& diagnostics() && { return std::move(diagnostics_); }

  void add(diagnostic diag);

  bool has_error() const { return count(diagnostic_level::error) != 0; }
  std::size_t count(diagnostic_level level) const {
    return counts_.at(static_cast<size_t>(level));
  }

  template <typename F>
  void retain_if(F&& f) {
    diagnostics_.erase(
        std::remove_if(
            diagnostics_.begin(),
            diagnostics_.end(),
            [&](const diagnostic& diag) {
              if (!f(diag)) {
                decrement(diag.level());
                return true;
              }
              return false;
            }),
        diagnostics_.end());
  }

 private:
  std::vector<diagnostic> diagnostics_;
  std::array<int, static_cast<size_t>(diagnostic_level::debug) + 1> counts_{};

  void increment(diagnostic_level level) {
    ++counts_.at(static_cast<size_t>(level));
  }

  void decrement(diagnostic_level level) {
    --counts_.at(static_cast<size_t>(level));
  }
};

struct diagnostic_params {
  bool debug = false;
  bool info = false;
  int warn_level = 1;
  bool log_metrics = true;

  bool should_report(diagnostic_level level) const {
    switch (level) {
      case diagnostic_level::warning:
        return warn_level > 0;
      case diagnostic_level::debug:
        return debug;
      case diagnostic_level::info:
        return info;
      default:
        return true;
    }
  }

  // Params that only collect errors.
  static diagnostic_params only_errors() { return {false, false, 0}; }
  static diagnostic_params strict() { return {false, false, 2}; }
  static diagnostic_params keep_all() { return {true, true, 2}; }
};

// A source location used in diagnostic reporting functions to support AST nodes
// without adding a dependency on AST.
struct diagnostic_location {
  source_location loc;

  diagnostic_location(source_location l) : loc(l) {}

  template <typename T>
  diagnostic_location(const T& locatable) : loc(locatable.src_range().begin) {}
};

// A class used by the Thrift compiler to report diagnostics.
class diagnostics_engine {
 public:
  explicit diagnostics_engine(
      source_manager& sm,
      std::function<void(diagnostic)> report_cb,
      diagnostic_params params = {})
      : source_mgr_(&sm),
        report_cb_(std::move(report_cb)),
        params_(std::move(params)) {}
  explicit diagnostics_engine(
      source_manager& sm,
      diagnostic_results& results,
      diagnostic_params params = {})
      : diagnostics_engine(
            sm,
            [&results](diagnostic diag) { results.add(std::move(diag)); },
            std::move(params)) {}

  static diagnostics_engine ignore_all(source_manager& sm) {
    return diagnostics_engine(
        sm, [](const diagnostic&) {}, diagnostic_params::only_errors());
  }

  diagnostic_params& params() { return params_; }
  const diagnostic_params& params() const { return params_; }

  source_manager& source_mgr() { return *source_mgr_; }
  const source_manager& source_mgr() const { return *source_mgr_; }

  bool has_errors() const { return has_errors_; }

  void report(diagnostic diag) {
    if (diag.level() == diagnostic_level::error) {
      has_errors_ = true;
    }
    if (params_.should_report(diag.level())) {
      report_cb_(std::move(diag));
    }
  }

  template <typename... T>
  void report(
      diagnostic_location loc,
      diagnostic_level level,
      fmt::format_string<T...> msg,
      T&&... args) {
    do_report(
        loc.loc,
        {}, /* name */
        {}, /* fixit_hint */
        level,
        fmt::format(msg, std::forward<T>(args)...));
  }

  template <typename... T>
  void report(
      diagnostic_location loc,
      std::string name,
      std::optional<fixit> fixit_hint,
      diagnostic_level level,
      fmt::format_string<T...> msg,
      T&&... args) {
    do_report(
        loc.loc,
        std::move(name),
        std::move(fixit_hint),
        level,
        fmt::format(msg, std::forward<T>(args)...));
  }

  template <typename... T>
  void report(
      diagnostic_location loc,
      std::string name,
      diagnostic_level level,
      fmt::format_string<T...> msg,
      T&&... args) {
    do_report(
        loc.loc,
        std::move(name),
        {}, /* fixit_hint */
        level,
        fmt::format(msg, std::forward<T>(args)...));
  }

  template <typename... T>
  void warning(
      diagnostic_location loc, fmt::format_string<T...> msg, T&&... args) {
    report(loc.loc, diagnostic_level::warning, msg, std::forward<T>(args)...);
  }

  template <typename... T>
  void warning_legacy_strict(
      diagnostic_location loc, fmt::format_string<T...> msg, T&&... args) {
    if (params().warn_level >= 2) {
      warning(loc.loc, msg, std::forward<T>(args)...);
    }
  }

  template <typename... T>
  void error(
      diagnostic_location loc, fmt::format_string<T...> msg, T&&... args) {
    report(loc.loc, diagnostic_level::error, msg, std::forward<T>(args)...);
  }

  // Reports an error and returns false, if the provided condition is false.
  template <typename... T>
  bool check(
      bool condition,
      diagnostic_location loc,
      fmt::format_string<T...> msg,
      T&&... args) {
    if (!condition) {
      error(loc, msg, std::forward<T>(args)...);
    }
    return condition;
  }

  detail::metrics& metrics() { return metrics_; }

 private:
  void do_report(
      source_location loc,
      std::string name,
      std::optional<fixit> fixit_hint,
      diagnostic_level level,
      std::string msg);

  source_manager* source_mgr_;
  std::function<void(diagnostic)> report_cb_;
  diagnostic_params params_;
  bool has_errors_ = false;
  detail::metrics metrics_;
};

// Makes a diagnostics engine that prints errors to stderr and ignores other
// kinds of diagnostics.
diagnostics_engine make_diagnostics_printer(source_manager& sm);

std::ostream& operator<<(std::ostream& out, const diagnostic&);

struct log_metrics_tag {
  static void default_impl(
      detail::metrics& metrics, const diagnostic_params& d_params) {
    if (!d_params.should_report(diagnostic_level::debug)) {
      return;
    }
    for (int metric = 0;
         metric <= static_cast<int>(detail::metrics::StringValue::LAST);
         ++metric) {
      auto stringValue = static_cast<detail::metrics::StringValue>(metric);
      fmt::print(
          stdout,
          "{}={}\n",
          detail::metrics::to_string(stringValue),
          metrics.get(stringValue).get());
    }
    for (int metric = 0;
         metric <= static_cast<int>(detail::metrics::IntValue::LAST);
         ++metric) {
      auto intValue = static_cast<detail::metrics::IntValue>(metric);
      fmt::print(
          stdout,
          "{}={}\n",
          detail::metrics::to_string(intValue),
          metrics.get(intValue).get());
    }
    for (int metric = 0;
         metric <= static_cast<int>(detail::metrics::EventString::LAST);
         ++metric) {
      auto event = static_cast<detail::metrics::EventString>(metric);
      fmt::print(
          stdout,
          "{}={}\n",
          detail::metrics::to_string(event),
          fmt::join(metrics.get(event).get(), ","));
    }
  }
};

} // namespace apache::thrift::compiler

template <>
struct fmt::formatter<apache::thrift::compiler::diagnostic> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
  format_context::iterator format(
      const apache::thrift::compiler::diagnostic& d, format_context& ctx) const;
};
