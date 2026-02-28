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

#include <gtest/gtest.h>
#include <folly/json.h>
#include <thrift/conformance/if/gen-cpp2/test_suite_types.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache::thrift::conformance {

// Gets and env value, returning dflt if not found.
const char* getEnvOr(const char* name, const char* dflt);

// Gets an env value, throwing an exception if not found.
const char* getEnvOrThrow(const char* name);

// Reads the contents of a file into a string.
// Returns "" if filename == nullptr.
std::string readFromFile(const char* filename);

// Runs the given command and returns its stdout as a string.
std::string readFromCmd(const std::vector<std::string>& argv);

// Names default to parent directory, or can be customized by appending
// '#<name>' to the command. If the command itself has a '#' character in it,
// appending an additional "#" will cause it to parse correctly.
std::pair<std::string_view, std::string_view> parseNameAndCmd(
    std::string_view entry);

// Parses commands (and optionally custom names) seporated by ','
std::map<std::string_view, std::string_view> parseCmds(
    std::string_view cmdsStr);

// Parses a set of non-conforming test names, seporated by '/n'
//
// Use # for comments.
std::set<std::string> parseNonconforming(std::string_view data);

// Reads or generates the test suites to run.
std::vector<TestSuite> getSuites();

// Read the list of non-conforming tests.
std::set<std::string> getNonconforming();

// From a newer version of gtest.
//
// TODO(afuller): Delete once gtest is updated.
template <typename Factory>
inline testing::TestInfo* registerTest(
    const char* test_suite_name,
    const char* test_name,
    const char* type_param,
    const char* value_param,
    const char* file,
    int line,
    Factory factory) {
  using TestT = std::remove_pointer_t<decltype(factory())>;

  class FactoryImpl : public testing::internal::TestFactoryBase {
   public:
    explicit FactoryImpl(Factory f) : factory_(std::move(f)) {}
    testing::Test* CreateTest() override { return factory_(); }

   private:
    Factory factory_;
  };

  return testing::internal::MakeAndRegisterTestInfo(
      test_suite_name,
      test_name,
      type_param,
      value_param,
      testing::internal::CodeLocation(file, line),
      testing::internal::GetTypeId<TestT>(),
      TestT::SetUpTestCase,
      TestT::TearDownTestCase,
      new FactoryImpl{std::move(factory)});
}

// Get conformance test description
template <typename T>
std::string genDescription(const T& test) {
  if (test.description().has_value()) {
    return *test.description() + "\n";
  }
  return "";
}

// Converts conformance test tags into helpful links, so they can be reported
// with failures.
template <typename T>
std::string genTagLinks(const T& tagged) {
  std::string result;
  for (const auto& tag : *tagged.tags()) {
    result += "    sdoc thrift/docs/" + tag + "\n";
  }
  return result;
}

template <class T>
std::string jsonify(const T& o) {
  std::string json =
      apache::thrift::SimpleJSONSerializer::serialize<std::string>(o);
  return folly::toPrettyJson(folly::parseJson(std::move(json)));
}

} // namespace apache::thrift::conformance
