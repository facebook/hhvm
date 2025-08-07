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

include "thrift/annotation/thrift.thrift"

@thrift.Uri{value = "facebook.com/icsp/new_any/Basic"}
struct Basic {
  1: i64 x;
  2: string y;
  3: list<string> z;
}

@thrift.Uri{value = "facebook.com/icsp/new_any/SimpleUnion"}
union SimpleUnion {
  1: i64 x;
  2: Basic basic;
  3: list<Basic> list_basic;
}

@thrift.Uri{value = "facebook.com/icsp/new_any/SimpleEnum"}
enum SimpleEnum {
  VARIANT1 = 1,
  VARIANT2 = 2,
}

@thrift.Uri{value = "facebook.com/icsp/new_any/SimpleException"}
permanent client exception SimpleException {
  1: string message;
}

@thrift.Uri{value = "facebook.com/icsp/new_any/BasicType"}
struct BasicType {
  1: i64 x;
}

@thrift.AllowLegacyTypedefUri
@thrift.Uri{value = "facebook.com/icsp/new_any/SimpleTypeDef"}
typedef BasicType SimpleTypeDef
