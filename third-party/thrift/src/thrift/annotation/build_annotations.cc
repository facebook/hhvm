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
#include <fmt/core.h>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
  if (argc != 2) {
    fmt::print(stderr, "usage: {} templates-dir\n", argv[0]);
    return 1;
  }

  const auto path = fs::path(argv[1]);

  // Print the header.
  fmt::print(
      "// {}generated\n"
      "\n"
      "#include <thrift/annotation/bundled_annotations.h>\n"
      "\n",
      '@');

  // Read the files.
  std::map<std::string, std::string> files;
  for (const auto& entry : fs::directory_iterator(path)) {
    const auto& filepath = entry.path();
    if (filepath.extension() == ".thrift" && fs::is_regular_file(filepath)) {
      std::ifstream file(filepath);
      auto buf = std::string(
          std::istreambuf_iterator<char>(file),
          std::istreambuf_iterator<char>());
      files[fmt::format(
          "thrift/annotation/{}", filepath.filename().generic_string())] =
          std::move(buf);
    }
  }

  // Print the content.
  fmt::print(
      "namespace apache::thrift::detail {{\n"
      "\n"
      "const std::map<std::string, std::string>& bundled_annotations::files() {{\n"
      "  static const std::map<std::string, std::string> files = {{\n");
  for (const auto& [path, content] : files) {
    fmt::print(
        "   {{\"{1}\", R\"{0}({2}){0}\"}},\n",
        "__FBTHRIFT_TAG__",
        path,
        content);
  }
  fmt::print(
      "  }};\n"
      "  return files;\n"
      "}}\n"
      "\n"
      "}} // namespace apache::thrift::detail\n");
}
