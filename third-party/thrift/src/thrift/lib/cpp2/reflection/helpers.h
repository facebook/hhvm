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

#ifndef THRIFT_FATAL_HELPERS_H_
#define THRIFT_FATAL_HELPERS_H_ 1

#include <fatal/type/get_type.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

#include <thrift/lib/cpp2/reflection/internal/helpers-inl-pre.h>

namespace apache {
namespace thrift {

/**
 * Gets the reflection metadata for a given struct's member based on a user
 * provided filter applied to each `reflected_struct_data_member` of the
 * struct's members.
 *
 * Example:
 *
 *  // Thrift file
 *  struct some_struct {
 *    123: i32 some_member
 *  }
 *
 *  // cpp file
 *  FATAL_S(member_name, "some_member");
 *
 *  using info = get_struct_member_by<
 *    some_struct,
 *    member_name,
 *    fatal::get_type::name
 *  >;
 *
 *  some_struct x;
 *  info::getter{}(x) = 10;
 *
 *  // prints `10`
 *  std::cout << x.some_member;
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename T, typename Key, typename Filter>
using get_struct_member_by =
    typename detail ::check_struct_has_member_for_criteria<
        T,
        fatal::find<typename reflect_struct<T>::members, Key, void, Filter>>::
        type;

/**
 * Gets the reflection metadata for a given struct's member based on its name.
 *
 * Example:
 *
 *  // Thrift file
 *  struct some_struct {
 *    123: i32 some_member
 *  }
 *
 *  // cpp file
 *  FATAL_S(member_name, "some_member");
 *
 *  using info = get_struct_member_by_name<some_struct, member_name>;
 *
 *  some_struct x;
 *  info::getter{}(x) = 10;
 *
 *  // prints `10`
 *  std::cout << x.some_member;
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename T, typename Name>
using get_struct_member_by_name =
    typename detail ::check_struct_has_member_for_criteria<
        T,
        fatal::find<
            typename reflect_struct<T>::members,
            Name,
            void,
            fatal::get_type::name>>::type;

/**
 * Gets the reflection metadata for a given struct's member based on its name.
 *
 * Example:
 *
 *  // Thrift file
 *  struct some_struct {
 *    123: i32 some_member
 *  }
 *
 *  // cpp file
 *  using info = get_struct_member_by_id<some_struct, 123>;
 *
 *  some_struct x;
 *  info::getter{}(x) = 10;
 *
 *  // prints `10`
 *  std::cout << x.some_member;
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename T, field_id_t Id>
using get_struct_member_by_id =
    typename detail ::check_struct_has_member_for_criteria<
        T,
        fatal::find<
            typename reflect_struct<T>::members,
            std::integral_constant<field_id_t, Id>,
            void,
            fatal::get_type::id>>::type;

} // namespace thrift
} // namespace apache

#endif // THRIFT_FATAL_HELPERS_H_
