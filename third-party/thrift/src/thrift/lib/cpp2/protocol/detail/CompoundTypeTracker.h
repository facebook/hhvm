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

#include <variant>

#include <folly/small_vector.h>

namespace apache::thrift::json5::detail {

/**
 * Tracks nested compound types (list/set/map/struct) during JSON
 * serialization/deserialization.
 *
 * Maintains a stack to track which compound type we are currently inside.
 * For maps, tracks additional state including the map form (Object or
 * KeyValueArray) and whether we're expecting a key or value.
 */
class CompoundTypeTracker final {
 public:
  /// Determines the JSON encoding form for this map:
  /// - Object: encoded as JSON object.
  /// - KeyValueArray: encoded as array of {key, value} objects.
  enum class MapForm { Object, KeyValueArray };

  /// Indicates whether we're expecting the next write to be a key or value.
  /// This determines whether primitive writes (e.g., writeI32) should output
  /// the value directly or as an object property name.
  enum class MapToken { Key, Value };

  /// Tracks state for serializing a Thrift map. Maps require additional state
  /// to handle key-value alternation and complex key encoding.
  struct MapState final {
    MapToken expectingToken = MapToken::Key;
    MapForm encodedAs = MapForm::Object;
  };

  // Container lifecycle
  void beginList();
  void beginSet();
  void beginMap(MapForm form);
  void beginStruct();

  void endList();
  void endSet();
  void endMap();
  void endStruct();

  // Map state access
  const MapState* mapState() const;
  bool inMap() const;
  bool inObjectMap() const;
  bool inObjectMapExpectingKey() const;

  // Map token manipulation
  void toggleExpectingKeyValue();

 private:
  /// Sentinel value that stays at the bottom of the container stack.
  /// This allows mapState() to safely call containerStack_.back() without
  /// checking if the stack is empty.
  struct Sentry final {};

  /// Tracks state for serializing a Thrift list. Lists are encoded as JSON
  /// arrays.
  struct List final {};

  /// Tracks state for serializing a Thrift set. Sets are encoded as JSON
  /// arrays (same as lists).
  struct Set final {};

  /// Tracks state for serializing a Thrift struct. Structs are encoded as JSON
  /// objects with field names as keys.
  struct Struct final {};

  using Map = MapState;

  /// Stack tracking the current nesting of containers during serialization.
  /// Each container begin (writeStructBegin, writeListBegin, etc.) pushes an
  /// entry, and each corresponding end method pops it. This enables proper
  /// handling of nested structures and map key/value state transitions.
  using ContainerType = std::variant<Sentry, List, Set, Map, Struct>;
  folly::small_vector<ContainerType, 4> containerStack_ = {Sentry{}};
};

} // namespace apache::thrift::json5::detail
