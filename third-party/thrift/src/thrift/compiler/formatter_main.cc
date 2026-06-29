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

#include <thrift/compiler/generate/formatter.h>

#include <fmt/core.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr std::string_view kDefaultSeverity = "warning";
constexpr std::string_view kDescription =
    "File is not formatted with thrift-fmt";

struct options {
  bool in_place = false;
  std::string_view severity = kDefaultSeverity;
  std::vector<std::filesystem::path> paths;
};

struct diagnostic {
  std::optional<int> line;
  std::optional<int> character;
  std::string description;
};

void print_usage(FILE* stream, std::string_view program) {
  fmt::print(
      stream,
      "usage: {} [-h] [--severity {{error,warning,advice,disabled}}] [-i] "
      "filenames [filenames ...]\n"
      "\n"
      "positional arguments:\n"
      "  filenames             paths to lint\n"
      "\n"
      "optional arguments:\n"
      "  -h, --help            show this help message and exit\n"
      "  --severity {{error,warning,advice,disabled}}\n"
      "                        severity for format lint messages\n"
      "  -i                    replace file contents in-place\n",
      program);
}

bool is_severity(std::string_view value) {
  return value == "error" || value == "warning" || value == "advice" ||
      value == "disabled";
}

std::optional<options> parse_args(
    int argc, char** argv, std::string_view program, int& exit_code) {
  options opts;
  for (int i = 1; i < argc; ++i) {
    if (argv[i] == nullptr) {
      fmt::print(stderr, "{}: error: missing argument\n", program);
      exit_code = 2;
      return std::nullopt;
    }
    const std::string_view arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      print_usage(stdout, program);
      exit_code = 0;
      return std::nullopt;
    }
    if (arg == "-i") {
      opts.in_place = true;
      continue;
    }
    if (arg == "--") {
      while (++i < argc) {
        if (argv[i] == nullptr) {
          fmt::print(stderr, "{}: error: missing argument\n", program);
          exit_code = 2;
          return std::nullopt;
        }
        opts.paths.emplace_back(argv[i]);
      }
      break;
    }
    if (arg == "--severity") {
      if (i + 1 >= argc || argv[i + 1] == nullptr) {
        fmt::print(
            stderr,
            "{}: error: argument --severity: expected one argument\n",
            program);
        exit_code = 2;
        return std::nullopt;
      }
      const std::string_view severity = argv[++i];
      if (!is_severity(severity)) {
        fmt::print(
            stderr,
            "{}: error: argument --severity: invalid choice: '{}' (choose from "
            "'error', 'warning', 'advice', 'disabled')\n",
            program,
            severity);
        exit_code = 2;
        return std::nullopt;
      }
      opts.severity = severity;
      continue;
    }
    if (arg.rfind("--severity=", 0) == 0) {
      const std::string_view severity =
          arg.substr(std::string_view("--severity=").size());
      if (!is_severity(severity)) {
        fmt::print(
            stderr,
            "{}: error: argument --severity: invalid choice: '{}' (choose from "
            "'error', 'warning', 'advice', 'disabled')\n",
            program,
            severity);
        exit_code = 2;
        return std::nullopt;
      }
      opts.severity = severity;
      continue;
    }
    if (!arg.empty() && arg.front() == '-') {
      fmt::print(
          stderr, "{}: error: unrecognized arguments: {}\n", program, arg);
      exit_code = 2;
      return std::nullopt;
    }
    opts.paths.emplace_back(arg);
  }
  if (opts.paths.empty()) {
    fmt::print(
        stderr,
        "{}: error: the following arguments are required: filenames\n",
        program);
    exit_code = 2;
    return std::nullopt;
  }
  return opts;
}

std::optional<std::string> read_file(
    const std::filesystem::path& path, std::string& error) {
  std::ifstream input(path, std::ios::binary);
  if (!input) {
    error = fmt::format("failed to open `{}` for reading", path.string());
    return std::nullopt;
  }
  std::string text(
      (std::istreambuf_iterator<char>(input)),
      std::istreambuf_iterator<char>());
  if (!input.eof() && input.fail()) {
    error = fmt::format("failed to read `{}`", path.string());
    return std::nullopt;
  }
  return text;
}

bool write_file(
    const std::filesystem::path& path,
    std::string_view text,
    std::string& error) {
  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output) {
    error = fmt::format("failed to open `{}` for writing", path.string());
    return false;
  }
  output << text;
  if (!output) {
    error = fmt::format("failed to write `{}`", path.string());
    return false;
  }
  return true;
}

std::string json_string(std::string_view value) {
  std::string result;
  result.reserve(value.size() + 2);
  result.push_back('"');
  for (const unsigned char c : value) {
    switch (c) {
      case '"':
        result += "\\\"";
        break;
      case '\\':
        result += "\\\\";
        break;
      case '\b':
        result += "\\b";
        break;
      case '\f':
        result += "\\f";
        break;
      case '\n':
        result += "\\n";
        break;
      case '\r':
        result += "\\r";
        break;
      case '\t':
        result += "\\t";
        break;
      default:
        if (c < 0x20) {
          result += fmt::format("\\u{:04x}", c);
        } else {
          result.push_back(static_cast<char>(c));
        }
        break;
    }
  }
  result.push_back('"');
  return result;
}

std::string json_int_or_null(std::optional<int> value) {
  return value ? std::to_string(*value) : "null";
}

diagnostic parse_format_error(std::string_view message) {
  diagnostic result;
  result.description = std::string(message);
  const size_t close = message.find(']');
  if (message.rfind("[ERROR:", 0) != 0 || close == std::string_view::npos) {
    return result;
  }
  const size_t line_start = message.rfind(':', close);
  if (line_start == std::string_view::npos || line_start + 1 >= close) {
    return result;
  }
  int line = 0;
  for (size_t i = line_start + 1; i < close; ++i) {
    const char c = message.at(i);
    if (c < '0' || c > '9') {
      return result;
    }
    line = line * 10 + (c - '0');
  }
  result.line = line;
  if (close + 2 <= message.size()) {
    result.description = std::string(message.substr(close + 2));
  }
  return result;
}

void print_lint_error(
    const std::filesystem::path& path,
    std::string_view severity,
    std::string_view name,
    const diagnostic& diag) {
  fmt::print(
      stdout,
      "{{\"path\":{},\"line\":{},\"char\":{},\"code\":\"THRIFTFORMAT\","
      "\"severity\":{},\"name\":{},\"description\":{}}}\n",
      json_string(path.string()),
      json_int_or_null(diag.line),
      json_int_or_null(diag.character),
      json_string(severity),
      json_string(name),
      json_string(diag.description));
}

void print_format_lint(
    const std::filesystem::path& path,
    std::string_view severity,
    std::string_view original,
    std::string_view replacement) {
  fmt::print(
      stdout,
      "{{\"path\":{},\"line\":1,\"char\":1,\"code\":\"THRIFTFORMAT\","
      "\"severity\":{},\"name\":\"format\",\"description\":{},\"original\":{},"
      "\"replacement\":{}}}\n",
      json_string(path.string()),
      json_string(severity),
      json_string(kDescription),
      json_string(original),
      json_string(replacement));
}

void print_in_place_error(
    const std::filesystem::path& path, const diagnostic& diag) {
  fmt::print(stderr, "ERROR[{}", path.string());
  if (diag.line) {
    fmt::print(stderr, ":{}", *diag.line);
  }
  fmt::print(stderr, "]\n{}\n", diag.description);
}

bool lint_or_format_path(
    const std::filesystem::path& path, const options& opts) {
  std::string error;
  std::optional<std::string> source = read_file(path, error);
  if (!source) {
    diagnostic diag{std::nullopt, std::nullopt, std::move(error)};
    if (opts.in_place) {
      print_in_place_error(path, diag);
      return false;
    } else {
      print_lint_error(path, opts.severity, "file", diag);
    }
    return true;
  }

  std::string formatted;
  try {
    formatted = apache::thrift::compiler::format_thrift_source(*source);
  } catch (const std::exception& ex) {
    diagnostic diag = parse_format_error(ex.what());
    if (opts.in_place) {
      print_in_place_error(path, diag);
      return false;
    } else {
      print_lint_error(path, opts.severity, "parser", diag);
    }
    return true;
  }

  if (formatted == *source) {
    return true;
  }
  if (!opts.in_place) {
    print_format_lint(path, opts.severity, *source, formatted);
    return true;
  }
  if (!write_file(path, formatted, error)) {
    print_in_place_error(path, {std::nullopt, std::nullopt, std::move(error)});
    return false;
  }
  return true;
}

} // namespace

int main(int argc, char** argv) {
  if (argv == nullptr) {
    print_usage(stderr, "thrift-fmt");
    return 1;
  }
  const std::string program = argc > 0 && argv[0] != nullptr
      ? std::filesystem::path(argv[0]).filename().string()
      : "thrift-fmt";
  int exit_code = 0;
  std::optional<options> opts = parse_args(argc, argv, program, exit_code);
  if (!opts) {
    return exit_code;
  }

  bool success = true;
  for (const auto& path : opts->paths) {
    if (!lint_or_format_path(path, *opts)) {
      success = false;
    }
  }
  return success ? 0 : 1;
}
