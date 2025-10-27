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

#include <thrift/conformance/Utils.h>

#include <chrono>
#include <cstdlib>

#include <folly/FileUtil.h>
#include <folly/String.h>
#include <folly/Subprocess.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace apache::thrift::conformance {

const char* getEnvOr(const char* name, const char* dflt) {
  if (const char* value = std::getenv(name)) {
    return value;
  }
  return dflt;
}

const char* getEnvOrThrow(const char* name) {
  if (const char* value = std::getenv(name)) {
    return value;
  }
  folly::throw_exception<std::runtime_error>(
      fmt::format("{} environment variable is required.", name));
}

std::string readFromFile(const char* filename) {
  if (filename == nullptr) {
    return {};
  }
  std::string data;
  if (!folly::readFile(filename, data)) {
    folly::throw_exception<std::invalid_argument>(
        fmt::format("Could not read file: {}", filename));
  }
  return data;
}

std::string readFromCmd(const std::vector<std::string>& argv) {
  folly::Subprocess proc(argv, folly::Subprocess::Options().pipeStdout());
  std::string result = proc.communicate().first;
  // NOTE: proc.waitOrTermiateOrKill() logs, so we can't use it before main().
  proc.sendSignal(SIGINT);
  if (proc.waitTimeout(std::chrono::seconds(1)).running()) {
    proc.terminate();
    if (proc.waitTimeout(std::chrono::seconds(1)).running()) {
      proc.kill();
      proc.wait();
    }
  }

  return result;
}

std::pair<std::string_view, std::string_view> parseNameAndCmd(
    std::string_view entry) {
  // Look for a custom name.
  auto pos = entry.find_last_of("#/");
  if (pos != std::string_view::npos && entry[pos] == '#') {
    if (pos == entry.size() - 1) {
      // Just a trailing delim, remove it.
      entry = entry.substr(0, pos);
    } else {
      // Use the custom name.
      return {entry.substr(pos + 1), entry.substr(0, pos)};
    }
  }

  // No custom name, so use parent directory as name.
  size_t stop = entry.find_last_of("\\/") - 1;
  size_t start = entry.find_last_of("\\/", stop);
  return {entry.substr(start + 1, stop - start), entry};
}

std::map<std::string_view, std::string_view> parseCmds(
    std::string_view cmdsStr) {
  std::map<std::string_view, std::string_view> result;
  if (cmdsStr.empty()) {
    return result;
  }
  std::vector<folly::StringPiece> cmds;
  folly::split(',', cmdsStr, cmds);
  for (auto cmd : cmds) {
    auto entry = parseNameAndCmd(folly::trimWhitespace(cmd));
    auto res = result.emplace(entry);
    if (!res.second) {
      folly::throw_exception<std::invalid_argument>(fmt::format(
          "Multiple servers have the name {}: {} vs {}",
          entry.first,
          res.first->second,
          entry.second));
    }
  }
  return result;
}

std::set<std::string> parseNonconforming(std::string_view data) {
  std::vector<folly::StringPiece> lines;
  folly::split('\n', data, lines);
  std::set<std::string> result;
  for (auto& line : lines) {
    // Strip any comments.
    if (auto pos = line.find_first_of('#'); pos != folly::StringPiece::npos) {
      line = line.subpiece(0, pos);
    }
    // Add trimmed, non-empty lines.
    line = folly::trimWhitespace(line);
    if (!line.empty()) {
      result.emplace(line);
    }
  }
  return result;
}

std::vector<TestSuite> getSuites() {
  std::vector<TestSuite> result;

  std::vector<std::string> suiteGens;
  folly::split(
      ',', getEnvOr("THRIFT_CONFORMANCE_TEST_SUITE_GENS", ""), suiteGens);
  for (auto cmd : suiteGens) {
    if (!cmd.empty()) {
      result.emplace_back(
          BinarySerializer::deserialize<TestSuite>(
              readFromCmd({std::move(cmd)})));
    }
  }
  if (result.empty()) {
    folly::throw_exception<std::invalid_argument>(fmt::format(
        "No test suites found:\n"
        "  THRIFT_CONFORMANCE_TEST_SUITE_GENS={}",
        getEnvOr("THRIFT_CONFORMANCE_TEST_SUITE_GENS", "<unset>")));
  }
  return result;
}

std::set<std::string> getNonconforming() {
  return parseNonconforming(
      readFromFile(std::getenv("THRIFT_CONFORMANCE_NONCONFORMING")));
}

} // namespace apache::thrift::conformance
