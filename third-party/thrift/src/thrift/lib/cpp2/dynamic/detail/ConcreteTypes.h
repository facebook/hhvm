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

// X macro pattern for all possible types that can be in a Set/Map
// This macro is used for explicit template instantiations
#define FBTHRIFT_DATUM_CONCRETE_TYPES(X) \
  X(bool)                                \
  X(int8_t)                              \
  X(int16_t)                             \
  X(int32_t)                             \
  X(int64_t)                             \
  X(float)                               \
  X(double)                              \
  X(String)                              \
  X(Binary)                              \
  X(Any)                                 \
  X(List)                                \
  X(Set)                                 \
  X(Map)                                 \
  X(Struct)                              \
  X(Union)

// Helper macro for 2D expansion (KeyType x ValueType) used by ConcreteMap
// Expands to X(KeyType, ValueType) for all combinations
#define FBTHRIFT_DATUM_CONCRETE_MAP_TYPES(X)            \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, bool)    \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, int8_t)  \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, int16_t) \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, int32_t) \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, int64_t) \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, float)   \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, double)  \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, String)  \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, Binary)  \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, Any)     \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, List)    \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, Set)     \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, Map)     \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, Struct)  \
  FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, Union)

#define FBTHRIFT_DATUM_CONCRETE_MAP_TYPES_FOR_KEY(X, KeyType) \
  X(KeyType, bool)                                            \
  X(KeyType, int8_t)                                          \
  X(KeyType, int16_t)                                         \
  X(KeyType, int32_t)                                         \
  X(KeyType, int64_t)                                         \
  X(KeyType, float)                                           \
  X(KeyType, double)                                          \
  X(KeyType, String)                                          \
  X(KeyType, Binary)                                          \
  X(KeyType, Any)                                             \
  X(KeyType, List)                                            \
  X(KeyType, Set)                                             \
  X(KeyType, Map)                                             \
  X(KeyType, Struct)                                          \
  X(KeyType, Union)
