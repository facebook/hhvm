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

#include <thrift/lib/cpp2/dynamic/detail/ConcreteMap.h>
#include <thrift/lib/cpp2/dynamic/detail/ConcreteTypes.h>

namespace apache::thrift::dynamic::detail {

#define FBTHRIFT_INSTANTIATE_CONCRETE_MAP_FOR_VALUE(ValueType) \
  template class ConcreteMap<Map, ValueType>;

FBTHRIFT_DATUM_CONCRETE_TYPES(FBTHRIFT_INSTANTIATE_CONCRETE_MAP_FOR_VALUE)

#undef FBTHRIFT_INSTANTIATE_CONCRETE_MAP_FOR_VALUE

} // namespace apache::thrift::dynamic::detail
