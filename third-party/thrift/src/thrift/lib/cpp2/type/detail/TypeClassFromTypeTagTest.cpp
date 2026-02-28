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

#include <set>
#include <type_traits>
#include <thrift/lib/cpp2/type/detail/TypeClassFromTypeTag.h>

namespace apache::thrift::type_class {
struct Foo {};
static_assert(std::is_same_v<from_type_tag_t<type::bool_t>, integral>);
static_assert(std::is_same_v<from_type_tag_t<type::byte_t>, integral>);
static_assert(std::is_same_v<from_type_tag_t<type::i16_t>, integral>);
static_assert(std::is_same_v<from_type_tag_t<type::i32_t>, integral>);
static_assert(std::is_same_v<from_type_tag_t<type::i64_t>, integral>);
static_assert(std::is_same_v<from_type_tag_t<type::float_t>, floating_point>);
static_assert(std::is_same_v<from_type_tag_t<type::double_t>, floating_point>);
static_assert(std::is_same_v<from_type_tag_t<type::enum_t<Foo>>, enumeration>);
static_assert(std::is_same_v<from_type_tag_t<type::struct_t<Foo>>, structure>);
static_assert(std::is_same_v<from_type_tag_t<type::union_t<Foo>>, variant>);
static_assert(std::is_same_v<
              from_type_tag_t<type::list<type::set<type::struct_t<Foo>>>>,
              list<set<structure>>>);
static_assert(
    std::is_same_v<
        from_type_tag_t<type::map<type::string_t, type::list<type::i32_t>>>,
        map<string, list<integral>>>);
static_assert(std::is_same_v<
              from_type_tag_t<type::adapted<Foo, type::bool_t>>,
              integral>);

static_assert(
    std::is_same_v<
        remove_indirection_tag_t<map<
            string,
            list<detail::
                     indirection_tag<set<integral>, std::set<std::int32_t>>>>>,
        map<string, list<set<integral>>>>);

} // namespace apache::thrift::type_class
