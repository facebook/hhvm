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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

cpp_include "thrift/lib/py3/test/BinaryTypes.h"

const string unicode_str = "Saint Barthélemy";
const binary unicode_bytes = "Saint Barthélemy";

@cpp.Type{name = "folly::IOBuf"}
typedef binary IOBuf
@cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
typedef binary IOBufPtr
@cpp.Type{name = "folly::fbstring"}
typedef binary fbstring_type
@cpp.Type{name = "test::Buffer"}
typedef binary Buffer

@thrift.DeprecatedUnvalidatedAnnotations{items = {"cpp.noncomparable": "1"}}
union BinaryUnion {
  1: IOBuf iobuf_val;
}

struct Binaries {
  1: binary no_special_type;
  2: IOBuf iobuf_val;
  3: IOBufPtr iobuf_ptr;
  4: fbstring_type fbstring;
  5: Buffer nonstandard_type;
}

service BinaryService {
  Binaries sendRecvBinaries(1: Binaries val);
  binary sendRecvBinary(1: binary val);
  IOBuf sendRecvIOBuf(1: IOBuf val);
  IOBufPtr sendRecvIOBufPtr(1: IOBufPtr val);
  fbstring_type sendRecvFbstring(1: fbstring_type val);
  Buffer sendRecvBuffer(1: Buffer val);
  BinaryUnion sendRecBinaryUnion(1: BinaryUnion val);
}
