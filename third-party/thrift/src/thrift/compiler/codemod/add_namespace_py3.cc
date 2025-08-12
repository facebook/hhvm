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

using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_program;
using apache::thrift::compiler::t_program_bundle;
using apache::thrift::compiler::codemod::file_manager;

namespace {

class add_namespace {
 public:
  add_namespace(
      source_manager& sm, t_program& program, const std::string& the_namespace)
      : fm_(sm, program), prog_(program), namespace_(the_namespace) {}

  void visit_program() {
    auto namespaces = get_namespaces();
    if (!namespaces.contains("py3")) {
      auto offset = fm_.get_namespace_offset();
      fm_.add(
          {offset,
           offset,
           std::string("namespace py3 ") + namespace_ + "\n" +
               (namespaces.empty() ? "\n" : "")});
    }
  }

  void run() {
    visit_program();
    fm_.apply_replacements();
  }

 private:
  file_manager fm_;
  const t_program& prog_;
  const std::string namespace_;

  std::map<std::string, std::string> get_namespaces() const {
    std::map<std::string, std::string> namespaces;
    for (const auto& [lang, ns] : prog_.namespaces()) {
      if (!ns.empty()) {
        namespaces[lang] = ns;
      }
    }
    return namespaces;
  }
};
} // namespace

int main(int argc, char** argv) {
  if (argc <= 2) {
    fmt::print(stderr, "Usage: {} <thrift-file> <namespace_py3>\n", argv[0]);
    return 1;
  }
  argc -= 1;
  std::string the_namespace = argv[argc];
  return apache::thrift::compiler::run_codemod(
      argc, argv, [the_namespace](source_manager& sm, t_program_bundle& pb) {
        add_namespace(sm, *pb.root_program(), the_namespace).run();
      });
}
