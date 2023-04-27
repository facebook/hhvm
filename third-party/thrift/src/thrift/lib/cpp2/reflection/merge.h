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

#include <type_traits>
#include <utility>

#include <fatal/type/call_traits.h>
#include <fatal/type/conditional.h>
#include <fatal/type/transform.h>
#include <folly/Memory.h>
#include <folly/Range.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

#include <thrift/lib/cpp2/reflection/internal/merge-inl-pre.h>

namespace apache {
namespace thrift {

/***
 *  Merges `src` into `dst` using Thrift's static reflection support.
 *
 *  If `src` is non-const rvalue-ref, will move pieces of `src` into `dst`.
 *  Otherwise, will copy pieces of `src` into `dst`.
 *
 *  Recurses into the struct-typed fields of `src` and `dst`.
 *
 *  The documentation in thrift/lib/cpp2/reflection/reflection.h describes the
 * steps required in order to make static reflection metadata available for your
 *  thrift types. Be sure to read it. The metadata is not available by default.
 *
 *  Usage example:
 *
 *    MyStruct dst;
 *
 *    MyStruct src1 = //...
 *    apache::thrift::merge_into(src1, dst); // copy-style
 *
 *    MyStruct src2 = //...
 *    apache::thrift::merge_into(std::move(src2), dst); // move-style
 */
template <typename T>
void merge_into(T&& src, merge_into_detail::remove_const_reference<T>& dst);

} // namespace thrift
} // namespace apache

#include <thrift/lib/cpp2/reflection/internal/merge-inl-post.h>
