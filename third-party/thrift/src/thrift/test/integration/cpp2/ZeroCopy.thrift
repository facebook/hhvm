/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

include "common/fb303/if/fb303.thrift"

namespace cpp thrift.zerocopy
namespace cpp2 thrift.zerocopy.cpp2

cpp_include "folly/io/IOBuf.h"

typedef binary (cpp2.type = "folly::IOBuf") IOBuf

service ZeroCopyService extends fb303.FacebookService {
  IOBuf echo(1: IOBuf data);
}
