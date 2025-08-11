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

include "thrift/annotation/scope.thrift"

package "facebook.com/thrift/annotation/go"

namespace java com.facebook.thrift.annotation.go_deprecated
namespace android com.facebook.thrift.annotation.go_deprecated
namespace js thrift.annotation.go
namespace py.asyncio facebook_thrift_asyncio.annotation.go
namespace go thrift.annotation.go
namespace py thrift.annotation.go

// start

// Annotation for overriding Go names (e.g. to avoid codegen name conflicts).
@scope.Field
@scope.Function
@scope.FunctionParameter
@scope.Typedef
struct Name {
  1: string name;
}

// Annotation for overriding Go struct field tags (e.g. json, yaml, serf tags).
@scope.Field
@scope.FunctionParameter
struct Tag {
  1: string tag;
}

/**
  This annotation enables reordering of fields in the generated Go structs to minimize padding.
  This is achieved by placing the fields in the order of decreasing alignments.
  The order of fields with the same alignment is preserved.

  ```
  @go.MinimizePadding
  struct Padded {
    1: byte small
    2: i64 big
    3: i16 medium
    4: i32 biggish
    5: byte tiny
  }
  ```

  For example, the Go fields for the `Padded` Thrift struct above will be generated in the following order:

  ```
  int64 big;
  int32 biggish;
  int16 medium;
  int8 small;
  int8 tiny;
  ```

  which gives the size of 16 bytes compared to 32 bytes if `go.MinimizePadding` was not specified.
*/
@scope.Structured
struct MinimizePadding {}

/**
  This annotation enables reflect-based encoding/decoding of the given struct.
  Rather than generating long manual code for encoding/decoding a struct,
  the struct would be encoded/decoded using a generic reflect-based encoder/decoder.

  This results in fewer lines of code in the resulting codegen, which in turn makes
  the compilation faster and makes the resulting binary smaller. The performance of
  encoding/decoding becomes a little bit slower as a trade off.

*/
@scope.Structured
struct UseReflectCodec {}
