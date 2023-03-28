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

using namespace apache::thrift::compiler;

namespace {
class hoist_annotated_types {
 public:
  hoist_annotated_types(source_manager& sm, t_program& program)
      : fm_(sm, program), sm_(sm), prog_(program) {}

  void run() {
    const_ast_visitor visitor;

    visitor.add_type_instantiation_visitor(
        [=](const auto& type) { visit_type(type); });
    visitor.add_function_visitor([=](const auto& f) { visit_function(f); });
    visitor.add_const_visitor([=](const auto& c) { visit_const(c); });

    visitor(prog_);

    if (typedefs_.empty()) {
      return;
    }
    auto len = sm_.get_file(prog_.path()).text.size() - 1;
    std::vector<std::string> typedefs;
    for (const auto& [k, v] : typedefs_) {
      typedefs.push_back(fmt::format("typedef {} {}", v, k));
    }
    fm_.add(
        {len,
         len,
         fmt::format(
             "\n// The following were automatically generated and may benefit from renaming.\n{}\n",
             fmt::join(typedefs, "\n"))});

    fm_.apply_replacements();
  }

  void visit_type(const t_templated_type& type) {
    std::string replacement;

    switch (type.get_type_value()) {
      case t_type::type::t_list: {
        const auto& t = static_cast<const t_list&>(type);
        if (!needs_replacement(t.elem_type())) {
          return;
        }
        replacement =
            fmt::format("list<{}>", maybe_create_typedef(t.elem_type()));
        break;
      }
      case t_type::type::t_set: {
        const auto& t = static_cast<const t_set&>(type);
        if (!needs_replacement(t.elem_type())) {
          return;
        }
        replacement =
            fmt::format("set<{}>", maybe_create_typedef(t.elem_type()));
        break;
      }
      case t_type::type::t_map: {
        const auto& t = static_cast<const t_map&>(type);
        if (!needs_replacement(t.key_type()) &&
            !needs_replacement(t.val_type())) {
          return;
        }
        replacement = fmt::format(
            "map<{}, {}>",
            maybe_create_typedef(t.key_type()),
            maybe_create_typedef(t.val_type()));
        break;
      }
      case t_type::type::t_stream: {
        const auto& t = static_cast<const t_stream_response&>(type);
        if (!needs_replacement(t.elem_type())) {
          return;
        }
        maybe_create_typedef(t.elem_type());
        // These are annoying to print and zbgs doesn't find any instances.
        replacement = fmt::format("{}nocommit must be fixed by hand", "@");
        break;
      }
      case t_type::type::t_sink: {
        const auto& t = static_cast<const t_sink&>(type);
        if (!needs_replacement(t.sink_type()) &&
            !needs_replacement(t.final_response_type())) {
          return;
        }
        maybe_create_typedef(t.sink_type());
        maybe_create_typedef(t.final_response_type());
        // These are annoying to print and zbgs doesn't find any instances.
        replacement = fmt::format("{}nocommit must be fixed by hand", "@");
        break;
      }
      default:
        throw std::runtime_error("Unknown templated type");
    }
    auto range = type.src_range();
    fm_.add({range.begin.offset(), range.end.offset(), replacement});
  }

  void visit_function(const t_function& f) {
    for (const auto& type : f.return_types()) {
      if (needs_replacement(type)) {
        auto range = f.src_range();
        auto annotations_end_offset = range.begin.offset();
        for (const auto& [k, v] : type.get_type()->annotations()) {
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
    }

    for (auto& field : f.params().fields()) {
      if (auto type = field.type(); needs_replacement(type)) {
        auto range = f.src_range();
        auto type_begin_offset = range.end.offset();
        auto type_end_offset = range.begin.offset();
        for (const auto& [k, v] : type.get_type()->annotations()) {
          type_begin_offset =
              std::min(type_begin_offset, v.src_range.begin.offset());
          type_end_offset = std::max(type_end_offset, v.src_range.end.offset());
        }
        auto old_content = fm_.old_content();
        while (type_begin_offset > 0 &&
               old_content[type_begin_offset - 1] != ':') {
          type_begin_offset--;
        }
        while (type_end_offset < old_content.size() &&
               old_content[type_end_offset++] != ')') {
        }
        fm_.add(
            {type_begin_offset, type_end_offset, maybe_create_typedef(type)});
      }
    }
  }

  void visit_const(const t_const& c) {
    if (auto type = c.type(); needs_replacement(type)) {
      auto range = c.src_range();
      auto annotations_end_offset = range.begin.offset();
      for (const auto& [k, v] : type.get_type()->annotations()) {
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

  bool needs_replacement(t_type_ref type) {
    auto ptr = type.get_type();
    if (ptr->annotations().empty()) {
      return false;
    }
    if (dynamic_cast<const t_templated_type*>(ptr)) {
      return true;
    }
    if (dynamic_cast<const t_base_type*>(ptr)) {
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
      assert(typedefs_[name] == render_type(type));
    } else {
      typedefs_[name] = render_type(type);
    }
    return name;
  }

  std::string name_typedef(t_type_ref ref) {
    auto type = ref.get_type();
    std::vector<std::string> annotations;
    for (const auto& [k, v] : type->annotations()) {
      annotations.push_back(fmt::format("{}_{}", k, v.value));
    }
    auto name = fmt::format(
        "{}_{}_{}",
        type->get_full_name(),
        fmt::join(annotations, "_"),
        std::hash<std::string>()(prog_.path()) % 1000);
    auto prefix = prog_.scope_name("");
    if (name.starts_with(prefix)) {
      name.erase(0, prefix.length());
    }
    std::replace(name.begin(), name.end(), '<', '_');
    std::replace(name.begin(), name.end(), ',', '_');
    name.erase(
        std::remove_if(
            name.begin(),
            name.end(),
            [](auto c) { return !std::isalnum(c) && c != '_'; }),
        name.end());
    return name;
  }

  std::string render_type(t_type_ref ref) {
    auto type = ref.get_type();
    auto name = type->get_full_name();
    auto prefix = prog_.scope_name("");
    if (name.starts_with(prefix)) {
      name.erase(0, prefix.length());
    }
    if (!needs_replacement(ref)) {
      return name;
    }
    std::vector<std::string> annotations;
    for (const auto& [k, v] : type->annotations()) {
      annotations.push_back(fmt::format("{} = '{}'", k, v.value));
    }
    assert(!annotations.empty());
    return fmt::format("{} ({})", name, fmt::join(annotations, ", "));
  }

 private:
  std::map<std::string, std::string> typedefs_;
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
  hoist_annotated_types(source_mgr, *program).run();

  return 0;
}
