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

@cpp.Type{name = "carbon::Keys<folly::IOBuf>"}
typedef binary (cpp.indirection) IOBufKey

@cpp.Type{name = "carbon::Keys<std::string>"}
typedef binary (cpp.indirection) StringKey
