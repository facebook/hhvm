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

#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>

using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_program;
using apache::thrift::compiler::t_program_bundle;
using apache::thrift::compiler::codemod::file_manager;

namespace {

class RemovePhpNamespaceIfHackPresent final {
 public:
  RemovePhpNamespaceIfHackPresent(source_manager& sm, t_program& program)
      : fm_(sm, program), program_(program) {}

  void run() {
    const auto& namespaces = program_.namespaces();

    // Only act when namespace php is present and non-empty.
    auto php_it = namespaces.find("php");
    if (php_it == namespaces.end() || php_it->second->ns().empty()) {
      return;
    }

    // Only act when namespace hack is also present.
    if (!namespaces.count("hack")) {
      return;
    }

    fm_.remove_namespace("php");
    fm_.apply_replacements();
  }

 private:
  file_manager fm_;
  const t_program& program_;
};

} // namespace

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& sm, t_program_bundle& pb) {
        RemovePhpNamespaceIfHackPresent(sm, *pb.root_program()).run();
      });
}
