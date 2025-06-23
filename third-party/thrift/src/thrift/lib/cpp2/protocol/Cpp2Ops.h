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

#include <stdint.h>

#include <folly/Traits.h>
#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/Thrift.h>

namespace apache::thrift {

/**
 * Class template (specialized for each type in generated code) that allows
 * access to write / read / serializedSize / serializedSizeZC functions in
 * a generic way.
 *
 * For native Cpp2 structs, one could call the corresponding methods
 * directly, but structs generated in compatibility mode (ie. typedef'ed
 * to the Thrift1 version) don't have them; they are defined as free
 * functions named <type>_read, <type>_write, etc, so they can't be accessed
 * generically (because the type name is part of the function name).
 *
 * Cpp2Ops bridges to either struct methods (for native Cpp2 structs)
 * or the corresponding free functions (for structs in compatibility mode).
 */
template <class T, class = void>
class Cpp2Ops {
  static_assert(
      folly::always_false<T>,
      "Only Thrift-generated classes are serializable.");
  //  When instantiated with a type T, includes:
  //
  //      template <class P>
  //      static uint32_t write(P*, const T*);
  //
  //      template <class P>
  //      static void read(P*, T*);
  //
  //      template <class P>
  //      static uint32_t serializedSize(P const*, T const*);
  //
  //      template <class P>
  //      static uint32_t serializedSizeZC(P const*, T const*);
  //
  //      static constexpr apache::thrift::protocol::TType thriftType();
};

} // namespace apache::thrift

#include <thrift/lib/cpp2/protocol/Cpp2Ops-inl.h>
