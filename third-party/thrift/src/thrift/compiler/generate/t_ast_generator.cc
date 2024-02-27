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
#include <stdexcept>
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
      } else if (pair.first == "source_ranges") {
        source_ranges_ = true;
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
  bool source_ranges_{false};
  bool is_root_program_{true};
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

  auto src_range = [&](source_range in, const t_program* program) {
    resolved_location begin(in.begin, source_mgr_);
    resolved_location end(in.end, source_mgr_);
    type::SourceRange range;
    range.programId() = program_index.at(program);
    range.beginLine() = begin.line();
    range.beginColumn() = begin.column();
    range.endLine() = end.line();
    range.endColumn() = end.column();
    return range;
  };

  auto set_source_range = [&](const t_named& def,
                              type::DefinitionAttrs& attrs,
                              const t_program* program = nullptr) {
    program = program ? program : def.program();
    attrs.sourceRange() = src_range(def.src_range(), program);
    if (def.has_doc()) {
      attrs.docs()->sourceRange() = src_range(def.doc_range(), program);
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
    } else if constexpr (std::is_base_of_v<t_interface, Node>) {
      int i = 0;
      for (const auto& function : node.functions()) {
        auto& def = parent_def.functions()[i++];
        assert(def.name() == function.name());
        set_source_range(function, *def.attrs(), node.program());
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
    if (program.has_doc()) {
      programs.back().attrs()->docs()->sourceRange() =
          src_range(program.doc_range(), &program);
    }

    auto is_root_program = std::exchange(is_root_program_, false);
    for (auto* include : program.get_included_programs()) {
      // This could invalidate references into `programs`.
      visitor(*include);
      programs.at(pos).includes().ensure().push_back(program_index.at(include));
      populate_defs(include);
    }
    is_root_program_ = is_root_program;

    // Double write to deprecated externed path. (T161963504)
    // The new path is populated in the Program struct by the schematizer.
    cpp2::SourceInfo info;
    info.fileName() =
        source_mgr_.found_include_file(program.path()).value_or(program.path());

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

  // Populate identifier source range map if enabled.
  auto span = [&](t_type_ref ref) {
    auto combinator = [&](t_type_ref ref, auto& recurse) -> void {
      if (!ref || !source_ranges_ || !is_root_program_) {
        return;
      }
      while (ref->is_typedef() &&
             static_cast<const t_typedef&>(*ref).typedef_kind() !=
                 t_typedef::kind::defined) {
        ref = static_cast<const t_typedef&>(*ref).type();
      }

      if (auto list_type = dynamic_cast<const t_list*>(ref.get_type())) {
        recurse(list_type->elem_type(), recurse);
      } else if (auto set_type = dynamic_cast<const t_set*>(ref.get_type())) {
        recurse(set_type->elem_type(), recurse);
      } else if (auto map_type = dynamic_cast<const t_map*>(ref.get_type())) {
        recurse(map_type->key_type(), recurse);
        recurse(map_type->val_type(), recurse);
      } else if (ref->is_base_type()) {
      } else {
        try {
          cpp2::IdentifierRef ident;
          ident.range() = src_range(ref.src_range(), program_);
          if (const auto& uri = ref->uri(); !uri.empty()) {
            ident.uri()->uri_ref() = uri;
          } else {
            ident.uri()->scopedName_ref() = ref->get_scoped_name();
          }
          ast.identifierSourceRanges()->push_back(std::move(ident));
        } catch (const std::invalid_argument&) {
          fmt::print(
              stderr, "No source range set for reference to {}\n", ref->name());
        }
      }
    };
    return combinator(ref, combinator);
  };

  auto const_spans = [&](const t_const_value* val) {
    auto combinator = [&](const t_const_value* val, auto recurse) -> void {
      if (!val || !source_ranges_ || !is_root_program_) {
        return;
      }
      if (auto rng = val->ref_range(); rng.begin != source_location{}) {
        try {
          cpp2::IdentifierRef ident;
          ident.range() = src_range(rng, program_);
          if (auto enum_owner = val->get_enum()) {
            if (const auto& uri = enum_owner->uri(); !uri.empty()) {
              ident.uri()->uri_ref() = uri;
            } else {
              ident.uri()->scopedName_ref() =
                  enum_owner->program()->scope_name(*enum_owner);
            }
            ident.enumValue() = val->get_owner()->get_name();
            ast.identifierSourceRanges()->push_back(std::move(ident));
          } else if (auto owner = val->get_owner()) {
            if (const auto& uri = owner->uri(); !uri.empty()) {
              ident.uri()->uri_ref() = uri;
            } else {
              ident.uri()->scopedName_ref() =
                  owner->program()->scope_name(*owner);
            }
            ast.identifierSourceRanges()->push_back(std::move(ident));
          } else {
            // Const used before being defined. The compiler already emitted a
            // warning so do nothing.
          }
        } catch (const std::invalid_argument&) {
          fmt::print(
              stderr,
              "No source range set for reference to {}\n",
              val->get_owner() ? val->get_owner()->name()
                               : "unnamed const val");
        }
      } else if (const auto& list = val->get_list(); !list.empty()) {
        for (auto item : list) {
          recurse(item, recurse);
        }
      } else if (const auto& map = val->get_map(); !map.empty()) {
        for (auto [k, v] : map) {
          recurse(k, recurse);
          recurse(v, recurse);
        }
      }
    };
    combinator(val, combinator);
  };

  visitor.add_field_visitor([&](const t_field& node) {
    span(node.type());
    const_spans(node.default_value());
  });
  visitor.add_function_visitor([&](const t_function& node) {
    if (node.has_return_type()) {
      span(node.return_type());
    }
    if (auto stream_type = node.stream()) {
      span(stream_type->elem_type());
      for (const auto& exn : get_elems(stream_type->exceptions())) {
        span(exn.type());
      }
    } else if (auto sink_type = node.sink()) {
      span(sink_type->elem_type());
      for (const auto& exn : get_elems(sink_type->sink_exceptions())) {
        span(exn.type());
      }
      span(sink_type->final_response_type());
      for (const auto& exn :
           get_elems(sink_type->final_response_exceptions())) {
        span(exn.type());
      }
    }

    for (const auto& param : node.params().fields()) {
      span(param.type());
      const_spans(param.default_value());
    }
    for (const auto& exn : get_elems(node.exceptions())) {
      span(exn.type());
    }
  });
  visitor.add_typedef_visitor(
      [&](const t_typedef& node) { span(node.type()); });
  visitor.add_service_visitor([&](const t_service& node) {
    if (auto extends = node.extends()) {
      span({*extends, node.extends_range()});
    }
  });
  visitor.add_const_visitor([&](const t_const& node) {
    if (node.generated()) {
      return;
    }
    span(node.type_ref());
    const_spans(node.value());
  });

  visitor(*program_);
  populate_defs(program_);
  if (source_ranges_) {
    for (auto inc : program_->includes()) {
      cpp2::IncludeRef ident;
      ident.range() = src_range(inc->str_range(), program_);
      ident.target() = program_index.at(inc->get_program());
      ast.includeSourceRanges()->push_back(std::move(ident));
    }
  }

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
    "    protocol:          Which of [json|debug|compact] protocols to use for serialization.\n"
    "    include_generated: Enables schematization of generated (patch) types.\n"
    "    source_ranges:     Enables population of the identifier source range map.\n"
    "");

} // namespace compiler
} // namespace thrift
} // namespace apache
