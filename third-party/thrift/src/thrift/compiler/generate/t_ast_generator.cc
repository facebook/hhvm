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
#include <unordered_map>
#include <vector>

#include <boost/filesystem.hpp>

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_include.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/generate/t_generator.h>
#include <thrift/compiler/lib/const_util.h>
#include <thrift/compiler/lib/schematizer.h>

#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/ast_types_custom_protocol.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace {

enum class ast_protocol {
  json,
  debug,
  compact,
};

template <typename Writer, typename T>
std::string serialize(const T& val) {
  folly::IOBufQueue queue;
  Writer proto;
  proto.setOutput(&queue);
  op::encode<type::struct_t<T>>(proto, val);
  auto buf = queue.move();
  auto br = buf->coalesce();
  return std::string(reinterpret_cast<const char*>(br.data()), br.size());
}
} // namespace

/**
 * AST generator.
 */
class t_ast_generator : public t_generator {
 public:
  using t_generator::t_generator;

  void process_options(
      const std::map<std::string, std::string>& options) override {
    out_dir_base_ = "gen-ast";
    bool protocol_set = false;
    for (auto& pair : options) {
      if (pair.first == "protocol") {
        protocol_set = true;
        if (pair.second == "json") {
          protocol_ = ast_protocol::json;
        } else if (pair.second == "debug") {
          protocol_ = ast_protocol::debug;
        } else if (pair.second == "compact") {
          protocol_ = ast_protocol::compact;
        } else {
          throw std::runtime_error(
              fmt::format("Unknown protocol `{}`", pair.second));
        }
      } else if (pair.first == "include_generated") {
        include_generated_ = true;
      } else if (pair.first == "ast") {
      } else {
        throw std::runtime_error(
            fmt::format("Unknown option `{}`", pair.first));
      }
    }
    if (!protocol_set) {
      throw std::runtime_error(
          "Missing required argument protocol=json|debug|compact");
    }
  }

  void generate_program() override;

 private:
  template <typename Id>
  Id positionToId(size_t pos) {
    return Id{int64_t(pos + 1)};
  }
  template <typename Id>
  size_t idToPosition(Id id) {
    return size_t(id) - 1;
  }

  std::ofstream f_out_;
  ast_protocol protocol_;
  bool include_generated_{false};
};

void t_ast_generator::generate_program() {
  boost::filesystem::create_directory(get_out_dir());
  std::string fname = fmt::format("{}/{}.ast", get_out_dir(), program_->name());
  f_out_.open(fname.c_str(), std::ios::out | std::ios::binary);

  cpp2::Ast ast;
  std::unordered_map<const t_program*, apache::thrift::type::ProgramId>
      program_index;
  std::unordered_map<const t_named*, apache::thrift::type::DefinitionId>
      definition_index;

  auto intern_value = [&](std::unique_ptr<t_const_value> val,
                          t_program* = nullptr) {
    // TODO: deduplication
    auto& values = ast.values().ensure();
    auto ret = positionToId<t_program::value_id>(values.size());
    values.push_back(const_to_value(*val));
    return ret;
  };

  auto populate_defs = [&](t_program* program) {
    auto& defs = ast.programs()
                     ->at(idToPosition(program_index.at(program)))
                     .definitions()
                     .ensure();
    for (auto& def : program->definitions()) {
      if (def.generated() && !include_generated_) {
        continue;
      }
      defs.push_back(definition_index.at(&def));
    }
  };

  auto set_source_range = [&](const t_named& def,
                              type::DefinitionAttrs& attrs,
                              const t_program* program = nullptr) {
    program = program ? program : def.program();

    {
      resolved_location begin(def.src_range().begin, source_mgr_);
      resolved_location end(def.src_range().end, source_mgr_);
      type::SourceRange& range = *attrs.sourceRange();
      range.programId() = program_index.at(program);
      range.beginLine() = begin.line();
      range.beginColumn() = begin.column();
      range.endLine() = end.line();
      range.endColumn() = end.column();
    }

    if (def.has_doc()) {
      resolved_location begin(def.doc_range().begin, source_mgr_);
      resolved_location end(def.doc_range().end, source_mgr_);
      type::SourceRange& range = *attrs.docs()->sourceRange();
      range.programId() = program_index.at(program);
      range.beginLine() = begin.line();
      range.beginColumn() = begin.column();
      range.endLine() = end.line();
      range.endColumn() = end.column();
    }
  };
  auto set_child_source_ranges = [&](const auto& node, auto& parent_def) {
    using Node = std::decay_t<decltype(node)>;
    if constexpr (std::is_base_of_v<t_structured, Node>) {
      int i = 0;
      for (const auto& field : node.fields()) {
        auto& def = parent_def.fields()[i++];
        assert(def.name() == field.name());
        set_source_range(field, *def.attrs(), node.program());
      }
    } else if constexpr (std::is_same_v<Node, t_enum>) {
      int i = 0;
      for (const auto& value : node.values()) {
        auto& def = parent_def.values()[i++];
        assert(def.name() == value.name());
        set_source_range(value, *def.attrs(), node.program());
      }
    }
  };

  if (!program_bundle_.root_program()->scope()->find_by_uri(
          "facebook.com/thrift/type/TypeUri")) {
    throw std::runtime_error(
        "thrift/lib/thrift/schema.thrift must be present in one of the include paths.");
  }

  schematizer schema_source(&program_bundle_, intern_value);
  const_ast_visitor visitor;
  visitor.add_program_visitor([&](const t_program& program) {
    if (program_index.count(&program)) {
      return;
    }

    auto& programs = *ast.programs();
    auto pos = programs.size();
    auto program_id = positionToId<apache::thrift::type::ProgramId>(pos);
    program_index[&program] = program_id;
    hydrate_const(programs.emplace_back(), *schema_source.gen_schema(program));

    for (auto* include : program.get_included_programs()) {
      // This could invalidate references into `programs`.
      visitor(*include);
      programs.at(pos).includes().ensure().push_back(program_index.at(include));
      populate_defs(include);
    }

    cpp2::SourceInfo info;
    info.fileName() = program.path();

    for (const auto& [lang, incs] : program.language_includes()) {
      for (const auto& inc : incs) {
        info.languageIncludes()[lang].push_back(static_cast<type::ValueId>(
            intern_value(std::make_unique<t_const_value>(inc))));
      }
    }

    for (const auto& [lang, langNamespace] : program.namespaces()) {
      info.namespaces()[lang] = static_cast<type::ValueId>(
          intern_value(std::make_unique<t_const_value>(langNamespace)));
    }

    ast.sources()[program_id] = std::move(info);

    // Note: have to populate `definitions` after the visitor completes since it
    // visits the children after this lambda returns.
  });

#define THRIFT_ADD_VISITOR(kind)                                     \
  visitor.add_##kind##_visitor([&](const t_##kind& node) {           \
    if (node.generated() && !include_generated_) {                   \
      return;                                                        \
    }                                                                \
    auto& definitions = *ast.definitions();                          \
    auto pos = definitions.size();                                   \
    definition_index[&node] =                                        \
        positionToId<apache::thrift::type::DefinitionId>(pos);       \
    auto& def = definitions.emplace_back().kind##Def_ref().ensure(); \
    hydrate_const(def, *schema_source.gen_schema(node));             \
    set_source_range(node, *def.attrs());                            \
    set_child_source_ranges(node, def);                              \
  })
  THRIFT_ADD_VISITOR(service);
  THRIFT_ADD_VISITOR(interaction);
  THRIFT_ADD_VISITOR(struct);
  THRIFT_ADD_VISITOR(union);
  THRIFT_ADD_VISITOR(exception);
  THRIFT_ADD_VISITOR(typedef);
  THRIFT_ADD_VISITOR(enum);
  THRIFT_ADD_VISITOR(const);
#undef THRIFT_ADD_VISITOR
  visitor(*program_);
  populate_defs(program_);

  switch (protocol_) {
    case ast_protocol::json:
      f_out_ << serialize<SimpleJSONProtocolWriter>(ast);
      break;
    case ast_protocol::debug:
      f_out_ << serialize<DebugProtocolWriter>(ast);
      break;
    case ast_protocol::compact:
      f_out_ << serialize<CompactProtocolWriter>(ast);
      break;
  }
  f_out_.close();
}

THRIFT_REGISTER_GENERATOR(
    ast,
    "AST",
    "    protocol:        Which of [json|debug|compact] protocols to use for serialization.\n");

} // namespace compiler
} // namespace thrift
} // namespace apache
