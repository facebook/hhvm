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

#include <set>
#include <string>
#include <fmt/core.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>

using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_program_bundle;

namespace apache::thrift::compiler {
namespace {

class RemoveDuplicateNamespaces final {
 public:
  RemoveDuplicateNamespaces(source_manager& src_manager, t_program& program)
      : source_manager_(src_manager),
        program_(program),
        file_manager_(source_manager_, program) {}

  void run() {
    const std::string_view old_content = file_manager_.old_content();
    std::set<std::string> seen_languages;
    bool has_duplicates = false;

    for (const t_namespace* ns : program_.all_namespace_nodes()) {
      const std::string& language = ns->language();

      if (seen_languages.insert(language).second) {
        // First occurrence of this language — keep it.
        continue;
      }

      // Duplicate found. Compute the line range to remove.
      const size_t begin_offset =
          file_manager_.to_offset(ns->src_range().begin);
      const size_t src_end_offset =
          file_manager_.to_offset(ns->src_range().end);
      const size_t newline_offset = old_content.find('\n', src_end_offset);

      // Check for any trailing content after the namespace node
      // (e.g. inline comments) before the end of the line.
      if (newline_offset != std::string_view::npos &&
          newline_offset != src_end_offset) {
        // Compute a 1-based line number for the warning.
        size_t line_number = 1;
        for (size_t i = 0; i < begin_offset && i < old_content.size(); ++i) {
          if (old_content[i] == '\n') {
            ++line_number;
          }
        }
        fmt::print(
            stderr,
            "{}:{}: warning: skipping file due to trailing content on "
            "duplicate namespace '{}'\n",
            program_.path(),
            line_number,
            language);
        return;
      }

      // Remove the entire line (including the trailing newline).
      const size_t end_offset = newline_offset == std::string_view::npos
          ? old_content.size()
          : newline_offset + 1;
      file_manager_.add({begin_offset, end_offset, ""});
      has_duplicates = true;
    }

    if (has_duplicates) {
      file_manager_.apply_replacements();
    }
  }

 private:
  source_manager& source_manager_;
  t_program& program_;
  codemod::file_manager file_manager_;
};

} // namespace
} // namespace apache::thrift::compiler

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& sm, t_program_bundle& pb) {
        apache::thrift::compiler::RemoveDuplicateNamespaces(
            sm, *pb.root_program())
            .run();
      });
}
