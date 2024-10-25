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

#include <fmt/core.h>
#include "thrift/compiler/source_location.h"

int main() {
  // Print the header.
  fmt::print(
      "// {}generated\n"
      "\n"
      "#include <thrift/annotation/bundled_annotations.h>\n"
      "\n",
      '@');

  // Read the file.
  apache::thrift::compiler::source_manager sm;
  std::string_view content = sm.get_file("scope.thrift")->text;
  content.remove_suffix(1); // Remove trailing NUL.

  // Print the content.
  fmt::print(
      "namespace apache::thrift::detail {{\n"
      "\n"
      "const std::string_view bundled_annotations::scope_file_content() {{\n"
      "  return R\"{0}({1}){0}\";\n"
      "}}\n"
      "\n"
      "}} // namespace apache::thrift::detail\n",
      "__FBTHRIFT_TAG__",
      content);
}
