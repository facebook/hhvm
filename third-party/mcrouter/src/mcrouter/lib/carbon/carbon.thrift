/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace cpp2 carbon.thrift

cpp_include "<mcrouter/lib/carbon/Keys.h>"

typedef binary (
  cpp.type = "carbon::Keys<folly::IOBuf>",
  cpp.indirection,
) IOBufKey

typedef binary (
  cpp.type = "carbon::Keys<std::string>",
  cpp.indirection,
) StringKey
