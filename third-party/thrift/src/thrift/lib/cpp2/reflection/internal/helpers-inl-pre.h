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

#ifndef THRIFT_FATAL_HELPERS_INL_PRE_H_
#define THRIFT_FATAL_HELPERS_INL_PRE_H_ 1

#include <fatal/type/find.h>

namespace apache::thrift::detail {

template <
    typename,
    typename T,
    bool StructHasMemberForCriteria = !std::is_same<void, T>::value>
struct check_struct_has_member_for_criteria {
  using type = T;

  static_assert(
      StructHasMemberForCriteria,
      "no struct member found for the given criteria");
};

} // namespace apache::thrift::detail

#endif // THRIFT_FATAL_HELPERS_INL_PRE_H_
