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

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>

// Input:
//   A directory containing ".thrift" files. All files (recursively) in the
//   directory tree are included.
//
// Output:
//   C++ code for a function returning a map<string, string> where the key
//   is the file path, and the value is the source code contained within that
//   file.

namespace fs = std::filesystem;

namespace {

using lines_t = std::vector<std::string>;

void populate_one_file(
    const fs::path& root,
    std::map<std::string, lines_t>& out,
    const fs::path& path) {
  std::ifstream file(path);
  lines_t lines;
  std::string line;
  while (std::getline(file, line)) {
    lines.push_back(line);
  }
  // .thrift files should always end with a trailing newline.
  lines.emplace_back();

  std::string key = fmt::format(
      "{}/{}", root.generic_string(), path.lexically_normal().generic_string());
  out[std::move(key)] = std::move(lines);
}

void populate_recursively(
    const fs::path& root,
    std::map<std::string, lines_t>& out,
    const fs::path& path) {
  for (const auto& entry : fs::directory_iterator(path)) {
    const auto& filepath = entry.path();
    if (entry.is_directory()) {
      populate_recursively(root, out, filepath);
    } else if (entry.is_regular_file() && filepath.extension() == ".thrift") {
      populate_one_file(root, out, filepath);
    }
  }
}

template <typename Func>
void for_each_chunk(const lines_t& lines, std::size_t chunk_size, Func func) {
  auto begin = lines.begin();
  auto end = lines.end();
  while (begin != end) {
    auto chunk_end = begin;
    auto distance = std::min(
        chunk_size, static_cast<std::size_t>(std::distance(begin, end)));
    std::advance(chunk_end, distance);
    // We will run into trouble if the line starts with punctuation, as it will
    // be interpreted as part of the raw string delimiter. Skip forward to a
    // line that starts with a space or is empty to avoid this.
    while (chunk_end != end && !chunk_end->empty() &&
           chunk_end->front() != ' ') {
      ++chunk_end;
    }
    func(begin, chunk_end);
    begin = chunk_end;
  }
}

void print_one_file(std::string_view filename, const lines_t& content) {
  // The "chunking" logic here is to avoid the generated file being too big.
  // Specifically, MSVC produces C2026 when...
  //     The string was longer than the limit of 16380 single-byte characters.
  // https://learn.microsoft.com/en-us/cpp/error-messages/compiler-errors-1/compiler-error-c2026?view=msvc-170

  // The size limit is around 16KB and with a *very* conservative estimate of
  // around ~100 bytes per line, we can fit ~150 lines in one string literal.
  // The goal here is not to maximize the number of characters that fit but to
  // avoid the noise from raw string literal delimiter (i.e. __FBTHRIFT_TAG__).
  constexpr std::size_t num_chunk_lines = 150;

  fmt::print("  {{\"{0}\",\n", filename);
  for_each_chunk(content, num_chunk_lines, [](auto begin, auto end) {
    fmt::print(
        "R\"{0}({1}){0}\"\n", "__FBTHRIFT_TAG__", fmt::join(begin, end, "\n"));
  });
  fmt::print("}},\n");
}

void print_all_files(
    std::string_view function_name,
    const std::map<std::string, lines_t>& files) {
  fmt::print(
      "namespace apache::thrift::detail {{\n"
      "\n"
      "const std::map<std::string, std::string>& {0}() {{\n"
      "  static const std::map<std::string, std::string> files = {{\n",
      function_name);
  for (const auto& [name, content] : files) {
    print_one_file(name, content);
  }
  fmt::print(
      "  }};\n"
      "  return files;\n"
      "}}\n"
      "\n"
      "}} // namespace apache::thrift::detail\n");
}

} // namespace

int main(int argc, char** argv) {
  if (argc != 4) {
    fmt::print(stderr, "Usage: {} ROOT FUNCTION_NAME DIRECTORY\n", argv[0]);
    return 1;
  }

  // The prefix added to each key in the map, e.g. "thrift/annotation/".
  const auto root = fs::path(argv[1]);
  // The name of the C++ function generated in apache::thrift::detail namespace.
  const auto function_name = std::string(argv[2]);
  // The directory to read files from.
  const auto directory = fs::path(argv[3]);

  // Print the header.
  fmt::print(
      "// {}generated\n"
      "\n"
      "#include <map>\n"
      "#include <string>\n"
      "\n",
      '@');

  // Read the files.
  std::map<std::string, lines_t> files;
  populate_recursively(root, files, directory);

  // Print the content.
  print_all_files(function_name, files);
}
