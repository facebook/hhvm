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

#include <folly/Memory.h>
#include <folly/Range.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp2/Thrift.h>

namespace apache::thrift {

/***
 *  Merges `src` into `dst` using Thrift's static reflection support.
 *
 *  CAUTION: this function has a number of surprising behaviors listed below.
 *
 *  If `src` is non-const rvalue-ref, will move pieces of `src` into `dst`.
 *  Otherwise, will copy pieces of `src` into `dst`.
 *
 *  Recurses into the struct-typed fields of `src` and `dst`, unless those
 *  fields have a `cpp.Ref` annotation in which case they are simply
 *  overwritten!
 *
 *  Does not recurse into union-typed fields of `src` and `dst`; these are
 *  simply overwritten!
 *
 *  Combines lists and sets by appending the elements of `src` to the elements
 *  of `dst` (unless annotated `cpp.Ref`).
 *
 *  Combines maps by recursively merging the key-value pairs of `src` into the
 *  key-value pairs of `dst` (unless annotated `cpp.Ref`).
 *
 *  Optional struct-typed fields that are not set in the destination will have
 *  source merged into any custom defaults rather than simply copied in, unless
 *  the field has a `thrift.Box` annotation!
 *
 *
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
void merge_into(T&& src, folly::remove_cvref_t<T>& dst);

} // namespace apache::thrift

#include <thrift/lib/cpp2/reflection/internal/merge-inl-post.h>
