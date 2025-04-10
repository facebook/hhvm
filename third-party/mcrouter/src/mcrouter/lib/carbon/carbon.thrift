/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace cpp2 carbon.thrift
namespace py3 carbon.thrift

cpp_include "<mcrouter/lib/carbon/Keys.h>"

include "thrift/annotation/cpp.thrift"

@cpp.Adapter{name = "::carbon::KeysAdapter"}
typedef IOBuf IOBufKey

@cpp.Adapter{name = "::carbon::KeysAdapter"}
typedef binary StringKey

@cpp.Type{name = "uint8_t"}
typedef byte ui8

@cpp.Type{name = "uint16_t"}
typedef i16 ui16

@cpp.Type{name = "uint32_t"}
typedef i32 ui32

@cpp.Type{name = "uint64_t"}
typedef i64 ui64

@cpp.Type{name = "folly::IOBuf"}
typedef binary IOBuf
