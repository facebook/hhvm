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

#include <unordered_set>

#include <fmt/core.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/cpp/orderable_type_utils.h>
#include <thrift/compiler/generate/cpp/util.h>

namespace apache::thrift::compiler::cpp2 {

namespace {

bool enable_custom_type_ordering(const t_structured& s) {
  return s.program() != nullptr &&
      s.program()->inherit_annotation_or_null(s, kCppEnableCustomTypeOrdering);
}

struct is_orderable_walk_context {
  bool enableCustomTypeOrderingIfStructureHasUri;

  /**
   * Set of types that are determined to be non-orderable, but whose "parents"
   * (i.e., types that were seen prior to each type, during the initial walk)
   * still need to be marked as non-orderable.
   */
  std::unordered_set<const t_type*> pending_unorderable_back_propagation = {};

  /**
   * The set of types encountered in the current (depth-first) walk.
   */
  std::unordered_set<const t_type*> current_visiting_set = {};

  /**
   * Maps of reverse dependencies, i.e. for a given key `type`, maps the set of
   * all "parent" types, i.e. types that were seen just before the key type.
   */
  std::unordered_map<const t_type*, std::unordered_set<const t_type*>>
      inv_graph = {};
};

/**
 * Returns an object that will invoke the given functor upon deletion.
 * Typically used to run `f` upon leaving the current scope (eg. returning).
 */
template <typename F>
auto make_scope_guard(F&& f) {
  auto deleter = [=](void*) { f(); };
  return std::unique_ptr<void, decltype(deleter)>(&f, deleter);
}

bool field_is_orderable_walk(
    std::unordered_map<const t_type*, bool>& memo,
    const t_field& field,
    const t_type* prev,
    is_orderable_walk_context& context,
    bool forceCustomTypeOrderable);

bool type_is_orderable_walk(
    std::unordered_map<const t_type*, bool>& memo,
    const t_type& type,
    const t_type* prev,
    is_orderable_walk_context& context,
    bool forceCustomTypeOrderable) {
  auto memo_it = memo.find(&type);
  if (memo_it != memo.end()) {
    return memo_it->second;
  }
  if (prev != nullptr) {
    context.inv_graph[&type].insert(prev);
  }
  if (!context.current_visiting_set.insert(&type).second) {
    return true; // Recursive type, speculate success.
  }

  // On return: pop the current type from the set of types being visited in
  // the current walk.
  auto g = make_scope_guard([&] { context.current_visiting_set.erase(&type); });

  // Primitive types and enums are always orderable.
  if (type.is<t_primitive_type>() || type.is<t_enum>()) {
    return true;
  }

  bool result = false;
  // On return: update the result cache (`memo`) with the returned value.
  auto g2 = make_scope_guard([&] {
    memo[&type] = result;
    if (!result) {
      context.pending_unorderable_back_propagation.insert(&type);
    }
  });

  // Structured types (struct, union, exception) and lists cannot have
  // disqualifying annotations themselves, but their underlying fields may have
  // some. fields) may have some.
  if (const t_structured* asStructured = type.try_as<t_structured>()) {
    return result = std::all_of(
               asStructured->fields().begin(),
               asStructured->fields().end(),
               [&](const t_field& field) {
                 return field_is_orderable_walk(
                     memo,
                     field,
                     &type,
                     context,
                     enable_custom_type_ordering(*asStructured) ||
                         (context.enableCustomTypeOrderingIfStructureHasUri &&
                          !asStructured->uri().empty()));
               });
  } else if (const t_list* asList = type.try_as<t_list>()) {
    return result = type_is_orderable_walk(
               memo,
               *asList->elem_type(),
               &type,
               context,
               forceCustomTypeOrderable);
  }

  // If this point is reached, `type` is one of: map, set or typedef.
  // Maps and sets may be non-orderable, if they have custom C++ types AND
  // custom type ordering is not enabled. Typedefs may be non-orderable if they
  // point to maps or sets that are non-orderable.
  const bool has_disqualifying_annotation =
      is_custom_type(type) && !forceCustomTypeOrderable;
  if (const t_typedef* asTypedef = type.try_as<t_typedef>()) {
    const t_type& typedef_true_type = *type.get_true_type();

    const t_type& next = *asTypedef->type();
    if (!type_is_orderable_walk(
            memo,
            next /* type */,
            &type /* prev */,
            context,
            forceCustomTypeOrderable)) {
      return result = false;
    }

    if (typedef_true_type.is<t_set>() || typedef_true_type.is<t_map>()) {
      return result = !has_disqualifying_annotation;
    } else {
      return result = true;
    }
  } else if (const t_set* asSet = type.try_as<t_set>()) {
    return result = !has_disqualifying_annotation &&
        type_is_orderable_walk(
               memo,
               *asSet->elem_type(),
               &type,
               context,
               forceCustomTypeOrderable);
  } else if (const t_map* asMap = type.try_as<t_map>()) {
    return result = !has_disqualifying_annotation &&
        type_is_orderable_walk(
               memo,
               *asMap->key_type(),
               &type,
               context,
               forceCustomTypeOrderable) &&
        type_is_orderable_walk(
               memo,
               *asMap->val_type(),
               &type,
               context,
               forceCustomTypeOrderable);
  }

  throw std::logic_error(
      fmt::format(
          "type_is_orderable_walk unhandled t_type: {} (of type {})",
          type.get_scoped_name(),
          type.get_full_name()));
}

bool field_is_orderable_walk(
    std::unordered_map<const t_type*, bool>& memo,
    const t_field& field,
    const t_type* prev,
    is_orderable_walk_context& context,
    bool forceCustomTypeOrderable) {
  const t_type& field_type = field.type().deref();
  const t_type& field_true_type = *field_type.get_true_type();
  // Unlike `is_custom_type()` above, we don't consider @cpp.Adapter on the
  // field to be disqualifying, since all adapted fields can be made orderable
  // by customizing Adapter::less.
  const bool has_disqualifying_annotation =
      field.has_structured_annotation(kCppTypeUri) && !forceCustomTypeOrderable;
  if ((field_true_type.is<t_set>() || field_true_type.is<t_map>()) &&
      has_disqualifying_annotation) {
    return false;
  } else {
    return type_is_orderable_walk(
        memo, field_type, prev, context, forceCustomTypeOrderable);
  }
}

void back_propagate_unorderable_types(
    std::unordered_map<const t_type*, bool>& memo,
    is_orderable_walk_context& context) {
  // BEWARE: items can be inserted into pending_unorderable_back_propagation
  // while iterating, so we cannot avoid getting new iterators (via `begin()`),
  // as we would typically do with a for loop.
  while (!context.pending_unorderable_back_propagation.empty()) {
    const t_type* type = *context.pending_unorderable_back_propagation.begin();
    context.pending_unorderable_back_propagation.erase(
        context.pending_unorderable_back_propagation.begin());
    auto parentEdgesIt = context.inv_graph.find(type);
    if (parentEdgesIt == context.inv_graph.end()) {
      continue;
    }

    for (const t_type* parent : parentEdgesIt->second) {
      const bool parentWasConsideredOrderable =
          std::exchange(memo[parent], false);
      if (parentWasConsideredOrderable) {
        // Parent was previously considered orderable: need to back-propagate
        // the negative signal from it too.
        context.pending_unorderable_back_propagation.insert(parent);
      }
    }
    context.inv_graph.erase(parentEdgesIt);
  }
}

// whether type is/has set/map with custom types (cpp.Type or cpp.Adapter)
// Nested fields don't count.
bool has_custom_set_or_map(
    const t_type& type, std::unordered_set<const t_type*>& seen) {
  if (!seen.emplace(&type).second) {
    // We already saw the `type` previously. It can either be that
    // 1. we checked this type and it has no custom set/map,
    // 2. or the structure has recurisve types and we haven't finished
    //    checking this type yet and meet it again.
    //
    // Either way we can assume `type` is not a custom set or map since it
    // will be checked elsewhere.
    return false;
  }
  if (is_custom_type(type) &&
      (type.get_true_type()->is<t_set>() ||
       type.get_true_type()->is<t_map>())) {
    // Example:
    // @cpp.Type{template = "std::unordered_map"}
    // 1: map<i32, i32> foo;
    return true;
  }
  if (const t_typedef* asTypedef = type.try_as<t_typedef>()) {
    // Examples:
    // @cpp.Type{template = "std::unordered_set"}
    // typedef set<i32> CustomSet1;
    //
    // @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
    // typedef set<i32> CustomSet3;
    return has_custom_set_or_map(*asTypedef->type(), seen);
  }
  if (const t_list* asList = type.try_as<t_list>()) {
    // Examples:
    // 1: list<CustomSet1> foo;
    // In this case the structure is still not orderable.
    return has_custom_set_or_map(*asList->elem_type(), seen);
  }
  if (const t_set* asSet = type.try_as<t_set>()) {
    // Examples:
    // 1: set<CustomSet1> foo;
    return has_custom_set_or_map(*asSet->elem_type(), seen);
  }
  if (const t_map* asMap = type.try_as<t_map>()) {
    // Examples:
    // 1: map<i32, CustomSet1> foo;
    return has_custom_set_or_map(asMap->key_type().deref(), seen) ||
        has_custom_set_or_map(asMap->val_type().deref(), seen);
  }
  return false;
}

bool structure_has_custom_set_or_map_field(const t_structured& s) {
  std::unordered_set<const t_type*> seen;
  for (const t_field& field : s.fields()) {
    const t_type& field_true_type = *field.type().deref().get_true_type();
    if (field.has_structured_annotation(kCppTypeUri) &&
        (field_true_type.is<t_set>() || field_true_type.is<t_map>())) {
      return true;
    }
    if (has_custom_set_or_map(field.type().deref(), seen)) {
      return true;
    }
  }
  return false;
}

} // namespace

bool OrderableTypeUtils::is_orderable(
    const t_structured& structured_type,
    bool enableCustomTypeOrderingIfStructureHasUri) {
  // Thrift struct could self-reference, so have to perform a two-stage walk:
  std::unordered_map<const t_type*, bool> memo;
  return is_orderable(
      memo, structured_type, enableCustomTypeOrderingIfStructureHasUri);
}

bool OrderableTypeUtils::is_orderable(
    std::unordered_map<const t_type*, bool>& memo,
    const t_structured& structured_type,
    bool enableCustomTypeOrderingIfStructureHasUri) {
  // Thrift struct could self-reference, so have to perform a two-stage walk:
  // first all self-references are speculated, then negative classification is
  // back-propagated through the traversed dependencies.
  is_orderable_walk_context context{enableCustomTypeOrderingIfStructureHasUri};

  // NOTE: The initial value of `forceCustomTypeOrderable` passed below is
  // ignored: it is merely a placeholder for recursive calls. Since the first
  // call is necessarily on a t_structured (i.e., `structured_type`), the
  // actual value of forceCustomTypeOrderable for recursive calls will be
  // determined based on annotations and properties of the structured type
  // (eg. @thrift.EnableCustomTypeOrdering).
  type_is_orderable_walk(
      memo,
      structured_type,
      nullptr /* prev */,
      context,
      false /* forceCustomTypeOrderable, ignored (see above) */);
  back_propagate_unorderable_types(memo, context);
  auto it = memo.find(&structured_type);
  return it == memo.end() || it->second;
}

OrderableTypeUtils::StructuredOrderableCondition
OrderableTypeUtils::get_orderable_condition(
    const t_structured& structured_type,
    bool enableCustomTypeOrderingIfStructureHasUri) {
  const bool isOrderedWithoutImplicitCustomTypeOrdering = is_orderable(
      structured_type, false /* enableCustomTypeOrderingIfStructureHasUri */);
  const bool hasCustomSetOrMapField =
      structure_has_custom_set_or_map_field(structured_type);
  const bool hasEnableCustomTypeOrderingAnnotation =
      enable_custom_type_ordering(structured_type);

  if (isOrderedWithoutImplicitCustomTypeOrdering) {
    if (hasCustomSetOrMapField) {
      // Assumption violation: orderable type has custom set/map, but no
      // @cpp.EnableCustomTypeOrdering annotation
      assert(hasEnableCustomTypeOrderingAnnotation);
      return StructuredOrderableCondition::OrderableByExplicitAnnotation;
    } else {
      return StructuredOrderableCondition::Always;
    }
  }

  // structured_type is not ordered without implicit ordering (if URI is set).

  if (!enableCustomTypeOrderingIfStructureHasUri ||
      !is_orderable(
          structured_type,
          true /* enableCustomTypeOrderingIfStructureHasUri */)) {
    return StructuredOrderableCondition::NotOrderable;
  }

  // structured_type is orderable when implicit custom type ordering is enabled.
  //
  // 1. Struct does not have custom sets or maps:
  //   it was not orderable merely becaue of nested struct fields, that are now
  //   implicitly made orderable. The nested struct types need to be annotated,
  //   but this type does not (and, in fact, having the annotation would be
  //   redundant and lear to a validation error).
  //   => OrderableByNestedLegacyImplicitLogicEnabledByUri
  //
  // 2. Struct has custom sets or maps:
  //   a. Struct is annotated with @cpp.EnableCustomTypeOrdering. Since it was
  //      not orderable without implicit ordering enabled, this means there
  //      were also some nested fields that were unorderable without implicit
  //      ordering.
  //      => OrderableByExplicitAnnotationAndNestedLegacyImplicitLogic
  //   b. Struct is not annotated with @cpp.EnableCustomTypeOrdering. It
  //      therefore needs (at least) the annotation, since the type contains
  //      custom sets/maps. It may also need nested struct fields to be made
  //      explicitly orderable (see (a)), but needs the annotation nonetheless.
  //      => OrderableByLegacyImplicitLogicEnabledByUri

  if (!hasCustomSetOrMapField) {
    // Assumption violation: type is orderable implicitly due to URI, and does
    // not have custom set/map, so should not be annotated:
    assert(!hasEnableCustomTypeOrderingAnnotation);
    return StructuredOrderableCondition::
        OrderableByNestedLegacyImplicitLogicEnabledByUri;
  }

  // hasCustomSetOrMapField == true

  // Assumption violation: type is orderable implicitly due to URI, but URI is
  // empty.
  assert(!structured_type.uri().empty());

  return hasEnableCustomTypeOrderingAnnotation
      ? StructuredOrderableCondition::
            OrderableByExplicitAnnotationAndNestedLegacyImplicitLogic
      : StructuredOrderableCondition::
            OrderableByLegacyImplicitLogicEnabledByUri;
}

} // namespace apache::thrift::compiler::cpp2
