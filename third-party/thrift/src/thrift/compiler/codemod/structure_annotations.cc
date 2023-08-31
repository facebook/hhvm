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
#include <map>
#include <string>
#include <fmt/core.h>

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/compiler.h>
#include <thrift/compiler/lib/cpp2/util.h>
#include <thrift/compiler/lib/uri.h>

using namespace apache::thrift::compiler;

namespace {
class structure_annotations {
 public:
  structure_annotations(source_manager& sm, t_program& program)
      : fm_(sm, program), sm_(sm), prog_(program) {}

  std::set<std::string> visit_type(t_type_ref type, const t_named& node) {
    std::set<std::string> to_add;
    if (type->is_base_type() || type->is_container() ||
        (type->is_typedef() &&
         static_cast<const t_typedef&>(*type).typedef_kind() !=
             t_typedef::kind::defined)) {
      std::vector<t_annotation> to_remove;
      bool has_cpp_type = node.find_structured_annotation_or_null(kCppTypeUri);
      for (const auto& [name, data] : type->annotations()) {
        // cpp type
        if (name == "cpp.template" || name == "cpp2.template") {
          to_remove.emplace_back(name, data);
          if (type->get_true_type()->is_container() &&
              !std::exchange(has_cpp_type, true)) {
            to_add.insert(
                fmt::format("@cpp.Type{{template = \"{}\"}}", data.value));
            fm_.add_include("thrift/annotation/cpp.thrift");
          }
        }
        if (name == "cpp.type" || name == "cpp2.type") {
          to_remove.emplace_back(name, data);
          if (!std::exchange(has_cpp_type, true)) {
            to_add.insert(
                fmt::format("@cpp.Type{{name = \"{}\"}}", data.value));
            fm_.add_include("thrift/annotation/cpp.thrift");
          }
        }
      }

      if (!to_remove.empty() &&
          to_remove.size() == type->annotations().size()) {
        fm_.remove_all_annotations(*type);
      } else {
        for (const auto& annot : to_remove) {
          fm_.remove(annot);
        }
      }
    }

    return to_add;
  }

  void visit_def(const t_named& node) {
    std::set<std::string> to_add;
    if (auto typedf = dynamic_cast<const t_typedef*>(&node)) {
      to_add = visit_type(typedf->type(), node);
    } else if (auto field = dynamic_cast<const t_field*>(&node)) {
      to_add = visit_type(field->type(), node);
    }

    std::vector<t_annotation> to_remove;
    bool has_cpp_type = node.find_structured_annotation_or_null(kCppTypeUri);
    bool has_cpp_ref = node.find_structured_annotation_or_null(kBoxUri) ||
        node.find_structured_annotation_or_null(kInternBoxUri) ||
        node.find_structured_annotation_or_null(kCppRefUri);
    for (const auto& [name, data] : node.annotations()) {
      // cpp type
      bool is_typedef = dynamic_cast<const t_typedef*>(&node);
      if (name == "cpp.template" || name == "cpp2.template") {
        to_remove.emplace_back(name, data);
        if (is_typedef &&
            static_cast<const t_typedef&>(node)
                .type()
                ->get_true_type()
                ->is_container() &&
            !std::exchange(has_cpp_type, true)) {
          to_add.insert(
              fmt::format("@cpp.Type{{template = \"{}\"}}", data.value));
          fm_.add_include("thrift/annotation/cpp.thrift");
        }
      }
      if (name == "cpp.type" || name == "cpp2.type") {
        to_remove.emplace_back(name, data);
        if (is_typedef && !std::exchange(has_cpp_type, true)) {
          to_add.insert(fmt::format("@cpp.Type{{name = \"{}\"}}", data.value));
          fm_.add_include("thrift/annotation/cpp.thrift");
        }
      }

      // cpp ref
      bool is_field = dynamic_cast<const t_field*>(&node);
      if (name == "cpp.box" || name == "thrift.box") {
        to_remove.emplace_back(name, data);
        if (is_field && !std::exchange(has_cpp_ref, true)) {
          to_add.insert("@thrift.Box");
          fm_.add_include("thrift/annotation/thrift.thrift");
        }
      }
      if (name == "cpp.ref_type" || name == "cpp2.ref_type") {
        to_remove.emplace_back(name, data);
        if (is_field &&
            !node.find_annotation_or_null({"cpp.box", "thrift.box"}) &&
            !std::exchange(has_cpp_ref, true)) {
          if (data.value == "unique") {
            to_add.insert("@cpp.Ref{type = cpp.RefType.Unique}");
          } else if (data.value == "shared" || data.value == "shared_mutable") {
            to_add.insert("@cpp.Ref{type = cpp.RefType.SharedMutable}");
          } else if (data.value == "shared_const") {
            to_add.insert("@cpp.Ref{type = cpp.RefType.Shared}");
          }
          fm_.add_include("thrift/annotation/cpp.thrift");
        }
      }
      if (name == "cpp.ref" || name == "cpp2.ref") {
        to_remove.emplace_back(name, data);
        if (is_field &&
            !node.find_annotation_or_null(
                {"cpp.box", "thrift.box", "cpp.ref_type", "cpp2.ref_type"}) &&
            !std::exchange(has_cpp_ref, true)) {
          to_add.insert("@cpp.Ref{type = cpp.RefType.Unique}");
          fm_.add_include("thrift/annotation/cpp.thrift");
        }
      }

      // hack
      if (name == "hack.attributes") {
        const char* pos = sm_.get_text(data.src_range.begin);
        const char* end = sm_.get_text(data.src_range.end);
        for (; pos != end && *pos != '"' && *pos != '\''; ++pos) {
        }
        assert(pos != end);
        char delim = *pos;
        ++pos;
        for (; pos != end && isspace(*pos); ++pos) {
        }
        int depth = 0;
        std::vector<std::string> attrs;
        const char* start = pos;
        for (; pos != end; ++pos) {
          if (*pos == '(') {
            ++depth;
          } else if (*pos == ')') {
            --depth;
          } else if (
              (*pos == ',' && depth == 0) ||
              (*pos == delim && pos == end - 1)) {
            int trailing_spaces = 0;
            for (; pos - trailing_spaces - 1 > start &&
                 isspace(*(pos - trailing_spaces - 1));
                 ++trailing_spaces) {
            }
            attrs.push_back(fmt::format(
                "{}{}{}",
                delim,
                fmt::string_view(start, pos - start - trailing_spaces),
                delim));
            start = pos + 1;
            for (; start != end && isspace(*start); ++start) {
            }
          }
        }
        to_remove.emplace_back(name, data);
        to_add.insert(fmt::format(
            "@hack.Attributes{{attributes = [{}]}}", fmt::join(attrs, ", ")));
        fm_.add_include("thrift/annotation/hack.thrift");
      }

      // python
      if (name == "py3.hidden") {
        to_remove.emplace_back(name, data);
        to_add.insert("@python.Py3Hidden");
        fm_.add_include("thrift/annotation/python.thrift");
      }
      if (name == "py3.name") {
        to_remove.emplace_back(name, data);
        to_add.insert(fmt::format("@python.Name{{name = \"{}\"}}", data.value));
        fm_.add_include("thrift/annotation/python.thrift");
      }
      if (name == "py3.flags") {
        to_remove.emplace_back(name, data);
        if (dynamic_cast<const t_enum*>(&node)) {
          to_add.insert("@python.Flags");
          fm_.add_include("thrift/annotation/python.thrift");
        }
      }

      // java
      if (name == "java.swift.mutable") {
        to_remove.emplace_back(name, data);
        if (data.value == "true" &&
            (dynamic_cast<const t_struct*>(&node) &&
             !dynamic_cast<const t_union*>(&node))) {
          to_add.insert("@java.Mutable");
          fm_.add_include("thrift/annotation/java.thrift");
        }
      }
      if (name == "java.swift.annotations") {
        to_remove.emplace_back(name, data);
        to_add.insert(fmt::format(
            "@java.Annotation{{java_annotation = \"{}\"}}", data.value));
        fm_.add_include("thrift/annotation/java.thrift");
      }

      // go
      if (name == "go.name") {
        to_remove.emplace_back(name, data);
        to_add.insert(fmt::format("@go.Name{{name = \"{}\"}}", data.value));
        fm_.add_include("thrift/annotation/go.thrift");
      }
      if (name == "go.tag") {
        to_remove.emplace_back(name, data);
        to_add.insert(fmt::format("@go.Tag{{tag = '{}'}}", data.value));
        fm_.add_include("thrift/annotation/go.thrift");
      }

      // erlang
      if (name == "erl.name") {
        to_remove.emplace_back(name, data);
        to_add.insert(fmt::format(
            "@annotations.NameOverride{{name = \"{}\"}}", data.value));
        fm_.add_include("thrift/facebook/erlang/annotations.thrift");
      }
      if (name == "erl.struct_repr") {
        to_remove.emplace_back(name, data);
        to_add.insert(fmt::format(
            "@annotations.StructRepr{{repr = {}}}",
            data.value == "record" ? "annotations.StructReprType.RECORD"
                                   : "annotations.StructReprType.MAP"));
        fm_.add_include("thrift/facebook/erlang/annotations.thrift");
      }
      if (name == "erl.default_value") {
        to_remove.emplace_back(name, data);
        to_add.insert(fmt::format(
            "@annotations.DefaultValue{{value = \"{}\"}}", data.value));
        fm_.add_include("thrift/facebook/erlang/annotations.thrift");
      }
      if (name == "iq.node_type") {
        to_remove.emplace_back(name, data);
        to_add.insert(fmt::format(
            "@annotations.Iq{{node_type = {}}}",
            data.value == "xmlcdata" ? "annotations.IqNodeType.XMLCDATA"
                : data.value == "xmlnode"
                ? "annotations.IqNodeType.XMLNODE"
                : "annotations.IqNodeType.XMLATTRIBUTE"));
        fm_.add_include("thrift/facebook/erlang/annotations.thrift");
      }
    }

    if (!to_remove.empty() && to_remove.size() == node.annotations().size()) {
      fm_.remove_all_annotations(node);
    } else {
      for (const auto& annot : to_remove) {
        fm_.remove(annot);
      }
    }

    if (!to_remove.empty() && to_remove.size() == node.annotations().size()) {
      fm_.remove_all_annotations(node);
    } else {
      for (const auto& annot : to_remove) {
        fm_.remove(annot);
      }
    }

    if (!to_add.empty()) {
      fm_.add(
          {node.src_range().begin.offset(),
           node.src_range().begin.offset(),
           fmt::format("{}\n", fmt::join(to_add, "\n"))});
    }
  }

  void run() {
    const_ast_visitor visitor;
    visitor.add_definition_visitor([=](const auto& node) { visit_def(node); });
    visitor(prog_);

    fm_.apply_replacements();
  }

 private:
  codemod::file_manager fm_;
  source_manager sm_;
  t_program& prog_;
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
  structure_annotations(source_mgr, *program).run();

  return 0;
}
