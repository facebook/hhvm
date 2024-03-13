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

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/service/DataLoader.h>

namespace facebook::sbe::test {

using apache::thrift::sbe::MessageWrapper;
using folly::F14FastMap;

void DataLoader::loadLines(
    const std::string& path, std::vector<std::string>& lines) {
  std::ifstream file(path);
  if (file.is_open()) {
    std::string line;
    bool first = true;
    while (std::getline(file, line)) {
      if (first) {
        std::cout << "Skipping header line: " << line << std::endl;
        first = false;
        continue;
      }
      if (line.empty()) {
        std::cout << "Skipping empty line: " << line << std::endl;
        continue;
      }
      lines.push_back(line);
    }
    file.close();
  } else {
    throw std::runtime_error("Failed to open file " + path);
  }
}

static std::vector<std::string> split_csv(const std::string& input) {
  std::vector<std::string> result;
  std::string token;
  bool in_quotes = false;

  for (char c : input) {
    if (c == '\"')
      in_quotes = !in_quotes;
    else if (c == ',' && !in_quotes) {
      result.push_back(token);
      token = "";
    } else
      token += c;
  }

  result.push_back(token); // push last token
  return result;
}

void DataLoader::loadIntoMap(
    const std::string& path, F14FastMap<std::string, Customer>& map) {
  std::vector<std::string> lines;
  loadLines(path, lines);
  for (std::string& line : lines) {
    std::vector<std::string> parts = split_csv(line);
    if (parts.size() != 12) {
      std::cout << "parts size: " << parts.size() << std::endl;
      std::cout << "Skipping Invalid line: " << line << std::endl;
      exit(1);
      continue;
    }

    map[parts[1]] = {
        std::stoi(parts[0]),
        parts[1],
        parts[2],
        parts[3],
        parts[4],
        parts[5],
        parts[6],
        parts[7],
        parts[8],
        parts[9],
        parts[10],
        parts[11]};
  }
}

} // namespace facebook::sbe::test
