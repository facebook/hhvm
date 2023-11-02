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

#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

namespace apache {
namespace thrift {
namespace compiler {

class t_node;

// Stores associations from an (unowned) t_named::name() to a const T& node.
//
// Unlike inserting into a std::map, putting into a name_index replaces (and
// returns) any existing entry. This is ideal for naming contexts/scopes, where
// an inner name should hide any defined in an outer context.
//
// Like std::string_view and std::span, name_index does not own the data it
// refers to, so all names and nodes in the name_index, must out live the map
// itself.
template <typename T>
class name_index {
  static_assert(std::is_base_of<t_node, T>::value, "T must be an AST node.");

 public:
  // Adds the given t_named node, under its own name, returning any replaced
  // value or nullptr.
  //
  // `named` must live the name_index entry.
  const T* put(const T& named) { return put(named.name(), named); }
  const T* put(T&&) = delete; // Must be an l-value.

  // Adds the given node, under the given name, returning any replaced value or
  // nullptr.
  //
  // Both `name` and `node` must out live the name_index entry. `name` must also
  // not change while in the name_index.
  const T* put(std::string_view name, const T& node);
  // Must be l-values.
  const T* put(std::string&& name, const T& node) = delete;
  const T* put(std::string_view name, T&& node) = delete;

  // Adds all the entries from other, replacing any existing entries.
  void put_all(const name_index& other);

  // Removes all entries from the map.
  void clear() { index_.clear(); }

  // Returns true iff the given name is in the map.
  bool contains(std::string_view name) const {
    return index_.find(name) != index_.end();
  }

  // Returns the node associated with the given name, or nullptr.
  const T* find(std::string_view name) const;

  // Calls the given function with `(std::string_view name, const T& node)`,
  // for each entry in the name_index.
  // TODO(afuller): Generalize the iterator adapter in detail/view.h and expose
  // them here.
  template <typename F>
  void for_each(const F& cb) const {
    for (const auto& entry : index_) {
      cb(entry.first, *entry.second);
    }
  }

 private:
  std::unordered_map<std::string_view, const T*> index_;
};

template <typename T>
const T* name_index<T>::put(std::string_view name, const T& node) {
  auto key = std::ref(name);
  const T* result = nullptr;
  auto itr = index_.find(key);
  if (itr != index_.end()) {
    result = itr->second;
    // Explicitly remove any old entry to be sure the map key's reference is
    // updated.
    index_.erase(itr);
  }
  index_[key] = &node;
  return result;
}

template <typename T>
void name_index<T>::put_all(const name_index& other) {
  for (const auto& entry : other.index_) {
    put(entry.first, *entry.second);
  }
}

template <typename T>
const T* name_index<T>::find(std::string_view name) const {
  auto itr = index_.find(name);
  if (itr != index_.end()) {
    return itr->second;
  }
  return nullptr;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
