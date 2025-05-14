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
  std::unordered_set<const t_type*> pending_back_propagation = {};
  std::unordered_set<const t_type*> seen = {};
  std::unordered_map<const t_type*, std::unordered_set<const t_type*>>
      inv_graph = {};
};

bool is_orderable_walk(
    std::unordered_map<const t_type*, bool>& memo,
    const t_type& type,
    const t_type* prev,
    is_orderable_walk_context& context,
    bool forceCustomTypeOrderable) {
  const bool has_disqualifying_annotation =
      is_custom_type(type) && !forceCustomTypeOrderable;
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
  if (type.is_primitive_type() || type.is_enum()) {
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
    const auto& real = [&]() -> auto&& { return *type.get_true_type(); };
    const auto& next = *(dynamic_cast<const t_typedef&>(type).get_type());
    return result = is_orderable_walk(
                        memo, next, &type, context, forceCustomTypeOrderable) &&
        (!(real().is_set() || real().is_map()) ||
         !has_disqualifying_annotation);
  } else if (const auto* as_struct = dynamic_cast<const t_structured*>(&type)) {
    return result = std::all_of(
               as_struct->fields().begin(),
               as_struct->fields().end(),
               [&](const auto& f) {
                 return is_orderable_walk(
                     memo,
                     f.type().deref(),
                     &type,
                     context,
                     enable_custom_type_ordering(*as_struct) ||
                         (context.enableCustomTypeOrderingIfStructureHasUri &&
                          !as_struct->uri().empty()));
               });
  } else if (type.is_list()) {
    return result = is_orderable_walk(
               memo,
               *(dynamic_cast<const t_list&>(type).get_elem_type()),
               &type,
               context,
               forceCustomTypeOrderable);
  } else if (type.is_set()) {
    return result = !has_disqualifying_annotation &&
        is_orderable_walk(
               memo,
               *(dynamic_cast<const t_set&>(type).get_elem_type()),
               &type,
               context,
               forceCustomTypeOrderable);
  } else if (type.is_map()) {
    return result = !has_disqualifying_annotation &&
        is_orderable_walk(
               memo,
               *(dynamic_cast<const t_map&>(type).get_key_type()),
               &type,
               context,
               forceCustomTypeOrderable) &&
        is_orderable_walk(
               memo,
               *(dynamic_cast<const t_map&>(type).get_val_type()),
               &type,
               context,
               forceCustomTypeOrderable);
  }
  return false;
}

void is_orderable_back_propagate(
    std::unordered_map<const t_type*, bool>& memo,
    is_orderable_walk_context& context) {
  while (!context.pending_back_propagation.empty()) {
    auto type = *context.pending_back_propagation.begin();
    context.pending_back_propagation.erase(
        context.pending_back_propagation.begin());
    auto edges = context.inv_graph.find(type);
    if (edges == context.inv_graph.end()) {
      continue;
    }
    for (const t_type* prev : edges->second) {
      if (std::exchange(memo[prev], false)) {
        context.pending_back_propagation.insert(prev);
      }
    }
    context.inv_graph.erase(edges);
  }
}
} // namespace

bool OrderableTypeUtils::is_orderable(
    const t_type& type, bool enableCustomTypeOrderingIfStructureHasUri) {
  // Thrift struct could self-reference, so have to perform a two-stage walk:
  std::unordered_map<const t_type*, bool> memo;
  return is_orderable(memo, type, enableCustomTypeOrderingIfStructureHasUri);
}

bool OrderableTypeUtils::is_orderable(
    std::unordered_map<const t_type*, bool>& memo,
    const t_type& type,
    bool enableCustomTypeOrderingIfStructureHasUri) {
  // Thrift struct could self-reference, so have to perform a two-stage walk:
  // first all self-references are speculated, then negative classification is
  // back-propagated through the traversed dependencies.
  is_orderable_walk_context context{enableCustomTypeOrderingIfStructureHasUri};
  is_orderable_walk(memo, type, nullptr, context, false);
  is_orderable_back_propagate(memo, context);
  auto it = memo.find(&type);
  return it == memo.end() || it->second;
}

} // namespace apache::thrift::compiler::cpp2
