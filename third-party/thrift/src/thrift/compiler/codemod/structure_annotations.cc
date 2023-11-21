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
bool is_container(const t_type& type) {
  auto true_type = type.get_true_type();
  return true_type && true_type->is_container();
}

class structure_annotations {
 public:
  structure_annotations(source_manager& sm, t_program& program)
      : fm_(sm, program), sm_(sm), prog_(program) {}

  std::set<std::string> visit_type(t_type_ref typeRef, const t_named& node) {
    std::set<std::string> to_add;
    if (!typeRef.resolve() || typeRef->is_base_type() ||
        typeRef->is_container() ||
        (typeRef->is_typedef() &&
         static_cast<const t_typedef&>(*typeRef).typedef_kind() !=
             t_typedef::kind::defined)) {
      auto type = typeRef.get_type();
      std::vector<t_annotation> to_remove;
      bool has_cpp_type = node.find_structured_annotation_or_null(kCppTypeUri);
      for (const auto& [name, data] : type->annotations()) {
        // cpp type
        if (name == "cpp.template" || name == "cpp2.template") {
          to_remove.emplace_back(name, data);
          if (is_container(*type) && !std::exchange(has_cpp_type, true)) {
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
        if (name == "cpp.ref" || name == "cpp2.ref" || name == "cpp.ref_type" ||
            name == "cpp2.ref_type") {
          to_remove.emplace_back(name, data);
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
            is_container(
                *static_cast<const t_typedef&>(node).type().get_type()) &&
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
      if (name == "cpp.name") {
        to_remove.emplace_back(name, data);
        if (!node.find_structured_annotation_or_null(kCppNameUri)) {
          to_add.insert(fmt::format("@cpp.Name{{value = \"{}\"}}", data.value));
          fm_.add_include("thrift/annotation/cpp.thrift");
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

      // thrift
      if (name == "priority") {
        if (!dynamic_cast<const t_function*>(&node)) {
          // pydeprecated consumes this annotation on services as well
          continue;
        }
        to_remove.emplace_back(name, data);
        if (!node.find_structured_annotation_or_null(kPriorityUri) &&
            std::set<std::string>({"HIGH_IMPORTANT",
                                   "HIGH",
                                   "IMPORTANT",
                                   "NORMAL",
                                   "BEST_EFFORT"})
                .contains(data.value)) {
          to_add.insert(fmt::format(
              "@thrift.Priority{{level = thrift.RpcPriority.{}}}", data.value));
          fm_.add_include("thrift/annotation/thrift.thrift");
        }
      }
      if (name == "serial") {
        to_remove.emplace_back(name, data);
        if (!node.find_structured_annotation_or_null(kSerialUri)) {
          to_add.insert("@thrift.Serial");
          fm_.add_include("thrift/annotation/thrift.thrift");
        }
      }

      // cpp
      if (name == "cpp.coroutine") {
        to_remove.emplace_back(name, data);
      }
      if (name == "cpp.minimize_padding") {
        to_remove.emplace_back(name, data);
        if (!node.find_structured_annotation_or_null(kCppMinimizePaddingUri)) {
          to_add.insert("@cpp.MinimizePadding");
          fm_.add_include("thrift/annotation/cpp.thrift");
        }
      }
      if (name == "cpp.enum_type" || name == "cpp2.enum_type") {
        to_remove.emplace_back(name, data);
        if (!node.find_structured_annotation_or_null(kCppEnumTypeUri)) {
          std::string_view value = data.value;
          if (value.substr(0, 2) == "::") {
            value = value.substr(2);
          }
          if (value.substr(0, 5) == "std::") {
            value = value.substr(5);
          }
          std::string type;
          if (value == "int8_t" || value == "char") {
            type = "I8";
          } else if (value == "uint8_t" || value == "unsigned char") {
            type = "U8";
          } else if (value == "int16_t" || value == "short") {
            type = "I16";
          } else if (value == "uint16_t" || value == "unsigned short") {
            type = "U16";
          } else if (
              value == "uint32_t" || value == "unsigned long" ||
              value == "unsigned" || value == "uint") {
            type = "U32";
          } else {
            continue;
          }
          to_add.insert(fmt::format(
              "@cpp.EnumType{{type = cpp.EnumUnderlyingType.{}}}", type));
          fm_.add_include("thrift/annotation/cpp.thrift");
        }
      }
      if (name == "cpp.declare_bitwise_ops" ||
          name == "cpp2.declare_bitwise_ops") {
        // This annotation is a subset of @thrift.BitmaskEnum, which carries
        // additional restrictions. We can't turn this into that but we can use
        // it if already present to remove this.
        if (node.find_structured_annotation_or_null(kBitmaskEnumUri) ||
            node.has_annotation("bitmask")) {
          to_remove.emplace_back(name, data);
        }
      }
      if (name == "cpp.mixin") {
        to_remove.emplace_back(name, data);
        if (!node.find_structured_annotation_or_null(kMixinUri)) {
          to_add.insert("@thrift.Mixin");
          fm_.add_include("thrift/annotation/thrift.thrift");
        }
      }
      if (name == "cpp.experimental.lazy") {
        to_remove.emplace_back(name, data);
        if (!node.find_structured_annotation_or_null(kCppLazyUri)) {
          to_add.insert("@cpp.Lazy");
          fm_.add_include("thrift/annotation/cpp.thrift");
        }
      }
      if (name == "thread" || name == "process_in_event_base") {
        to_remove.emplace_back(name, data);
        if (!node.find_structured_annotation_or_null(
                kCppProcessInEbThreadUri) &&
            (name != "thread" || data.value == "eb")) {
          to_add.insert("@cpp.ProcessInEbThreadUnsafe");
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
        if (!node.find_structured_annotation_or_null(kHackAttributeUri)) {
          to_add.insert(fmt::format(
              "@hack.Attributes{{attributes = [{}]}}", fmt::join(attrs, ", ")));
          fm_.add_include("thrift/annotation/hack.thrift");
        }
      }
      if (name == "hack.name") {
        to_remove.emplace_back(name, data);
        to_add.insert(fmt::format("@hack.Name{{name = \"{}\"}}", data.value));
        fm_.add_include("thrift/annotation/hack.thrift");
      }
      if (name == "code") {
        to_remove.emplace_back(name, data);
      }
      if (name == "message") {
        to_remove.emplace_back(name, data);
        if (auto field =
                dynamic_cast<const t_structured&>(node).get_field_by_name(
                    data.value);
            field &&
            !node.find_structured_annotation_or_null(kExceptionMessageUri)) {
          fm_.add(
              {field->src_range().begin.offset(),
               field->src_range().begin.offset(),
               "@thrift.ExceptionMessage\n"});
          fm_.add_include("thrift/annotation/thrift.thrift");
        }
      }
      if (name == "bitmask") {
        to_remove.emplace_back(name, data);
        if (!node.find_structured_annotation_or_null(kBitmaskEnumUri)) {
          to_add.insert("@thrift.BitmaskEnum");
          fm_.add_include("thrift/annotation/thrift.thrift");
        }
      }

      // python
      if (name == "py3.hidden") {
        to_remove.emplace_back(name, data);
        if (!node.find_structured_annotation_or_null(kPythonPy3HiddenUri)) {
          to_add.insert("@python.Py3Hidden");
          fm_.add_include("thrift/annotation/python.thrift");
        }
      }
      if (name == "py3.name") {
        to_remove.emplace_back(name, data);
        if (!node.find_structured_annotation_or_null(kPythonNameUri)) {
          to_add.insert(
              fmt::format("@python.Name{{name = \"{}\"}}", data.value));
          fm_.add_include("thrift/annotation/python.thrift");
        }
      }
      if (name == "py3.flags") {
        to_remove.emplace_back(name, data);
        if (dynamic_cast<const t_enum*>(&node) &&
            !node.find_structured_annotation_or_null(kPythonFlagsUri)) {
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
