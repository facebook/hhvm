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
include "thrift/annotation/java.thrift"
include "thrift/annotation/compat.thrift"

@thrift.Experimental
package "test.dev/thrift/lib/java/test/utf8"

namespace java.swift com.facebook.thrift.test.utf8

typedef string str

@compat.Strings{onInvalidUtf8 = compat.CodingErrorAction.Report}
typedef string UTF8StringReport

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
  typeClassName = "java.lang.Long",
}
@compat.Strings{onInvalidUtf8 = compat.CodingErrorAction.Report}
typedef string UTF8AdaptedStringReport

@compat.Strings{onInvalidUtf8 = compat.CodingErrorAction.Legacy}
typedef string UTF8String

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
  typeClassName = "java.lang.Long",
}
@compat.Strings{onInvalidUtf8 = compat.CodingErrorAction.Legacy}
typedef string UTF8AdaptedString

struct TestStruct {
  1: binary str_field;
  2: list<binary> str_list;
  3: set<binary> str_set;
  4: map<i32, binary> str_map;
  5: list<list<binary>> str_list_of_list;
  6: binary adapted_str;
  7: binary terse_str;
  8: map<binary, map<binary, binary>> complex_field;
  9: binary utf8_str;
  10: binary adapted_utf8_str;
}

struct TestStructString {
  1: string str_field;
  2: list<string> str_list;
  3: set<string> str_set;
  4: map<i32, string> str_map;
  5: list<list<string>> str_list_of_list;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
    typeClassName = "java.lang.Long",
  }
  6: string adapted_str;
  @thrift.TerseWrite
  7: string terse_str;
  8: map<string, map<string, string>> complex_field;
  9: UTF8StringReport utf8_str;
  10: UTF8AdaptedStringReport adapted_utf8_str;
}

@compat.Utf8
struct Utf8TestStructReport {
  1: optional string str_field;
  2: optional list<string> str_list;
  3: optional set<string> str_set;
  4: optional map<i32, string> str_map;
  5: optional list<list<string>> str_list_of_list;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
    typeClassName = "java.lang.Long",
  }
  6: string adapted_str;
  @thrift.TerseWrite
  7: string terse_str;
  8: map<string, map<string, string>> complex_field;
  9: UTF8StringReport utf8_str;
  10: UTF8AdaptedStringReport adapted_utf8_str;
}

@compat.Strings{onInvalidUtf8 = compat.CodingErrorAction.Legacy}
struct Utf8TestStructLegacy {
  1: str str_field;
  2: list<str> str_list;
  3: set<str> str_set;
  4: map<i32, str> str_map;
  5: list<list<str>> str_list_of_list;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
    typeClassName = "java.lang.Long",
  }
  6: string adapted_str;
  @thrift.TerseWrite
  7: str terse_str;
  8: map<str, map<str, str>> complex_field;
  @compat.Strings{onInvalidUtf8 = compat.CodingErrorAction.Report}
  9: UTF8String utf8_str;
  10: UTF8AdaptedString adapted_utf8_str;
}

struct Utf8TestStructBin {
  1: string_5433 str_field;
  2: list<string_7069> str_list;
  @compat.Strings{onInvalidUtf8 = compat.CodingErrorAction.Report}
  3: set<string_7069> str_set;
  4: map<i32, string_7069> str_map;
  5: list<list<string_7069>> str_list_of_list;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.common.UnpooledByteBufTypeAdapter",
    typeClassName = "io.netty.buffer.ByteBuf",
  }
  6: string_5433 adapted_str;
  @thrift.TerseWrite
  7: string_5433 terse_str;
  8: map<string_7069, map<string_7069, string_7069>> complex_field;
}

union TestUnion {
  1: binary field1;
  2: binary field2;
  3: binary utf8_str;
  4: binary adapted_utf8_str;
}

union TestUnionCompat {
  @compat.Strings
  1: string field1;
  @compat.Strings
  2: string field2;
  3: UTF8StringReport utf8_str;
  4: UTF8AdaptedStringReport adapted_utf8_str;
}

struct TestStructCompat {
  @compat.Strings
  1: string str_field;
  @compat.Strings
  8: map<
    UTF8StringReport,
    map<UTF8String, UTF8AdaptedStringReport>
  > complex_field;
  @compat.Strings{onInvalidUtf8 = compat.CodingErrorAction.Legacy}
  9: UTF8StringReport utf8_str;
}

transient exception TestException {
  1: i32 err_code;
  2: binary err_msg;
}

transient exception TestExceptionCompat {
  1: i32 err_code;
  @compat.Strings
  2: string err_msg;
}

// The following were automatically generated and may benefit from renaming.
@thrift.DeprecatedUnvalidatedAnnotations{
  items = {"java.swift.binary_string": "1"},
}
typedef string string_7069

// The following were automatically generated and may benefit from renaming.
@thrift.DeprecatedUnvalidatedAnnotations{
  items = {"java.swift.binary_string": "1"},
}
typedef string string_5433
