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

#include <filesystem>
#include <fstream>
#include <map>
#include <string>

#include <fmt/core.h>

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

void populate_one_file(
    const fs::path& root,
    std::map<std::string, std::string>& out,
    const fs::path& path) {
  std::ifstream file(path);
  auto buf = std::string(
      std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  std::string key = fmt::format(
      "{}/{}", root.generic_string(), path.lexically_normal().generic_string());
  out[std::move(key)] = std::move(buf);
}

void populate_recursively(
    const fs::path& root,
    std::map<std::string, std::string>& out,
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
  std::map<std::string, std::string> files;
  populate_recursively(root, files, directory);

  // Print the content.
  fmt::print(
      "namespace apache::thrift::detail {{\n"
      "\n"
      "const std::map<std::string, std::string>& {0}() {{\n"
      "  static const std::map<std::string, std::string> files = {{\n",
      function_name);
  for (const auto& [name, content] : files) {
    fmt::print(
        "   {{\"{1}\", R\"{0}({2}){0}\"}},\n",
        "__FBTHRIFT_TAG__",
        name,
        content);
  }
  fmt::print(
      "  }};\n"
      "  return files;\n"
      "}}\n"
      "\n"
      "}} // namespace apache::thrift::detail\n");
}
