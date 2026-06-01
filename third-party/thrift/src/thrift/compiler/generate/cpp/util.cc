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

// Container templates known to support incomplete element types.
const std::unordered_set<std::string>& get_template_exceptions() {
  static const std::unordered_set<std::string> types = [] {
    std::unordered_set<std::string> s;
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

             "::apache::thrift::metadata::detail::LimitedVector",
         }) {
      s.insert(type);
      s.insert(fmt::format("::{}", type));
    }
    return s;
  }();
  return types;
}

// Check if a field's @cpp.Type template supports incomplete container params.
// Returns true if there is no field-level @cpp.Type or if the template is
// known to support incomplete params.
bool field_cpp_type_supports_incomplete_params(const t_field& field) {
  auto* annot = field.find_structured_annotation_or_null(kCppTypeUri);
  if (!annot) {
    return true;
  }
  const auto& exceptions = get_template_exceptions();
  if (auto* tmpl =
          annot->get_value_from_structured_annotation_or_null("template")) {
    return exceptions.count(tmpl->get_string()) > 0;
  }
  if (auto* name =
          annot->get_value_from_structured_annotation_or_null("name")) {
    auto cpp_template =
        name->get_string().substr(0, name->get_string().find('<'));
    return exceptions.count(cpp_template) > 0;
  }
  // Has @cpp.Type but no template or name — assume requires complete types.
  return false;
}

} // namespace

bool is_custom_type(const t_type& type) {
  return t_typedef::get_first_structured_annotation_or_null(
             &type, kCppTypeUri) ||
      t_typedef::get_first_structured_annotation_or_null(&type, kCppAdapterUri);
}

bool is_custom_type(const t_field& field) {
  return cpp_name_resolver::find_first_adapter(field) ||
      field.has_structured_annotation(kCppTypeUri) ||
      is_custom_type(*field.type());
}

bool container_supports_incomplete_params(const t_type& type) {
  if (t_typedef::get_first_structured_annotation_or_null(
          &type,
          "facebook.com/thrift/annotation/cpp/Frozen2RequiresCompleteContainerParams")) {
    return false;
  }

  if (!is_custom_type(type)) {
    return true;
  }

  const auto& template_exceptions = get_template_exceptions();
  if (auto* annot = t_typedef::get_first_structured_annotation_or_null(
          &type, kCppTypeUri)) {
    if (auto* tmpl =
            annot->get_value_from_structured_annotation_or_null("template")) {
      if (template_exceptions.count(tmpl->get_string())) {
        return true;
      }
    }
    if (auto* name =
            annot->get_value_from_structured_annotation_or_null("name")) {
      auto cpp_template =
          name->get_string().substr(0, name->get_string().find('<'));
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

    // force_complete_container: when true, overrides
    // container_supports_incomplete_params for the first container
    // encountered. Used when a field has @cpp.Type that requires complete
    // element types (the annotation is on the field, not the type node).
    std::function<void(const t_type*, bool, bool)> add_dependency =
        [&](const t_type* type,
            bool include_structured_types,
            bool force_complete_container) {
          if (const auto* typedf = type->try_as<t_typedef>()) {
            // Resolve unnamed typedefs
            if (typedf->typedef_kind() != t_typedef::kind::defined) {
              type = &typedf->type().deref();
            }
          }

          if (const auto* map = type->try_as<t_map>()) {
            bool supports_incomplete = !force_complete_container &&
                container_supports_incomplete_params(*map);
            add_dependency(
                map->key_type().get_type(),
                include_structured_types && !supports_incomplete,
                false);
            return add_dependency(
                map->val_type().get_type(),
                include_structured_types && !supports_incomplete,
                false);
          } else if (const auto* set = type->try_as<t_set>()) {
            bool supports_incomplete = !force_complete_container &&
                container_supports_incomplete_params(*set);
            return add_dependency(
                set->elem_type().get_type(),
                include_structured_types && !supports_incomplete,
                false);
          } else if (const auto* list = type->try_as<t_list>()) {
            bool supports_incomplete = !force_complete_container &&
                container_supports_incomplete_params(*list);
            return add_dependency(
                list->elem_type().get_type(),
                include_structured_types && !supports_incomplete,
                false);
          } else if (const auto* typedf = type->try_as<t_typedef>()) {
            // Transitively depend on true type if necessary, since typedefs
            // generally don't depend on their underlying types.
            //
            // When a typedef has @cpp.Type (structured annotation) and its
            // true type is a container, check
            // container_supports_incomplete_params on the typedef (which has
            // the annotation) rather than the container (which no longer has it
            // after removing annotation lowering). This ensures element types
            // are added as dependencies when the container template requires
            // complete types.
            if (typedf->typedef_kind() == t_typedef::kind::defined &&
                include_structured_types &&
                typedf->find_structured_annotation_or_null(kCppTypeUri) !=
                    nullptr) {
              const auto* true_type = typedf->get_true_type();
              if (true_type->try_as<t_container>() != nullptr &&
                  !container_supports_incomplete_params(*typedf)) {
                if (const auto* inner_map = true_type->try_as<t_map>()) {
                  add_dependency(inner_map->key_type().get_type(), true, false);
                  add_dependency(inner_map->val_type().get_type(), true, false);
                } else if (const auto* inner_set = true_type->try_as<t_set>()) {
                  add_dependency(
                      inner_set->elem_type().get_type(), true, false);
                } else if (const auto* list_t = true_type->try_as<t_list>()) {
                  add_dependency(list_t->elem_type().get_type(), true, false);
                }
              } else {
                add_dependency(
                    typedf->get_true_type(), include_structured_types, false);
              }
            } else {
              add_dependency(
                  typedf->get_true_type(), include_structured_types, false);
            }
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
      bool include_structured = has_dependent_adapter(*typedf);
      add_dependency(type, include_structured, false);
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
        bool include_structured = !cpp2::is_explicit_ref(&field);
        // When @cpp.Type is on the field (not the type node), check if the
        // field's custom container requires complete element types.
        bool force_complete = include_structured &&
            !field_cpp_type_supports_incomplete_params(field);
        add_dependency(type, include_structured, force_complete);
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
  if (resolved_typedef == nullptr || !resolved_typedef->is_binary()) {
    return false;
  }
  // Check for cpp.type on the true type (legacy unstructured annotations)
  // or on any typedef in the chain (structured @cpp.Type).
  auto type_str = get_type(resolved_typedef);
  if (type_str.empty()) {
    if (auto* annot = t_typedef::get_first_structured_annotation_or_null(
            type, kCppTypeUri)) {
      if (auto* name =
              annot->get_value_from_structured_annotation_or_null("name")) {
        type_str = name->get_string();
      }
    }
  }
  return contains(type_str, "std::unique_ptr") &&
      contains(type_str, "folly::IOBuf");
}

bool is_binary_iobuf_unique_ptr(const t_field& field) {
  // Check field-level @cpp.Type annotation first.
  if (auto* annot = field.find_structured_annotation_or_null(kCppTypeUri)) {
    if (auto* name =
            annot->get_value_from_structured_annotation_or_null("name")) {
      const auto& type_str = name->get_string();
      return field.type().deref().get_true_type()->is_binary() &&
          contains(type_str, "std::unique_ptr") &&
          contains(type_str, "folly::IOBuf");
    }
  }
  // Fall back to type-level check (for typedef-level @cpp.Type).
  return is_binary_iobuf_unique_ptr(field.type().get_type());
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
  // Check field-level @cpp.Type for iobuf first.
  if (cpp2::is_binary_iobuf_unique_ptr(*field)) {
    return true;
  }
  // Then walk container element types (type-only check).
  std::queue<const t_type*> queue;
  {
    auto type = field->type().get_type()->get_true_type();
    if (const t_list* list = type->try_as<t_list>()) {
      queue.push(list->elem_type().get_type());
    } else if (const t_set* set = type->try_as<t_set>()) {
      queue.push(set->elem_type().get_type());
    } else if (const t_map* map = type->try_as<t_map>()) {
      queue.push(map->key_type().get_type());
      queue.push(map->val_type().get_type());
    }
  }
  while (!queue.empty()) {
    auto orig_type = queue.front();
    auto type = orig_type->get_true_type();
    queue.pop();
    if (cpp2::is_binary_iobuf_unique_ptr(orig_type)) {
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
      // Field-level @cpp.Type{name=...} on a named type (typedef/struct)
      // means the C++ type may have non-trivial constructors.
      if (auto* annot = field->find_structured_annotation_or_null(kCppTypeUri);
          annot &&
          (annot->get_value_from_structured_annotation_or_null("name") ||
           annot->get_value_from_structured_annotation_or_null("template")) &&
          !field->type()->get_true_type()->is<t_primitive_type>() &&
          !field->type()->get_true_type()->is<t_container>()) {
        result = eligible::no;
        return false;
      }
      result = check(field->type().get_type());
      if (result == eligible::no) {
        return false;
      } else if (is_explicit_ref(field) || is_lazy(field)) {
        result = eligible::no;
        return false;
      } else if (result == eligible::unknown) {
        if (!field->type()->is<t_struct>() && !field->type()->is<t_union>()) {
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
  auto t = field->type()->get_true_type();
  return field->qualifier() == t_field_qualifier::none &&
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
