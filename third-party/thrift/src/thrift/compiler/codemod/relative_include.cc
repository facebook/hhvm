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
#include <set>
#include <string>

#include <fmt/core.h>
#include <thrift/compiler/ast/t_include.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>

using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_program_bundle;

namespace apache::thrift::compiler {
namespace {

class CodemodRelativeInclude final {
 public:
  CodemodRelativeInclude(source_manager& src_manager, t_program& program)
      : source_manager_(src_manager),
        program_(program),
        file_manager_(source_manager_, program) {}

  void run() {
    bool any_codemodded = false;

    for (const auto* include : program_.includes()) {
      any_codemodded |= maybe_codemod_include(*include);
    }

    if (any_codemodded) {
      file_manager_.apply_replacements();
    }
  }

 private:
  source_manager& source_manager_;
  t_program& program_;
  codemod::file_manager file_manager_;

  bool maybe_codemod_include(const t_include& include) {
    const auto& program = *include.get_program();
    if (include.raw_path() == program.full_path()) {
      return false;
    }

    std::set<std::string> raw_path_heuristic = {// WWW
                                                "cf/",
                                                "cfa/",
                                                "cfdsi/",
                                                "cfgk/",
                                                "cfmeta/",
                                                "cfsitevars/",
                                                "fbcode/",
                                                "fbsource/",
                                                "igsrv/",
                                                // fbcode
                                                "arvr/",
                                                "fbandroid/",
                                                "fbobjc/",
                                                "xplat/",
                                                // bundled
                                                "thrift/annotation/"};
    if (std::any_of(
            raw_path_heuristic.begin(),
            raw_path_heuristic.end(),
            [&](const auto& root) {
              return include.raw_path().starts_with(root);
            })) {
      return false;
    }

    // Already relative to configerator/source
    if (program.full_path().starts_with("source/") &&
        program.full_path().substr(7) == include.raw_path()) {
      return false;
    }
    if (program.full_path().find("/instagram-server/") != std::string::npos) {
      return false;
    }

    auto parent_path = std::filesystem::path{program_.path()}.parent_path();
    if (parent_path.is_relative()) {
      auto resolved_path =
          (parent_path / include.raw_path()).lexically_normal();

      // Configerator codemods are ran from configerator repo root, strip away
      // source.
      if (!resolved_path.empty() &&
          resolved_path.begin()->string() == "source") {
        resolved_path = resolved_path.lexically_relative("source");
      }

      file_manager_.add(
          {.begin_pos = include.src_range().begin.offset(),
           .end_pos = include.src_range().end.offset(),
           .new_content =
               fmt::format("include \"{}\"", resolved_path.c_str())});
      return true;
    } else {
      throw std::runtime_error(
          "Provide the relative path from the repo root for the codemod.");
    }
  }
};

} // namespace
} // namespace apache::thrift::compiler

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& sm, t_program_bundle& pb) {
        apache::thrift::compiler::CodemodRelativeInclude(sm, *pb.root_program())
            .run();
      });
}
