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

#include <string>
#include <string_view>
#include <fmt/core.h>
#include <range/v3/view.hpp>

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/generate/cpp/util.h>

using namespace apache::thrift::compiler;

namespace {
bool is_container(const t_type& type) {
  auto true_type = type.get_true_type();
  return true_type && true_type->is<t_container>();
}

std::string quote(std::string_view str) {
  if (str.find('"') == std::string::npos) {
    return fmt::format("\"{}\"", str);
  } else if (str.find('\'') == std::string::npos) {
    return fmt::format("'{}'", str);
  } else {
    std::string out(str);
    size_t start_pos = 0;
    while ((start_pos = out.find('"', start_pos)) != std::string::npos) {
      out.replace(start_pos, 1, "\\\"");
      start_pos += 2;
    }
    return fmt::format("\"{}\"", out);
  }
}

class structure_annotations {
 public:
  structure_annotations(source_manager& sm, t_program& program)
      : fm_(sm, program), sm_(sm), prog_(program) {}

  // if annotations_for_catch_all is non-null, type annotations will be removed
  // and added to that map. (This only makes sense for typedefs).
  std::set<std::string> visit_type(
      t_type_ref type_ref,
      const t_named& node,
      std::map<std::string, std::string>* annotations_for_catch_all) {
    std::set<std::string> to_add;
    if (!type_ref.resolve() || type_ref->is<t_primitive_type>() ||
        type_ref->is<t_container>() ||
        (type_ref->is<t_typedef>() &&
         static_cast<const t_typedef&>(*type_ref).typedef_kind() !=
             t_typedef::kind::defined)) {
      auto type = type_ref.get_type();
      std::vector<t_annotation> to_remove;
      bool has_cpp_type = node.has_structured_annotation(kCppTypeUri);
      for (const auto& [name, data] : type->unstructured_annotations()) {
        // cpp type
        if (name == "cpp.template" || name == "cpp2.template") {
          to_remove.emplace_back(name, data);
          if (is_container(*type) && !std::exchange(has_cpp_type, true)) {
            to_add.insert(
                fmt::format("@cpp.Type{{template = \"{}\"}}", data.value));
            fm_.add_include("thrift/annotation/cpp.thrift");
          }
        } else if (name == "cpp.type" || name == "cpp2.type") {
          to_remove.emplace_back(name, data);
          if (!std::exchange(has_cpp_type, true)) {
            to_add.insert(
                fmt::format("@cpp.Type{{name = \"{}\"}}", data.value));
            fm_.add_include("thrift/annotation/cpp.thrift");
          }
        } else if (
            name == "cpp.ref" || name == "cpp2.ref" || name == "cpp.ref_type" ||
            name == "cpp2.ref_type") {
          to_remove.emplace_back(name, data);
        }

        // rust type
        else if (name == "rust.type") {
          to_remove.emplace_back(name, data);
          to_add.insert(fmt::format("@rust.Type{{name = \"{}\"}}", data.value));
          fm_.add_include("thrift/annotation/rust.thrift");
        }

        // haskell annotations are ignored by the main compiler
        else if (name.find("hs.") == 0) {
        }

        // catch-all (if typedef)
        else if (annotations_for_catch_all) {
          to_remove.emplace_back(name, data);
          annotations_for_catch_all->emplace(name, data.value);
        }
      }

      if (!to_remove.empty() &&
          to_remove.size() == type->unstructured_annotations().size()) {
        fm_.remove_all_annotations(*type);
      } else {
        for (const auto& annot : to_remove) {
          fm_.remove(annot);
        }
      }
    }

    return to_add;
  }

  void visit_unstructured_annotation(
      const t_named& node,
      const std::string& name,
      const deprecated_annotation_value& data,
      std::set<std::string>& to_add,
      std::vector<t_annotation>& to_remove,
      std::map<std::string, std::string>& annotations_for_catch_all,
      bool& has_cpp_type,
      bool& has_cpp_ref) {
    // cpp type
    bool is_typedef = dynamic_cast<const t_typedef*>(&node);
    bool is_field = dynamic_cast<const t_field*>(&node);
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
    } else if (name == "cpp.type" || name == "cpp2.type") {
      to_remove.emplace_back(name, data);
      if (is_typedef && !std::exchange(has_cpp_type, true)) {
        to_add.insert(fmt::format("@cpp.Type{{name = \"{}\"}}", data.value));
        fm_.add_include("thrift/annotation/cpp.thrift");
      }
    }

    // cpp ref
    else if (name == "thrift.box") {
      to_remove.emplace_back(name, data);
      if (is_field && !std::exchange(has_cpp_ref, true)) {
        to_add.insert("@thrift.Box");
        fm_.add_include("thrift/annotation/thrift.thrift");
      }
    } else if (name == "cpp.name") {
      to_remove.emplace_back(name, data);
      if (!node.has_structured_annotation(kCppNameUri)) {
        to_add.insert(fmt::format("@cpp.Name{{value = \"{}\"}}", data.value));
        fm_.add_include("thrift/annotation/cpp.thrift");
      }
    } else if (name == "cpp.ref_type" || name == "cpp2.ref_type") {
      to_remove.emplace_back(name, data);
      if (is_field &&
          !node.find_unstructured_annotation_or_null(
              {"cpp.box", "thrift.box"}) &&
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
    } else if (name == "cpp.ref" || name == "cpp2.ref") {
      to_remove.emplace_back(name, data);
      if (is_field &&
          !node.find_unstructured_annotation_or_null(
              {"cpp.box", "thrift.box", "cpp.ref_type", "cpp2.ref_type"}) &&
          !std::exchange(has_cpp_ref, true)) {
        to_add.insert("@cpp.Ref{type = cpp.RefType.Unique}");
        fm_.add_include("thrift/annotation/cpp.thrift");
      }
    }

    // thrift
    else if (name == "priority") {
      if (!dynamic_cast<const t_function*>(&node)) {
        // pydeprecated consumes this annotation on services as well
        return;
      }
      to_remove.emplace_back(name, data);
      if (!node.has_structured_annotation(kPriorityUri) &&
          std::set<std::string>(
              {"HIGH_IMPORTANT", "HIGH", "IMPORTANT", "NORMAL", "BEST_EFFORT"})
              .contains(data.value)) {
        to_add.insert(
            fmt::format(
                "@thrift.Priority{{level = thrift.RpcPriority.{}}}",
                data.value));
        fm_.add_include("thrift/annotation/thrift.thrift");
      }
    } else if (name == "serial") {
      to_remove.emplace_back(name, data);
      if (!node.has_structured_annotation(kSerialUri)) {
        to_add.insert("@thrift.Serial");
        fm_.add_include("thrift/annotation/thrift.thrift");
      }
    } else if (name == "thrift.uri") {
      to_remove.emplace_back(name, data);
      if (!node.has_structured_annotation(kUriUri)) {
        to_add.insert(fmt::format("@thrift.Uri{{value = \"{}\"}}", data.value));
        fm_.add_include("thrift/annotation/thrift.thrift");
      }
    }

    // cpp
    else if (name == "cpp.coroutine") {
      to_remove.emplace_back(name, data);
    } else if (name == "cpp.minimize_padding") {
      to_remove.emplace_back(name, data);
      if (!node.has_structured_annotation(kCppMinimizePaddingUri)) {
        to_add.insert("@cpp.MinimizePadding");
        fm_.add_include("thrift/annotation/cpp.thrift");
      }
    } else if (
        name == "cpp.declare_bitwise_ops" &&
        (node.has_structured_annotation(kBitmaskEnumUri) ||
         node.has_unstructured_annotation("bitmask"))) {
      // This annotation is a subset of @thrift.BitmaskEnum, which carries
      // additional restrictions. We can't turn this into that but we can use
      // it if already present to remove this.
      to_remove.emplace_back(name, data);
    } else if (name == "cpp.mixin") {
      to_remove.emplace_back(name, data);
      if (!node.has_structured_annotation(kMixinUri)) {
        to_add.insert("@thrift.Mixin");
        fm_.add_include("thrift/annotation/thrift.thrift");
      }
    } else if (name == "cpp.experimental.lazy") {
      to_remove.emplace_back(name, data);
      if (!node.has_structured_annotation(kCppLazyUri)) {
        to_add.insert("@cpp.Lazy");
        fm_.add_include("thrift/annotation/cpp.thrift");
      }
    } else if (name == "thread" || name == "process_in_event_base") {
      to_remove.emplace_back(name, data);
      if (!node.has_structured_annotation(kCppProcessInEbThreadUri) &&
          (name != "thread" || data.value == "eb")) {
        to_add.insert("@cpp.ProcessInEbThreadUnsafe");
        fm_.add_include("thrift/annotation/cpp.thrift");
      }
    } else if (name == "cpp.declare_hash" || name == "cpp2.declare_hash") {
      to_remove.emplace_back(name, data);
      if (!node.has_structured_annotation(kCppDeclareHashSpecialization)) {
        to_add.insert("@cpp.DeclareHashSpecialization");
        fm_.add_include("thrift/annotation/cpp.thrift");
      }
    } else if (
        name == "cpp.declare_equal_to" || name == "cpp2.declare_equal_to") {
      to_remove.emplace_back(name, data);
      if (!node.has_structured_annotation(kCppDeclareEqualToSpecialization)) {
        to_add.insert("@cpp.DeclareEqualToSpecialization");
        fm_.add_include("thrift/annotation/cpp.thrift");
      }
    }

    // hack
    else if (name == "hack.attributes") {
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
            (*pos == ',' && depth == 0) || (*pos == delim && pos == end - 1)) {
          int trailing_spaces = 0;
          for (; pos - trailing_spaces - 1 > start &&
               isspace(*(pos - trailing_spaces - 1));
               ++trailing_spaces) {
          }
          attrs.push_back(
              fmt::format(
                  "{}{}{}",
                  delim,
                  std::string_view(start, pos - start - trailing_spaces),
                  delim));
          start = pos + 1;
          for (; start != end && isspace(*start); ++start) {
          }
        }
      }
      to_remove.emplace_back(name, data);
      if (!node.has_structured_annotation(kHackAttributeUri)) {
        to_add.insert(
            fmt::format(
                "@hack.Attributes{{attributes = [{}]}}",
                fmt::join(attrs, ", ")));
        fm_.add_include("thrift/annotation/hack.thrift");
      }
    } else if (name == "hack.name") {
      to_remove.emplace_back(name, data);
      to_add.insert(fmt::format("@hack.Name{{name = \"{}\"}}", data.value));
      fm_.add_include("thrift/annotation/hack.thrift");
    } else if (name == "code") {
      to_remove.emplace_back(name, data);
    } else if (name == "message") {
      to_remove.emplace_back(name, data);
      if (auto field =
              dynamic_cast<const t_structured&>(node).get_field_by_name(
                  data.value);
          field && !node.has_structured_annotation(kExceptionMessageUri)) {
        fm_.add(
            {field->src_range().begin.offset(),
             field->src_range().begin.offset(),
             "@thrift.ExceptionMessage\n"});
        fm_.add_include("thrift/annotation/thrift.thrift");
      }
    } else if (name == "bitmask") {
      to_remove.emplace_back(name, data);
      if (!node.has_structured_annotation(kBitmaskEnumUri)) {
        to_add.insert("@thrift.BitmaskEnum");
        fm_.add_include("thrift/annotation/thrift.thrift");
      }
    }

    // python
    else if (name == "py3.hidden") {
      to_remove.emplace_back(name, data);
      if (!node.has_structured_annotation(kPythonPy3HiddenUri)) {
        to_add.insert("@python.Py3Hidden");
        fm_.add_include("thrift/annotation/python.thrift");
      }
    } else if (name == "py3.name") {
      to_remove.emplace_back(name, data);
      if (!node.has_structured_annotation(kPythonNameUri)) {
        to_add.insert(fmt::format("@python.Name{{name = \"{}\"}}", data.value));
        fm_.add_include("thrift/annotation/python.thrift");
      }
    } else if (name == "py3.flags") {
      to_remove.emplace_back(name, data);
      if (dynamic_cast<const t_enum*>(&node) &&
          !node.has_structured_annotation(kPythonFlagsUri)) {
        to_add.insert("@python.Flags");
        fm_.add_include("thrift/annotation/python.thrift");
      }
    }

    // java
    else if (name == "java.swift.mutable") {
      to_remove.emplace_back(name, data);
      if (data.value == "true" &&
          (dynamic_cast<const t_struct*>(&node) &&
           !dynamic_cast<const t_union*>(&node))) {
        to_add.insert("@java.Mutable");
        fm_.add_include("thrift/annotation/java.thrift");
      }
    } else if (name == "java.swift.annotations") {
      to_remove.emplace_back(name, data);
      to_add.insert(
          fmt::format(
              "@java.Annotation{{java_annotation = \"{}\"}}", data.value));
      fm_.add_include("thrift/annotation/java.thrift");
    } else if (name == "swift.recursive_reference") {
      to_remove.emplace_back(name, data);
      if (data.value == "true" && dynamic_cast<const t_field*>(&node) &&
          !node.has_structured_annotation(kJavaRecursiveUri)) {
        to_add.insert("@java.Recursive");
        fm_.add_include("thrift/annotation/java.thrift");
      }
    }

    // go
    else if (name == "go.name") {
      to_remove.emplace_back(name, data);
      to_add.insert(fmt::format("@go.Name{{name = \"{}\"}}", data.value));
      fm_.add_include("thrift/annotation/go.thrift");
    } else if (name == "go.tag") {
      to_remove.emplace_back(name, data);
      to_add.insert(fmt::format("@go.Tag{{tag = '{}'}}", data.value));
      fm_.add_include("thrift/annotation/go.thrift");
    }

    // erlang
    else if (name == "erl.name") {
      to_remove.emplace_back(name, data);
      to_add.insert(
          fmt::format("@erlang.NameOverride{{name = \"{}\"}}", data.value));
      fm_.add_include("thrift/annotation/erlang.thrift");
    } else if (name == "erl.struct_repr") {
      to_remove.emplace_back(name, data);
      to_add.insert(
          fmt::format(
              "@erlang.StructRepr{{repr = {}}}",
              data.value == "record" ? "erlang.StructReprType.RECORD"
                                     : "erlang.StructReprType.MAP"));
      fm_.add_include("thrift/annotation/erlang.thrift");
    } else if (name == "erl.default_value") {
      to_remove.emplace_back(name, data);
      to_add.insert(
          fmt::format("@erlang.DefaultValue{{value = \"{}\"}}", data.value));
      fm_.add_include("thrift/annotation/erlang.thrift");
    } else if (name == "iq.node_type") {
      to_remove.emplace_back(name, data);
      to_add.insert(
          fmt::format(
              "@erlang.Iq{{node_type = {}}}",
              data.value == "xmlcdata" ? "erlang.IqNodeType.XMLCDATA"
                  : data.value == "xmlnode"
                  ? "erlang.IqNodeType.XMLNODE"
                  : "erlang.IqNodeType.XMLATTRIBUTE"));
      fm_.add_include("thrift/annotation/erlang.thrift");
    }

    // rust
    else if (name == "rust.name") {
      to_remove.emplace_back(name, data);
      to_add.insert(fmt::format("@rust.Name{{name = \"{}\"}}", data.value));
      fm_.add_include("thrift/annotation/rust.thrift");
    } else if (name == "rust.copy") {
      to_remove.emplace_back(name, data);
      if (!dynamic_cast<const t_enum*>(&node)) {
        // enums automatically implement Copy
        to_add.insert("@rust.Copy");
        fm_.add_include("thrift/annotation/rust.thrift");
      }
    } else if (name == "rust.arc") {
      to_remove.emplace_back(name, data);
      to_add.insert("@rust.Arc");
      fm_.add_include("thrift/annotation/rust.thrift");
    } else if (name == "rust.box") {
      to_remove.emplace_back(name, data);
      to_add.insert("@rust.Box");
      fm_.add_include("thrift/annotation/rust.thrift");
    } else if (name == "rust.exhaustive") {
      to_remove.emplace_back(name, data);
      if (!dynamic_cast<const t_union*>(&node) &&
          !dynamic_cast<const t_enum*>(&node)) {
        // annotation is no-op on unions and enums
        to_add.insert("@rust.Exhaustive");
        fm_.add_include("thrift/annotation/rust.thrift");
      }
    } else if (name == "rust.ord") {
      to_remove.emplace_back(name, data);
      if (!dynamic_cast<const t_enum*>(&node)) {
        // enums automatically implement Ord
        to_add.insert("@rust.Ord");
        fm_.add_include("thrift/annotation/rust.thrift");
      }
    } else if (name == "rust.newtype") {
      to_remove.emplace_back(name, data);
      to_add.insert("@rust.NewType");
      fm_.add_include("thrift/annotation/rust.thrift");
    } else if (name == "rust.type") {
      to_remove.emplace_back(name, data);
      if (dynamic_cast<const t_typedef*>(&node)) {
        to_add.insert(fmt::format("@rust.Type{{name = \"{}\"}}", data.value));
        fm_.add_include("thrift/annotation/rust.thrift");
      }
    } else if (name == "rust.serde") {
      to_remove.emplace_back(name, data);
      if (data.value == "false") {
        to_add.insert("@rust.Serde{enabled = false}");
      } else {
        to_add.insert("@rust.Serde");
      }
      fm_.add_include("thrift/annotation/rust.thrift");
    } else if (name == "rust.mod") {
      to_remove.emplace_back(name, data);
      to_add.insert(fmt::format("@rust.Mod{{name = \"{}\"}}", data.value));
      fm_.add_include("thrift/annotation/rust.thrift");
    } else if (name == "rust.derive") {
      to_remove.emplace_back(name, data);
      auto derives =
          data.value | ranges::views::split(',') |
          ranges::views::transform([](auto s) {
            auto str = s | ranges::to<std::string>;
            str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
            return fmt::format("\"{}\"", str);
          }) |
          ranges::views::join(',') | ranges::to<std::string>;
      to_add.insert(fmt::format("@rust.Derive{{derives = [{}]}}", derives));
      fm_.add_include("thrift/annotation/rust.thrift");
    } else if (name == "rust.request_context") {
      to_remove.emplace_back(name, data);
      to_add.insert("@rust.RequestContext");
      fm_.add_include("thrift/annotation/rust.thrift");
    }

    // haskell annotations are ignored by the main compiler
    else if (name.find("hs.") == 0) {
    }

    // catch-all
    else {
      to_remove.emplace_back(name, data);
      annotations_for_catch_all.emplace(name, data.value);
    }
  }

  // Insert, replace or remove @thrift.DeprecatedUnvalidatedAnnotations
  // annotation based on initial and target content
  void update_deprecated_unvalidated_annotations(
      const t_named& node,
      const std::map<std::string, std::string>& annotations_for_catch_all,
      std::set<std::string>& to_add) {
    // Existing @thrift.DeprecatedUnvalidatedAnnotations annotation
    const auto deprecated_unvalidated = node.find_structured_annotation_or_null(
        kDeprecatedUnvalidatedAnnotationsUri);
    if (annotations_for_catch_all.empty()) {
      // No annotations to add - remove existing
      // @thrift.DeprecatedUnvalidatedAnnotations if present
      if (deprecated_unvalidated != nullptr) {
        fm_.add(
            {deprecated_unvalidated->src_range().begin.offset(),
             deprecated_unvalidated->src_range().end.offset(),
             ""});
      }

      return;
    }

    // Items content of existing @thrift.DeprecatedUnvalidatedAnnotations
    std::map<std::string, std::string> original_unstructured_items;
    const auto unstructured_items = deprecated_unvalidated
        ? deprecated_unvalidated->get_value_from_structured_annotation_or_null(
              "items")
        : nullptr;

    if (unstructured_items != nullptr &&
        !unstructured_items->get_map().empty()) {
      for (const auto& [name, value] : unstructured_items->get_map()) {
        original_unstructured_items.emplace(
            name->get_string(), value->get_string());
      }
    }

    if (unstructured_items != nullptr &&
        original_unstructured_items == annotations_for_catch_all) {
      // Existing @thrift.DeprecatedUnvalidatedAnnotations matches
      // annotations_for_catch_all - no need to update
      return;
    }

    if (deprecated_unvalidated != nullptr) {
      // Remove an existing deprecated annotation before replacement
      fm_.add(
          {deprecated_unvalidated->src_range().begin.offset(),
           deprecated_unvalidated->src_range().end.offset(),
           ""});
    }

    std::vector<std::string> annotations_for_catch_all_strs;
    annotations_for_catch_all_strs.reserve(annotations_for_catch_all.size());
    for (const auto& [name, value] : annotations_for_catch_all) {
      annotations_for_catch_all_strs.push_back(
          fmt::format(R"("{}": {})", name, quote(value)));
    }

    to_add.insert(
        fmt::format(
            "@thrift.DeprecatedUnvalidatedAnnotations{{items = {{{}}}}}",
            fmt::join(annotations_for_catch_all_strs, ", ")));
    fm_.add_include("thrift/annotation/thrift.thrift");
  }

  void visit_def(const t_named& node) {
    std::map<std::string, std::string> annotations_for_catch_all;
    std::set<std::string> to_add;
    if (auto typedf = dynamic_cast<const t_typedef*>(&node)) {
      to_add = visit_type(typedf->type(), node, &annotations_for_catch_all);
    } else if (auto field = dynamic_cast<const t_field*>(&node)) {
      to_add = visit_type(field->type(), node, nullptr);
    }

    std::vector<t_annotation> to_remove;
    bool has_cpp_type = node.has_structured_annotation(kCppTypeUri);
    bool has_cpp_ref = node.has_structured_annotation(kBoxUri) ||
        node.has_structured_annotation(kInternBoxUri) ||
        node.has_structured_annotation(kCppRefUri);

    // Replace existing `@thrift.DeprecatedUnvalidatedAnnotations` items with
    // structured equivalents where possible
    const auto deprecated_unvalidated = node.find_structured_annotation_or_null(
        kDeprecatedUnvalidatedAnnotationsUri);
    const auto unstructured_items = deprecated_unvalidated
        ? deprecated_unvalidated->get_value_from_structured_annotation_or_null(
              "items")
        : nullptr;

    if (unstructured_items && !unstructured_items->get_map().empty()) {
      for (const auto& [name, data] : unstructured_items->get_map()) {
        visit_unstructured_annotation(
            node,
            name->get_string(),
            deprecated_annotation_value{
                {},
                data->get_string(),
                {},
            },
            to_add,
            to_remove,
            annotations_for_catch_all,
            has_cpp_type,
            has_cpp_ref);
      }
    }

    for (const auto& [name, data] : node.unstructured_annotations()) {
      visit_unstructured_annotation(
          node,
          name,
          data,
          to_add,
          to_remove,
          annotations_for_catch_all,
          has_cpp_type,
          has_cpp_ref);
    }

    if (!to_remove.empty() &&
        to_remove.size() == node.unstructured_annotations().size()) {
      fm_.remove_all_annotations(node);
    } else {
      for (const auto& annot : to_remove) {
        fm_.remove(annot);
      }
    }

    update_deprecated_unvalidated_annotations(
        node, annotations_for_catch_all, to_add);

    if (!to_add.empty()) {
      fm_.add(
          {node.src_range().begin.offset(),
           node.src_range().begin.offset(),
           fmt::format("{}\n", fmt::join(to_add, "\n"))});
    }
  }

  void run() {
    const_ast_visitor visitor;
    visitor.add_named_visitor([=, this](const auto& node) { visit_def(node); });
    visitor.add_function_visitor([=, this](const t_function& node) {
      for (const t_field& param : node.params().fields()) {
        visit_def(param);
      }
    });
    visitor(prog_);

    fm_.apply_replacements();
  }

 private:
  codemod::file_manager fm_;
  source_manager& sm_;
  t_program& prog_;
};
} // namespace

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& sm, t_program_bundle& pb) {
        structure_annotations(sm, *pb.root_program()).run();
      });
}
