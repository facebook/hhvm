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

#include <thrift/compiler/generate/cpp/util.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <queue>
#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/sha.h>

#include <fmt/core.h>

#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_list.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_set.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/cpp/name_resolver.h>

namespace apache::thrift::compiler::cpp2 {
namespace {

bool contains(std::string_view s, std::string_view what) {
  return std::search(s.begin(), s.end(), what.begin(), what.end()) != s.end();
}

std::string_view value_or_empty(const std::string* value) {
  return value ? *value : std::string_view("");
}

bool has_dependent_adapter(const t_type& node) {
  if (auto annotation = t_typedef::get_first_structured_annotation_or_null(
          &node, kCppAdapterUri)) {
    return !annotation->get_value_from_structured_annotation_or_null(
        "adaptedType");
  }
  return false;
}

} // namespace

bool is_custom_type(const t_type& type) {
  return t_typedef::get_first_unstructured_annotation_or_null(
             &type,
             {
                 "cpp.template",
                 "cpp2.template",
                 "cpp.type",
                 "cpp2.type",
             }) ||
      t_typedef::get_first_structured_annotation_or_null(&type, kCppTypeUri) ||
      t_typedef::get_first_structured_annotation_or_null(&type, kCppAdapterUri);
}

bool is_custom_type(const t_field& field) {
  return cpp_name_resolver::find_first_adapter(field) ||
      field.has_structured_annotation(kCppTypeUri) ||
      is_custom_type(*field.get_type());
}

bool container_supports_incomplete_params(const t_type& type) {
  if (t_typedef::get_first_structured_annotation_or_null(
          &type,
          "facebook.com/thrift/annotation/cpp/Frozen2RequiresCompleteContainerParams")) {
    return false;
  }

  if (t_typedef::get_first_unstructured_annotation_or_null(
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
    auto cpp_template = t_typedef::get_first_unstructured_annotation_or_null(
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
    auto cpp_type = t_typedef::get_first_unstructured_annotation_or_null(
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

std::unordered_map<const t_type*, std::vector<const t_type*>>
gen_dependency_graph(
    const t_program* program, const std::vector<const t_type*>& types) {
  std::unordered_map<const t_type*, std::vector<const t_type*>> edges(
      types.size());
  for (const auto* obj : types) {
    auto& deps = edges[obj];

    std::function<void(const t_type*, bool)> add_dependency =
        [&](const t_type* type, bool include_structured_types) {
          if (auto typedf = dynamic_cast<t_typedef const*>(type)) {
            // Resolve unnamed typedefs
            if (typedf->typedef_kind() != t_typedef::kind::defined) {
              type = typedf->get_type();
            }
          }

          if (auto map = dynamic_cast<t_map const*>(type)) {
            add_dependency(
                map->key_type().get_type(),
                include_structured_types &&
                    !container_supports_incomplete_params(*map));
            return add_dependency(
                map->val_type().get_type(),
                include_structured_types &&
                    !container_supports_incomplete_params(*map));
          } else if (auto set = dynamic_cast<t_set const*>(type)) {
            return add_dependency(
                set->elem_type().get_type(),
                include_structured_types &&
                    !container_supports_incomplete_params(*set));
          } else if (auto list = dynamic_cast<t_list const*>(type)) {
            return add_dependency(
                list->elem_type().get_type(),
                include_structured_types &&
                    !container_supports_incomplete_params(*list));
          } else if (auto typedf = dynamic_cast<t_typedef const*>(type)) {
            // Transitively depend on true type if necessary, since typedefs
            // generally don't depend on their underlying types.
            add_dependency(typedf->get_true_type(), include_structured_types);
          } else if (!(type->is<t_structured>() &&
                       (include_structured_types ||
                        has_dependent_adapter(*type)))) {
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
    } else if (auto* strct = dynamic_cast<t_structured const*>(obj)) {
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

std::string_view get_type(const t_type* type) {
  return value_or_empty(cpp_name_resolver::find_type(*type));
}

bool is_binary_iobuf_unique_ptr(const t_type* type) {
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
    if (cpp2::is_binary_iobuf_unique_ptr(type)) {
      return true;
    }
    if (const t_list* list = type->try_as<t_list>()) {
      queue.push(list->elem_type().get_type());
    } else if (const t_set* set = type->try_as<t_set>()) {
      queue.push(set->elem_type().get_type());
    } else if (const t_map* map = type->try_as<t_map>()) {
      queue.push(map->key_type().get_type());
      queue.push(map->val_type().get_type());
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
    if (t->is_any_int() || t->is_floating_point() || t->is_bool() ||
        t->is<t_enum>()) {
      result = true;
    } else if (t->is<t_union>() || t->is<t_exception>()) {
      // Union and exception constructors are not defaulted.
      result = false;
    } else if (t->has_unstructured_annotation(
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
  if (const auto* s = dynamic_cast<const t_structured*>(type)) {
    result = eligible::yes;
    for_each_transitive_field(s, [&](const t_field* field) {
      result = check(field->get_type());
      if (result == eligible::no) {
        return false;
      } else if (is_explicit_ref(field) || is_lazy(field)) {
        result = eligible::no;
        return false;
      } else if (result == eligible::unknown) {
        if (!field->get_type()->is<t_struct>() &&
            !field->get_type()->is<t_union>()) {
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
    std::map<std::string, std::string, std::less<>> const& options,
    t_function const& function) {
  if (function.has_unstructured_annotation("cpp.stack_arguments")) {
    return function.get_unstructured_annotation("cpp.stack_arguments") != "0";
  }
  return options.count("stack_arguments");
}

bool is_mixin(const t_field& field) {
  return field.has_unstructured_annotation("cpp.mixin") ||
      field.has_structured_annotation(kMixinUri);
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
      assert(member.type()->get_true_type()->is<t_structured>());
      auto mixin_struct =
          member.type()->get_true_type()->try_as<t_structured>();
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

std::string get_gen_type_class(t_type const& type) {
  std::string const ns = "::apache::thrift::";
  std::string const tc = ns + "type_class::";

  auto const& ttype = *type.get_true_type();

  if (ttype.is_void()) {
    return tc + "nothing";
  } else if (ttype.is_bool() || ttype.is_byte() || ttype.is_any_int()) {
    return tc + "integral";
  } else if (ttype.is_floating_point()) {
    return tc + "floating_point";
  } else if (ttype.is<t_enum>()) {
    return tc + "enumeration";
  } else if (ttype.is_string()) {
    return tc + "string";
  } else if (ttype.is_binary()) {
    return tc + "binary";
  } else if (const t_list* list = ttype.try_as<t_list>()) {
    auto& elem = *list->elem_type();
    auto elem_tc = get_gen_type_class(elem);
    return tc + "list<" + elem_tc + ">";
  } else if (const t_set* set = ttype.try_as<t_set>()) {
    auto& elem = *set->elem_type();
    auto elem_tc = get_gen_type_class(elem);
    return tc + "set<" + elem_tc + ">";
  } else if (const t_map* map = ttype.try_as<t_map>()) {
    auto& key = *map->key_type();
    auto& val = *map->val_type();
    auto key_tc = get_gen_type_class(key);
    auto val_tc = get_gen_type_class(val);
    return tc + "map<" + key_tc + ", " + val_tc + ">";
  } else if (ttype.is<t_union>()) {
    return tc + "variant";
  } else if (ttype.is<t_structured>()) {
    return tc + "structure";
  } else {
    throw std::runtime_error(
        "unknown type class for: " + ttype.get_full_name());
  }
}

std::string sha256_hex(std::string const& in) {
  std::uint8_t mid[SHA256_DIGEST_LENGTH];
  EVP_Digest(in.data(), in.size(), mid, nullptr, EVP_sha256(), nullptr);

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

bool deprecated_terse_writes(const t_field* field) {
  // Add terse writes for unqualified fields when comparison is cheap:
  // (e.g. i32/i64, empty strings/list/map)
  auto t = field->get_type()->get_true_type();
  return field->get_req() == t_field::e_req::opt_in_req_out &&
      (cpp2::is_unique_ref(field) || (!t->is<t_structured>()));
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
      if (annotation_type.has_structured_annotation(kCppAdapterUri)) {
        return &annotation;
      }
    }
  }
  return nullptr;
}

} // namespace apache::thrift::compiler::cpp2
