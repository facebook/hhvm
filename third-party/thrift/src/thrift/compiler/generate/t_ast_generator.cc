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

#include <fstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/filesystem.hpp>

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_include.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/generate/t_generator.h>

#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/ast_types_custom_protocol.h>

namespace apache {
namespace thrift {
namespace compiler {

enum class ast_protocol {
  json,
  debug,
};

/**
 * AST generator.
 */
class t_ast_generator : public t_generator {
 public:
  using t_generator::t_generator;

  void process_options(
      const std::map<std::string, std::string>& options) override {
    out_dir_base_ = "gen-ast";
    protocol_ = ast_protocol::debug;
    for (auto& pair : options) {
      if (pair.first == "protocol") {
        if (pair.second == "debug") {
          protocol_ = ast_protocol::debug;
        } else if (pair.second == "json") {
          protocol_ = ast_protocol::json;
          throw std::runtime_error("TODO");
        } else {
          throw std::runtime_error(
              fmt::format("Unknown protocol `{}`", pair.second));
        }
      } else if (pair.first == "ast") {
      } else {
        throw std::runtime_error(
            fmt::format("Unknown option `{}`", pair.first));
      }
    }
  }

  void generate_program() override;

 private:
  std::ofstream f_out_;
  ast_protocol protocol_;
};

void t_ast_generator::generate_program() {
  boost::filesystem::create_directory(get_out_dir());
  std::string fname = fmt::format("{}/{}.ast", get_out_dir(), program_->name());
  f_out_.open(fname.c_str());

  cpp2::AST ast;
  std::unordered_set<const t_program*> visited;
  const_ast_visitor visitor;
  visitor.add_program_visitor([&](const t_program& program) {
    if (!visited.insert(&program).second) {
      return;
    }

    ast.schema()->programs()->emplace_back();
    auto& schema = ast.schema()->programs()->back();
    auto id = ast.schema()->programs()->size();
    // Dummy schematization logic. TODO: connect to schematizer
    schema.name() = program.name();

    for (auto* include : program.get_included_programs()) {
      visitor(*include);
    }

    // TODO: sourceInfo
    // TODO: languageIncludes
    (void)id;
  });
  // TODO: other visitors
  visitor(*program_);

  f_out_ << debugString(ast);
  // TODO: JSON
  f_out_.close();
}

THRIFT_REGISTER_GENERATOR(
    ast,
    "AST",
    "    protocol:        Which of [json|debug] protocols to use for serialization.\n");

} // namespace compiler
} // namespace thrift
} // namespace apache
