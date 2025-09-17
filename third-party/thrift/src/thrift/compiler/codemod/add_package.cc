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

#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/codemod/package_generator.h>

using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_program;
using apache::thrift::compiler::t_program_bundle;
using apache::thrift::compiler::codemod::file_manager;
using apache::thrift::compiler::codemod::get_package;
using apache::thrift::compiler::codemod::get_replacement_content;
using apache::thrift::compiler::codemod::package_name_generator;
using apache::thrift::compiler::codemod::package_name_generator_util;

namespace {

class add_package {
 public:
  add_package(source_manager& sm, t_program& program)
      : fm_(sm, program), prog_(program) {}

  void visit_program() {
    if (!prog_.package().empty()) {
      return;
    }
    auto pkg = get_package(prog_.path(), prog_.namespaces());
    if (pkg.empty()) {
      return;
    }
    auto offset = fm_.get_namespace_offset();
    fm_.add({offset, offset, get_replacement_content(prog_, pkg)});

    if (!prog_.namespaces().contains("cpp2") &&
        prog_.namespaces().contains("cpp")) {
      // get_replacement_content will always generate a cpp2 namespace
      fm_.remove_namespace("cpp");
    }
  }

  void run() {
    visit_program();
    fm_.apply_replacements();
  }

 private:
  file_manager fm_;
  const t_program& prog_;
};
} // namespace

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& sm, t_program_bundle& pb) {
        add_package(sm, *pb.root_program()).run();
      });
}
