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
#include <re2/re2.h>

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/generate/cpp/util.h>

using namespace apache::thrift::compiler;

namespace {
class hoist_annotated_types {
 public:
  hoist_annotated_types(source_manager& sm, t_program& program)
      : fm_(sm, program), sm_(sm), prog_(program) {}

  void run() {
    const_ast_visitor visitor;

    visitor.add_container_visitor(
        [=, this](const auto& type) { visit_type(type); });
    // Have to visit containers first to handle nesting correctly
    visitor(prog_);

    visitor = {};
    visitor.add_function_visitor(
        [=, this](const auto& f) { visit_function(f); });
    visitor.add_const_visitor([=, this](const auto& c) { visit_const(c); });
    visitor.add_field_visitor([=, this](const auto& f) { visit_field(f); });

    visitor(prog_);

    if (typedefs_.empty()) {
      return;
    }
    auto len = sm_.get_file(prog_.path())->text.size() - 1;
    std::vector<std::string> typedefs;
    for (const auto& [k, v] : typedefs_) {
      typedefs.push_back(
          fmt::format("{}typedef {} {}", v.structured, v.type, k));
    }
    fm_.add(
        {len,
         len,
         fmt::format(
             "\n// The following were automatically generated and may benefit from renaming.\n{}\n",
             fmt::join(typedefs, "\n"))});

    fm_.apply_replacements();
  }

  void visit_type(const t_container& type) {
    std::string replacement;

    switch (type.container_type()) {
      case t_container::type::t_list: {
        const auto& t = static_cast<const t_list&>(type);
        if (!needs_replacement(t.elem_type())) {
          return;
        }
        auto name = maybe_create_typedef(t.elem_type());
        replacement = fmt::format("list<{}>", name);
        // We modify the AST in case we're visiting a nested type, so that the
        // outer type will render correctly without us needing to propagate this
        // state.
        const_cast<t_type_ref&>(t.elem_type()) =
            t_type_ref::from_ptr(typedefs_.at(name).ptr);
        break;
      }
      case t_container::type::t_set: {
        const auto& t = static_cast<const t_set&>(type);
        if (!needs_replacement(t.elem_type())) {
          return;
        }
        auto name = maybe_create_typedef(t.elem_type());
        replacement = fmt::format("set<{}>", name);
        const_cast<t_type_ref&>(t.elem_type()) =
            t_type_ref::from_ptr(typedefs_.at(name).ptr);
        break;
      }
      case t_container::type::t_map: {
        const auto& t = static_cast<const t_map&>(type);
        if (!needs_replacement(t.key_type()) &&
            !needs_replacement(t.val_type())) {
          return;
        }
        auto name = maybe_create_typedef(t.key_type());
        if (auto it = typedefs_.find(name); it != typedefs_.end()) {
          const_cast<t_type_ref&>(t.key_type()) =
              t_type_ref::from_ptr(it->second.ptr);
        }
        name = maybe_create_typedef(t.val_type());
        if (auto it = typedefs_.find(name); it != typedefs_.end()) {
          const_cast<t_type_ref&>(t.val_type()) =
              t_type_ref::from_ptr(it->second.ptr);
        }
        replacement = fmt::format(
            "map<{}, {}>",
            maybe_create_typedef(t.key_type()),
            maybe_create_typedef(t.val_type()));
        break;
      }
      default:
        throw std::runtime_error("Unknown container type");
    }
    auto range = type.src_range();
    fm_.add({range.begin.offset(), range.end.offset(), replacement});
  }

  void visit_function(const t_function& f) {
    const auto& type = f.return_type();
    if (needs_replacement(type)) {
      auto range = f.src_range();
      auto annotations_end_offset = range.begin.offset();
      for (const auto& [k, v] : type.get_type()->unstructured_annotations()) {
        annotations_end_offset =
            std::max(annotations_end_offset, v.src_range.end.offset());
      }
      auto old_content = fm_.old_content();
      while (annotations_end_offset < old_content.size() &&
             old_content[annotations_end_offset++] != ')') {
      }
      fm_.add(
          {range.begin.offset(),
           annotations_end_offset,
           maybe_create_typedef(type)});
    }

    auto fix_by_hand = [&](const t_node& n) {
      // These are annoying to print and zbgs doesn't find any instances.
      auto range = n.src_range();
      fm_.add(
          {range.begin.offset(),
           range.end.offset(),
           fmt::format("{}nocommit must be fixed by hand", "@")});
    };

    if (const auto* t = dynamic_cast<const t_sink*>(f.sink_or_stream())) {
      if (needs_replacement(t->elem_type()) ||
          needs_replacement(t->final_response_type())) {
        maybe_create_typedef(t->elem_type());
        maybe_create_typedef(t->final_response_type());
        fix_by_hand(*t);
      }
    }
    if (const auto* t = dynamic_cast<const t_stream*>(f.sink_or_stream())) {
      if (needs_replacement(t->elem_type())) {
        maybe_create_typedef(t->elem_type());
        fix_by_hand(*t);
      }
    }

    for (auto& field : f.params().fields()) {
      visit_field(field);
    }
  }

  void visit_const(const t_const& c) {
    if (auto type = c.type_ref(); needs_replacement(type)) {
      auto range = c.src_range();
      auto annotations_end_offset = range.begin.offset();
      for (const auto& [k, v] : type.get_type()->unstructured_annotations()) {
        annotations_end_offset =
            std::max(annotations_end_offset, v.src_range.end.offset());
      }
      auto old_content = fm_.old_content();
      while (annotations_end_offset < old_content.size() &&
             old_content[annotations_end_offset++] != ')') {
      }
      fm_.add(
          {range.begin.offset(),
           annotations_end_offset,
           fmt::format("const {}", maybe_create_typedef(type))});
    }
  }

  void visit_field(const t_field& f) {
    if (auto type = f.type(); needs_replacement(type)) {
      auto range = f.src_range();
      auto type_begin_offset = range.begin.offset();
      auto old_content = fm_.old_content();
      while (type_begin_offset < range.end.offset() &&
             (!isdigit(old_content[type_begin_offset - 2]) ||
              old_content[type_begin_offset - 1] != ':' ||
              !isspace(old_content[type_begin_offset]))) {
        type_begin_offset++;
      }
      if (f.qualifier() == t_field_qualifier::optional) {
        while (type_begin_offset < range.end.offset() &&
               old_content[type_begin_offset - 1] != 'o') {
          type_begin_offset++;
        }
        while (type_begin_offset < range.end.offset() &&
               old_content[type_begin_offset - 1] != 'l') {
          type_begin_offset++;
        }
      }
      if (f.qualifier() == t_field_qualifier::required) {
        while (type_begin_offset < range.end.offset() &&
               old_content[type_begin_offset - 1] != 'r') {
          type_begin_offset++;
        }
        while (type_begin_offset < range.end.offset() &&
               old_content[type_begin_offset - 1] != 'd') {
          type_begin_offset++;
        }
      }
      auto type_end_offset = range.begin.offset();
      for (const auto& [k, v] : type.get_type()->unstructured_annotations()) {
        type_end_offset = std::max(type_end_offset, v.src_range.end.offset());
      }
      while (type_end_offset < old_content.size() &&
             old_content[type_end_offset++] != ')') {
      }
      // @cpp.Type interacts with some other annotations, so have to move it
      // when extracting a typedef.
      if (auto annot = f.find_structured_annotation_or_null(kCppTypeUri)) {
        // Store this structured annotation in the unstructured map, where
        // render_type separates it back out.
        auto begin = annot->src_range().begin.offset();
        auto end = annot->src_range().end.offset();
        const_cast<t_type&>(*type).set_unstructured_annotation(
            std::string(old_content.substr(begin, end - begin)));
        if (old_content[end] == '\n') {
          end++;
        }
        while (::isspace(old_content[begin - 1]) &&
               old_content[begin - 1] != '\n') {
          --begin;
        }
        fm_.add({begin, end, ""});
      }
      fm_.add(
          {type_begin_offset,
           type_end_offset,
           fmt::format(" {}", maybe_create_typedef(type))});
    }
  }

  bool needs_replacement(t_type_ref type) {
    auto ptr = type.get_type();
    if (ptr->unstructured_annotations().empty()) {
      return false;
    }
    if (dynamic_cast<const t_container*>(ptr)) {
      return true;
    }
    if (dynamic_cast<const t_primitive_type*>(ptr)) {
      return true;
    }
    if (auto t = dynamic_cast<const t_typedef*>(ptr)) {
      return t->typedef_kind() != t_typedef::kind::defined;
    }
    return false;
  }

  std::string maybe_create_typedef(t_type_ref type) {
    if (!needs_replacement(type)) {
      return render_type(type);
    }
    auto name = name_typedef(type);
    if (typedefs_.count(name)) {
      assert(typedefs_[name].type == render_type(type));
    } else if (auto existing = prog_.find<t_type>({name, type.src_range()})) {
      if (existing->get_true_type()->get_full_name() !=
          type->get_true_type()->get_full_name()) {
        throw std::runtime_error(fmt::format(
            "Type conflict: {} is already defined as {} but we need to define it as {}",
            name,
            existing->get_true_type()->get_full_name(),
            type->get_true_type()->get_full_name()));
      }
    } else {
      auto typedf = std::make_unique<t_typedef>(&prog_, name, type);
      std::string structured;
      for (const auto& [k, v] : type->unstructured_annotations()) {
        if (k[0] == '@') {
          structured = fmt::format("{}{}\n", structured, k);
        }
      }
      typedefs_[name] = {render_type(type), typedf.get(), structured};
      prog_.add_def(std::move(typedf));
    }
    return name;
  }

  std::string name_typedef(t_type_ref ref) {
    auto type = ref.get_type();
    std::vector<std::string> annotations;
    for (const auto& [k, v] : type->unstructured_annotations()) {
      annotations.push_back(fmt::format("{}_{}", k, v.value));
    }
    auto id = std::hash<std::string>()(fmt::format(
                  "{}_{}_{}",
                  type->get_full_name(),
                  fmt::join(annotations, "_"),
                  prog_.path())) %
        10000;
    auto name = type->get_full_name();
    // Removes scope prefix | ids of inner types.
    static const re2::RE2 stripNoise(R"((\b\w+?\.|_\d+\b))");
    static const re2::RE2 addUnderscores("[<,]");
    static const re2::RE2 stripNonAlnum("\\W+");
    re2::RE2::GlobalReplace(&name, stripNoise, "");
    re2::RE2::GlobalReplace(&name, addUnderscores, "_");
    re2::RE2::GlobalReplace(&name, stripNonAlnum, "");
    return fmt::format("{}_{}", name, id);
  }

  std::string render_type(t_type_ref ref) {
    auto type = ref.get_type();
    auto name = type->get_full_name();
    re2::RE2 prefix(fmt::format("\\b{}\\.", prog_.name()));
    re2::RE2::GlobalReplace(&name, prefix, "");
    if (!needs_replacement(ref)) {
      return name;
    }
    std::vector<std::string> annotations;
    std::string structured;
    for (const auto& [k, v] : type->unstructured_annotations()) {
      if (k[0] == '@') {
        // handled in maybe_create_typedef
        continue;
      }
      annotations.push_back(fmt::format("{} = \"{}\"", k, v.value));
    }
    assert(!annotations.empty());
    return fmt::format("{} ({})", name, fmt::join(annotations, ", "));
  }

 private:
  struct typedef_info {
    std::string type;
    t_typedef* ptr;
    std::string structured;
  };
  std::map<std::string, typedef_info> typedefs_;
  codemod::file_manager fm_;
  source_manager& sm_;
  t_program& prog_;
};
} // namespace

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& sm, t_program_bundle& pb) {
        hoist_annotated_types(sm, *pb.root_program()).run();
      });
}
