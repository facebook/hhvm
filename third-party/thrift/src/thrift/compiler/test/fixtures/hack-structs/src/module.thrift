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

include "thrift/annotation/hack.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

@hack.StructTrait
struct MyStruct {
  1: string foo;
} (php.structtrait)

@hack.StructTrait
struct MySecondStruct {
  1: string foo;
}

@hack.StructTrait{name = '\\CustomTraitName'}
struct MyThirdStruct {
  1: string foo;
}

@hack.StructTrait
struct MyFourthStruct {
  1: string foo;
}

@hack.StructTrait{name = '\\CustomTraitName'}
struct MyFifthStruct {
  1: string foo;
}

@hack.StructAsTrait
struct MySixthStruct {
  1: string foo;
}

@hack.StructAsTrait
struct MySeventhStruct {
  1: string foo;
}
