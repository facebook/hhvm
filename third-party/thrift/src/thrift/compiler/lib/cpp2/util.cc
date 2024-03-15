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

#include <thrift/compiler/lib/cpp2/util.h>
#include "thrift/compiler/ast/t_program.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <queue>
#include <stdexcept>

#include <openssl/sha.h>

#include <fmt/core.h>

#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_list.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/ast/t_set.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/gen/cpp/type_resolver.h>
#include <thrift/compiler/lib/uri.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace cpp2 {
namespace {

bool contains(std::string_view s, std::string_view what) {
  return std::search(s.begin(), s.end(), what.begin(), what.end()) != s.end();
}

std::string_view value_or_empty(const std::string* value) {
  return value ? *value : std::string_view("");
}

} // namespace

bool is_custom_type(const t_type& type) {
  return t_typedef::get_first_annotation_or_null(
             &type,
             {
                 "cpp.template",
                 "cpp2.template",
                 "cpp.type",
                 "cpp2.type",
             }) ||
      t_typedef::get_first_structured_annotation_or_null(&type, kCppAdapterUri);
}

bool is_custom_type(const t_field& field) {
  return gen::cpp::type_resolver::find_first_adapter(field) ||
      is_custom_type(*field.get_type());
}

bool container_supports_incomplete_params(const t_type& type) {
  if (t_typedef::get_first_structured_annotation_or_null(
          &type,
          "facebook.com/thrift/annotation/cpp/Frozen2RequiresCompleteContainerParams")) {
    return false;
  }

  if (t_typedef::get_first_annotation_or_null(
          &type,
          {
              "cpp.container_supports_incomplete_params",
          }) ||
      !is_custom_type(type)) {
    return true;
  }

  static const std::unordered_set<std::string> template_exceptions = [] {
    std::unordered_set<std::string> types;
    for (auto& type : {
             "folly::F14NodeMap",
             "folly::F14VectorMap",
             "folly::small_vector_map",
             "folly::sorted_vector_map",

             "folly::F14NodeSet",
             "folly::F14VectorSet",
             "folly::small_vector_set",
             "folly::sorted_vector_set",

             "std::forward_list",
             "std::list",
         }) {
      types.insert(type);
      types.insert(fmt::format("::{}", type));
    }
    return types;
  }();
  {
    auto cpp_template = t_typedef::get_first_annotation_or_null(
        &type,
        {
            "cpp.template",
            "cpp2.template",
        });
    if (cpp_template && template_exceptions.count(*cpp_template)) {
      return true;
    }
  }
  {
    auto cpp_type = t_typedef::get_first_annotation_or_null(
        &type,
        {
            "cpp.type",
            "cpp2.type",
        });
    if (cpp_type) {
      auto cpp_template = cpp_type->substr(0, cpp_type->find('<'));
      if (template_exceptions.count(cpp_template)) {
        return true;
      }
    }
  }

  return false;
}

namespace {
bool has_dependent_adapter(const t_type& node) {
  if (auto annotation = t_typedef::get_first_structured_annotation_or_null(
          &node, kCppAdapterUri)) {
    return !annotation->get_value_from_structured_annotation_or_null(
        "adaptedType");
  }
  return false;
}
} // namespace

std::unordered_map<const t_type*, std::vector<const t_type*>>
gen_dependency_graph(
    const t_program* program, const std::vector<const t_type*>& types) {
  std::unordered_map<const t_type*, std::vector<const t_type*>> edges(
      types.size());
  for (const auto* obj : types) {
    auto& deps = edges[obj];

    std::function<void(const t_type*, bool)> add_dependency =
        [&](const t_type* type, bool include_structs) {
          if (auto typedf = dynamic_cast<t_typedef const*>(type)) {
            // Resolve unnamed typedefs
            if (typedf->typedef_kind() != t_typedef::kind::defined) {
              type = typedf->get_type();
            }
          }

          if (auto map = dynamic_cast<t_map const*>(type)) {
            add_dependency(
                map->get_key_type(),
                include_structs && !container_supports_incomplete_params(*map));
            return add_dependency(
                map->get_val_type(),
                include_structs && !container_supports_incomplete_params(*map));
          } else if (auto set = dynamic_cast<t_set const*>(type)) {
            return add_dependency(
                set->get_elem_type(),
                include_structs && !container_supports_incomplete_params(*set));
          } else if (auto list = dynamic_cast<t_list const*>(type)) {
            return add_dependency(
                list->get_elem_type(),
                include_structs &&
                    !container_supports_incomplete_params(*list));
          } else if (auto typedf = dynamic_cast<t_typedef const*>(type)) {
            // Transitively depend on true type if necessary, since typedefs
            // generally don't depend on their underlying types.
            add_dependency(typedf->get_true_type(), include_structs);
          } else if (!(dynamic_cast<t_struct const*>(type) &&
                       (include_structs || has_dependent_adapter(*type)))) {
            return;
          }

          // We're only interested in types defined in the current program.
          if (type->program() != program) {
            return;
          }

          deps.emplace_back(type);
        };

    if (auto* typedf = dynamic_cast<t_typedef const*>(obj)) {
      // The adjacency list of a typedef is the list of structs and typedefs
      // named in its underlying type, but we only care about structs if the
      // typedef or struct has an adapter annotation without adaptedType
      // specified.
      const auto* type = &*typedf->type();
      add_dependency(type, has_dependent_adapter(*typedf));
    } else if (auto* strct = dynamic_cast<t_struct const*>(obj)) {
      // The adjacency list of a struct is the structs and typedefs named in its
      // fields.
      for (const auto& field : strct->fields()) {
        auto ftype = field.type();
        ftype.resolve();
        if (!ftype.resolved()) {
          continue;
        }
        const auto* type = &*ftype;
        add_dependency(type, !cpp2::is_explicit_ref(&field));
      }
    } else {
      assert(false);
    }

    // Order all deps in the order they are defined in.
    std::sort(deps.begin(), deps.end(), [](const t_type* a, const t_type* b) {
      return a->src_range().begin.offset() < b->src_range().begin.offset();
    });
  }
  return edges;
}

namespace {

struct is_orderable_walk_context {
  std::unordered_set<t_type const*> pending_back_propagation;
  std::unordered_set<t_type const*> seen;
  std::unordered_map<t_type const*, std::unordered_set<t_type const*>>
      inv_graph;
};

bool is_orderable_walk(
    std::unordered_map<t_type const*, bool>& memo,
    t_type const& type,
    t_type const* prev,
    is_orderable_walk_context& context,
    bool enabledReflection) {
  const bool has_disqualifying_annotation =
      is_custom_type(type) && !enabledReflection;
  auto memo_it = memo.find(&type);
  if (memo_it != memo.end()) {
    return memo_it->second;
  }
  if (prev != nullptr) {
    context.inv_graph[&type].insert(prev);
  }
  if (!context.seen.insert(&type).second) {
    return true; // Recursive type, speculate success.
  }
  auto make_scope_guard = [](auto f) {
    auto deleter = [=](void*) { f(); };
    return std::unique_ptr<void, decltype(deleter)>(&f, deleter);
  };
  auto g = make_scope_guard([&] { context.seen.erase(&type); });
  if (type.is_base_type() || type.is_enum()) {
    return true;
  }
  bool result = false;
  auto g2 = make_scope_guard([&] {
    memo[&type] = result;
    if (!result) {
      context.pending_back_propagation.insert(&type);
    }
  });
  if (type.is_typedef()) {
    auto const& real = [&]() -> auto&& { return *type.get_true_type(); };
    auto const& next = *(dynamic_cast<t_typedef const&>(type).get_type());
    return result = is_orderable_walk(
                        memo, next, &type, context, enabledReflection) &&
        (!(real().is_set() || real().is_map()) ||
         !has_disqualifying_annotation);
  } else if (type.is_struct() || type.is_exception()) {
    const auto& as_struct = static_cast<t_struct const&>(type);
    return result = std::all_of(
               as_struct.fields().begin(),
               as_struct.fields().end(),
               [&](const auto& f) {
                 return is_orderable_walk(
                     memo,
                     f.type().deref(),
                     &type,
                     context,
                     !as_struct.uri().empty());
               });
  } else if (type.is_list()) {
    return result = is_orderable_walk(
               memo,
               *(dynamic_cast<t_list const&>(type).get_elem_type()),
               &type,
               context,
               enabledReflection);
  } else if (type.is_set()) {
    return result = !has_disqualifying_annotation &&
        is_orderable_walk(
               memo,
               *(dynamic_cast<t_set const&>(type).get_elem_type()),
               &type,
               context,
               enabledReflection);
  } else if (type.is_map()) {
    return result = !has_disqualifying_annotation &&
        is_orderable_walk(
               memo,
               *(dynamic_cast<t_map const&>(type).get_key_type()),
               &type,
               context,
               enabledReflection) &&
        is_orderable_walk(
               memo,
               *(dynamic_cast<t_map const&>(type).get_val_type()),
               &type,
               context,
               enabledReflection);
  }
  return false;
}

void is_orderable_back_propagate(
    std::unordered_map<t_type const*, bool>& memo,
    is_orderable_walk_context& context) {
  while (!context.pending_back_propagation.empty()) {
    auto type = *context.pending_back_propagation.begin();
    context.pending_back_propagation.erase(
        context.pending_back_propagation.begin());
    auto edges = context.inv_graph.find(type);
    if (edges == context.inv_graph.end()) {
      continue;
    }
    for (t_type const* prev : edges->second) {
      if (std::exchange(memo[prev], false)) {
        context.pending_back_propagation.insert(prev);
      }
    }
    context.inv_graph.erase(edges);
  }
}

} // namespace

bool is_orderable(
    std::unordered_map<t_type const*, bool>& memo, t_type const& type) {
  // Thrift struct could self-reference, so have to perform a two-stage walk:
  // first all self-references are speculated, then negative classification is
  // back-propagated through the traversed dependencies.
  is_orderable_walk_context context;
  is_orderable_walk(memo, type, nullptr, context, false);
  is_orderable_back_propagate(memo, context);
  auto it = memo.find(&type);
  return it == memo.end() || it->second;
}

bool is_orderable(t_type const& type) {
  std::unordered_map<t_type const*, bool> memo;
  return is_orderable(memo, type);
}

std::string_view get_type(const t_type* type) {
  return value_or_empty(gen::cpp::type_resolver::find_type(*type));
}

bool is_implicit_ref(const t_type* type) {
  auto const* resolved_typedef = type->get_true_type();
  return resolved_typedef != nullptr && resolved_typedef->is_binary() &&
      contains(get_type(resolved_typedef), "std::unique_ptr") &&
      contains(get_type(resolved_typedef), "folly::IOBuf");
}

bool field_transitively_refers_to_unique(const t_field* field) {
  switch (gen::cpp::find_ref_type(*field)) {
    case gen::cpp::reference_type::none:
      break;
    case gen::cpp::reference_type::unique: {
      return true;
    }
    case gen::cpp::reference_type::boxed:
    case gen::cpp::reference_type::boxed_intern:
    case gen::cpp::reference_type::shared_const:
    case gen::cpp::reference_type::shared_mutable: {
      return false;
    }
  }
  std::queue<const t_type*> queue;
  queue.push(field->get_type());
  while (!queue.empty()) {
    auto type = queue.front()->get_true_type();
    queue.pop();
    if (cpp2::is_implicit_ref(type)) {
      return true;
    }
    switch (type->get_type_value()) {
      case t_type::type::t_list: {
        queue.push(static_cast<const t_list*>(type)->get_elem_type());
        break;
      }
      case t_type::type::t_set: {
        queue.push(static_cast<const t_set*>(type)->get_elem_type());
        break;
      }
      case t_type::type::t_map: {
        queue.push(static_cast<const t_map*>(type)->get_key_type());
        queue.push(static_cast<const t_map*>(type)->get_val_type());
        break;
      }
      default: {
        break;
      }
    }
  }
  return false;
}

bool is_eligible_for_constexpr::operator()(const t_type* type) {
  enum class eligible { unknown, yes, no };
  auto check = [this](const t_type* t) {
    auto it = cache_.find(t);
    if (it != cache_.end()) {
      return it->second ? eligible::yes : eligible::no;
    }
    bool result = false;
    if (t->has_annotation("cpp.indirection")) {
      // Custom types may not have constexpr constructors.
      result = false;
    } else if (
        t->is_any_int() || t->is_floating_point() || t->is_bool() ||
        t->is_enum()) {
      result = true;
    } else if (t->is_union() || t->is_exception()) {
      // Union and exception constructors are not defaulted.
      result = false;
    } else if (t->has_annotation(
                   {"cpp.virtual", "cpp2.virtual", "cpp.allocator"})) {
      result = false;
    } else {
      return eligible::unknown;
    }
    cache_[t] = result;
    return result ? eligible::yes : eligible::no;
  };
  auto result = check(type);
  if (result != eligible::unknown) {
    return result == eligible::yes;
  }
  if (const auto* s = dynamic_cast<const t_struct*>(type)) {
    result = eligible::yes;
    for_each_transitive_field(s, [&](const t_field* field) {
      result = check(field->get_type());
      if (result == eligible::no) {
        return false;
      } else if (is_explicit_ref(field) || is_lazy(field)) {
        result = eligible::no;
        return false;
      } else if (result == eligible::unknown) {
        if (!field->get_type()->is_struct()) {
          return false;
        }
        // Structs are eligible if all their fields are.
        result = eligible::yes;
      }
      return true;
    });
    return result == eligible::yes;
  }
  return false;
}

bool is_stack_arguments(
    std::map<std::string, std::string> const& options,
    t_function const& function) {
  if (function.has_annotation("cpp.stack_arguments")) {
    return function.get_annotation("cpp.stack_arguments") != "0";
  }
  return options.count("stack_arguments");
}

bool is_mixin(const t_field& field) {
  return field.has_annotation("cpp.mixin") ||
      field.find_structured_annotation_or_null(kMixinUri) != nullptr;
}

bool has_ref_annotation(const t_field& field) {
  switch (gen::cpp::find_ref_type(field)) {
    case gen::cpp::reference_type::unique:
    case gen::cpp::reference_type::shared_const:
    case gen::cpp::reference_type::shared_mutable:
      return true;
    case gen::cpp::reference_type::none:
    case gen::cpp::reference_type::boxed_intern:
    case gen::cpp::reference_type::boxed:
      return false;
  }
  throw std::logic_error("Unhandled ref_type");
}

static void get_mixins_and_members_impl(
    const t_structured& strct,
    const t_field* top_level_mixin,
    std::vector<mixin_member>& out) {
  for (const auto& member : strct.fields()) {
    if (is_mixin(member)) {
      assert(member.type()->get_true_type()->is_struct());
      auto mixin_struct =
          static_cast<const t_struct*>(member.type()->get_true_type());
      const auto& mixin =
          top_level_mixin != nullptr ? *top_level_mixin : member;

      // import members from mixin field
      for (const auto& member_from_mixin : mixin_struct->fields()) {
        out.push_back({&mixin, &member_from_mixin});
      }

      // import members from nested mixin field
      get_mixins_and_members_impl(*mixin_struct, &mixin, out);
    }
  }
}

std::vector<mixin_member> get_mixins_and_members(const t_structured& strct) {
  std::vector<mixin_member> ret;
  get_mixins_and_members_impl(strct, nullptr, ret);
  return ret;
}

namespace {
struct get_gen_type_class_options {
  bool gen_indirection = false;
  bool gen_indirection_inner_ = false;
};

std::string get_gen_type_class_(
    t_type const& type_, get_gen_type_class_options opts) {
  std::string const ns = "::apache::thrift::";
  std::string const tc = ns + "type_class::";

  auto const& type = *type_.get_true_type();

  bool const ind = type.has_annotation("cpp.indirection");
  if (ind && opts.gen_indirection && !opts.gen_indirection_inner_) {
    opts.gen_indirection_inner_ = true;
    auto const inner = get_gen_type_class_(type_, opts);
    auto const tag = ns + "detail::indirection_tag";
    auto const fun = ns + "detail::apply_indirection_fn";
    return tag + "<" + inner + ", " + fun + ">";
  }
  opts.gen_indirection_inner_ = false;

  if (type.is_void()) {
    return tc + "nothing";
  } else if (type.is_bool() || type.is_byte() || type.is_any_int()) {
    return tc + "integral";
  } else if (type.is_floating_point()) {
    return tc + "floating_point";
  } else if (type.is_enum()) {
    return tc + "enumeration";
  } else if (type.is_string()) {
    return tc + "string";
  } else if (type.is_binary()) {
    return tc + "binary";
  } else if (type.is_list()) {
    auto& list = dynamic_cast<t_list const&>(type);
    auto& elem = *list.get_elem_type();
    auto elem_tc = get_gen_type_class_(elem, opts);
    return tc + "list<" + elem_tc + ">";
  } else if (type.is_set()) {
    auto& set = dynamic_cast<t_set const&>(type);
    auto& elem = *set.get_elem_type();
    auto elem_tc = get_gen_type_class_(elem, opts);
    return tc + "set<" + elem_tc + ">";
  } else if (type.is_map()) {
    auto& map = dynamic_cast<t_map const&>(type);
    auto& key = *map.get_key_type();
    auto& val = *map.get_val_type();
    auto key_tc = get_gen_type_class_(key, opts);
    auto val_tc = get_gen_type_class_(val, opts);
    return tc + "map<" + key_tc + ", " + val_tc + ">";
  } else if (type.is_union()) {
    return tc + "variant";
  } else if (type.is_struct() || type.is_exception()) {
    return tc + "structure";
  } else {
    throw std::runtime_error("unknown type class for: " + type.get_full_name());
  }
}

} // namespace

std::string get_gen_type_class(t_type const& type) {
  get_gen_type_class_options opts;
  return get_gen_type_class_(type, opts);
}

std::string get_gen_type_class_with_indirection(t_type const& type) {
  get_gen_type_class_options opts;
  opts.gen_indirection = true;
  return get_gen_type_class_(type, opts);
}

std::string sha256_hex(std::string const& in) {
  std::uint8_t mid[SHA256_DIGEST_LENGTH];
  SHA256_CTX hasher;
  SHA256_Init(&hasher);
  SHA256_Update(&hasher, in.data(), in.size());
  SHA256_Final(mid, &hasher);

  constexpr auto alpha = "0123456789abcdef";

  std::string out;
  for (size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
    constexpr auto mask = std::uint8_t(std::uint8_t(~std::uint8_t(0)) >> 4);
    auto hi = (mid[i] >> 4) & mask;
    auto lo = (mid[i] >> 0) & mask;
    out.push_back(alpha[hi]);
    out.push_back(alpha[lo]);
  }
  return out;
}

bool is_cpp_ref_unique_either(const t_field* f) {
  return cpp2::is_unique_ref(f) || cpp2::is_implicit_ref(f->get_type());
}

bool deprecated_terse_writes(const t_field* field) {
  // Add terse writes for unqualified fields when comparison is cheap:
  // (e.g. i32/i64, empty strings/list/map)
  auto t = field->get_type()->get_true_type();
  return field->get_req() == t_field::e_req::opt_in_req_out &&
      (is_cpp_ref_unique_either(field) ||
       (!t->is_struct() && !t->is_exception()));
}

t_field_id get_internal_injected_field_id(t_field_id id) {
  t_field_id internal_id = kInjectMetadataFieldsStartId - id;
  if (internal_id > kInjectMetadataFieldsStartId ||
      internal_id <= kInjectMetadataFieldsLastId) {
    throw std::runtime_error(
        fmt::format("Field id `{}` does not mapped to valid internal id.", id));
  }
  return internal_id;
}

const t_const* get_transitive_annotation_of_adapter_or_null(
    const t_named& node) {
  for (const auto& annotation : node.structured_annotations()) {
    const t_type& annotation_type = *annotation.type();
    if (is_transitive_annotation(annotation_type)) {
      if (annotation_type.find_structured_annotation_or_null(kCppAdapterUri)) {
        return &annotation;
      }
    }
  }
  return nullptr;
}

} // namespace cpp2
} // namespace compiler
} // namespace thrift
} // namespace apache
