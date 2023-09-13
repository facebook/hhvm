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

#include <vector>

#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/codemod/package_generator.h>
#include <thrift/compiler/compiler.h>

using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_program;
using apache::thrift::compiler::codemod::file_manager;
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
    auto pkg = get_package();
    if (pkg.empty()) {
      return;
    }
    auto offset = fm_.get_namespace_offset();
    fm_.add({offset, offset, get_replacement_content(pkg)});
  }

  void run() {
    visit_program();
    fm_.apply_replacements();
  }

 private:
  file_manager fm_;
  const t_program& prog_;

  std::string get_package() const {
    // If no namespaces exist, use the file path as the package.
    if (prog_.namespaces().empty()) {
      return package_name_generator::from_file_path(prog_.path());
    }

    if (prog_.namespaces().size() == 1) {
      // If there is only a single namespace, use that to generate the package
      // name.
      auto ns = prog_.namespaces().begin();
      return package_name_generator(ns->first, ns->second).generate();
    }
    auto gen = package_name_generator_util::from_namespaces(prog_.namespaces());

    // If there are multiple namespaces, then find the one that works with
    // most namespaces.
    auto pkg = gen.find_common_package();
    if (!pkg.empty()) {
      return pkg;
    }
    pkg = gen.get_package_from_common_identifiers();

    if (!pkg.empty()) {
      return pkg;
    }

    // If there are no common identifiers,
    // then prioritize the longest package as it is more likely to be
    // unique
    return gen.get_longest_package();
  }

  std::string get_replacement_content(const std::string& pkg) const {
    auto content = fmt::format("package \"{}\"\n\n", pkg);

    /*
     * Adding a package to a file without namespaces
     * breaks the existing references to generated code.
     *
     * For certain languages, in absence of namespace a default one is used.
     * Override the namespace from package by adding default namespaces.
     * This ensures that the existing references don't break.
     *
     * This is only needed if there are some definitions in the thrift file.
     * If there are no definitions, then this can be skipped.
     */
    if (!prog_.definitions().empty()) {
      auto has_ns = [&](std::string lang) {
        return prog_.namespaces().find(lang) != prog_.namespaces().end();
      };

      if (!has_ns("cpp2")) {
        content += "namespace cpp2 \"cpp2\"\n";
      }
      if (!has_ns("hack") && !has_ns("php")) {
        content += "namespace hack \"\"\n";
      }
      if (!has_ns("py3")) {
        content += "namespace py3 \"\"\n";
      }

      if (prog_.namespaces().empty()) {
        content += "\n";
      }
    }
    return content;
  }
};
} // namespace

int main(int argc, char** argv) {
  auto source_mgr = source_manager();
  auto program_bundle = parse_and_get_program(
      source_mgr, std::vector<std::string>(argv, argv + argc));
  if (!program_bundle) {
    return 1;
  }
  auto program = program_bundle->root_program();
  add_package(source_mgr, *program).run();

  return 0;
}
