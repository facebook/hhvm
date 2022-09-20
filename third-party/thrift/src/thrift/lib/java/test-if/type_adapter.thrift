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

namespace java.swift com.facebook.thrift.test.adapter

include "thrift/annotation/java.thrift"

enum TestEnum {
  ZERO = 0,
  ONE = 1,
}

typedef i32 Integer

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.RetainedSlicedPooledByteBufTypeAdapter",
  typeClassName = "io.netty.buffer.ByteBuf",
}
typedef binary SlicedByteBuf

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.CopiedPooledByteBufTypeAdapter",
  typeClassName = "io.netty.buffer.ByteBuf",
}
typedef binary CopiedByteBuf

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.UnpooledByteBufTypeAdapter",
  typeClassName = "io.netty.buffer.ByteBuf",
}
typedef binary UnpooledByteBuf

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.DateTypeAdapter",
  typeClassName = "java.util.Date",
}
typedef i64 Date

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.BooleanToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef bool adaptedBoolean

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.ByteToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef byte adaptedByte

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.ShortToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef i16 adaptedShort

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.IntToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef Integer adaptedInt

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.LongToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef i64 adaptedLong

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.FloatToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef float adaptedFloat

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.DoubleToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef double adaptedDouble

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.StringToByteBufTypeAdapter",
  typeClassName = "io.netty.buffer.ByteBuf",
}
typedef string adaptedString

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.ListToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef list<i32> adaptedIntList

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.ListToHexTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef list<binary> adaptedBinaryList

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.ListToStringListTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef list<list<i32>> adaptedListIntList

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.IntSetToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef set<i32> adaptedIntSet

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.BinarySetToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef set<binary> adaptedBinarySet

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.IntMapToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef map<i32, i32> adaptedIntMap

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.IntBinaryMapToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef map<i32, binary> adaptedIntBinaryMap

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.IntStringMapToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef map<i32, string> adaptedIntStringMap

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.IntBinaryMapToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef map<i32, string (java.swift.binary_string)> adaptedIntBinaryStringMap

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.IntBinaryListMapToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef map<i32, list<binary>> adaptedIntBinaryListMap

typedef adaptedBoolean doubleTypedefBoolean
typedef adaptedInt doubleTypedefInt
typedef doubleTypedefInt multipleTypedefInt

struct TestStruct {
  1: bool boolean_field;
  2: byte byte_field;
  3: i16 short_field;
  4: i32 int_field;
  5: i64 long_field;
  6: float float_field;
  7: double double_field;
  8: string string_field;
  9: TestEnum testEnum_field;
  10: binary b1;
  11: binary b2;
  12: binary b3;
  13: i64 date_field;
  20: list<i32> intList_field;
  21: list<binary> binaryList_field;
  22: list<list<i32>> listIntList_field;
  23: set<i32> intSet_field;
  24: set<binary> binarySet_field;
  25: map<i32, i32> intMap_field;
  26: map<i32, binary> intBinaryMap_field;
  27: map<i32, string> intStringMap_field;
  28: map<i32, string (java.swift.binary_string)> intBinaryStringMap_field;
  29: map<i32, list<binary>> intBinaryListMap_field;
  51: bool boolean_default = true;
  52: byte byte_default = 9;
  53: i16 short_default = 101;
  54: i32 int_default = 1024;
  55: i64 long_default = 5000;
  56: float float_default = 2.3;
  57: double double_default = 5.67;
  58: string string_default = "test";
  59: list<i32> intList_default = [2, 4];
  60: set<i32> intSet_default = [10, 20];
  61: map<i32, i32> intMap_default = {1: 7};
  62: binary b1_default = "b1b1";
  63: binary b2_default = "b2b2";
  64: binary b3_default = "b3b3";
  65: list<binary> binaryList_default = ["aa", "bb"];
  66: map<i32, binary> intBinaryMap_default = {8: "foo"};
  67: set<binary> binarySet_default;
  68: map<i32, list<binary>> intBinaryListMap_default = {7: ["foo", "bar"]};
  101: optional bool optionalBoolean_field;
  102: optional binary optional_b1;
  201: bool boolean_field2;
  202: list<binary> binaryList_field2;
  204: i32 int_field2;
  205: i32 int_default2;
  206: i32 doubleTypedefInt_field;
  207: i32 multipleTypedefInt_field;
  208: i32 multipleTypedefInt_default;
}

// Adapted version of TestStruct.
struct AdaptedTestStruct {
  1: adaptedBoolean adaptedBoolean_field;
  2: adaptedByte adaptedByte_field;
  3: adaptedShort adaptedShort_field;
  4: adaptedInt adaptedInt_field;
  5: adaptedLong adaptedLong_field;
  6: adaptedFloat adaptedFloat_field;
  7: adaptedDouble adaptedDouble_field;
  8: adaptedString adaptedString_field;
  9: TestEnum testEnum_field;
  10: SlicedByteBuf b1;
  11: CopiedByteBuf b2;
  12: UnpooledByteBuf b3;
  13: Date date_field;
  20: adaptedIntList adaptedIntList_field;
  21: adaptedBinaryList adaptedBinaryList_field;
  22: adaptedListIntList adaptedListIntList_field;
  23: adaptedIntSet adaptedIntSet_field;
  24: adaptedBinarySet adaptedBinarySet_field;
  25: adaptedIntMap adaptedIntMap_field;
  26: adaptedIntBinaryMap adaptedIntBinaryMap_field;
  27: adaptedIntStringMap adaptedIntStringMap_field;
  28: adaptedIntBinaryStringMap adaptedIntBinaryStringMap_field;
  29: adaptedIntBinaryListMap adaptedIntBinaryListMap_field;
  51: adaptedBoolean adaptedBoolean_default = true;
  52: adaptedByte adaptedByte_default = 9;
  53: adaptedShort adaptedShort_default = 101;
  54: adaptedInt adaptedInt_default = 1024;
  55: adaptedLong adaptedLong_default = 5000;
  56: adaptedFloat adaptedFloat_default = 2.3;
  57: adaptedDouble adaptedDouble_default = 5.67;
  58: adaptedString adaptedString_default = "test";
  59: adaptedIntList adaptedIntList_default = [2, 4];
  60: adaptedIntSet adaptedIntSet_default = [10, 20];
  61: adaptedIntMap adaptedIntMap_default = {1: 7};
  62: SlicedByteBuf b1_default = "b1b1";
  63: CopiedByteBuf b2_default = "b2b2";
  64: UnpooledByteBuf b3_default = "b3b3";
  65: adaptedBinaryList adaptedBinaryList_default = ["aa", "bb"];
  66: adaptedIntBinaryMap adaptedIntBinaryMap_default = {8: "foo"};
  67: adaptedBinarySet adaptedBinarySet_default = ["foo", "bar"];
  68: adaptedIntBinaryListMap adaptedIntBinaryListMap_default = {
    7: ["foo", "bar"],
  };
  101: optional adaptedBoolean optionalAdaptedBoolean_field;
  102: optional SlicedByteBuf optional_b1;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.BooleanToStringTypeAdapter",
    typeClassName = "java.lang.String",
  }
  201: bool adaptedBoolean_field2;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.ListToHexTypeAdapter",
    typeClassName = "java.lang.String",
  }
  202: list<binary> adaptedBinaryList_field2;

  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
    typeClassName = "java.lang.Long",
  }
  204: adaptedInt doubleAdaptedInt_field;

  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
    typeClassName = "java.lang.Long",
  }
  205: adaptedInt doubleAdaptedInt_default = 3000;

  206: doubleTypedefInt doubleTypedefAdaptedInt_field;
  207: multipleTypedefInt multipleTypedefAdaptedInt_field;
  208: multipleTypedefInt multipleTypedefAdaptedInt_default = 50;
}

struct AdaptedTestStructWithoutDefaults {
  1: adaptedBoolean adaptedBoolean_field;
  2: adaptedByte adaptedByte_field;
  10: SlicedByteBuf b1;
  11: CopiedByteBuf b2;
  12: UnpooledByteBuf b3;
  13: Date date_field;
  20: adaptedIntList adaptedIntList_field;
  21: adaptedBinaryList adaptedBinaryList_field;
  24: adaptedBinarySet adaptedBinarySet_field;
  25: adaptedIntMap adaptedIntMap_field;
  26: adaptedIntBinaryMap adaptedIntBinaryMap_field;
  101: optional adaptedBoolean optionalAdaptedBoolean_field;
  102: optional SlicedByteBuf optional_b1;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.BooleanToStringTypeAdapter",
    typeClassName = "java.lang.String",
  }
  201: bool adaptedBoolean_field2;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.ListToHexTypeAdapter",
    typeClassName = "java.lang.String",
  }
  202: list<binary> adaptedBinaryList_field2;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
    typeClassName = "java.lang.Long",
  }
  204: adaptedInt doubleAdaptedInt_field;
  205: doubleTypedefInt doubleTypedefAdaptedInt_field;
}

union TestUnion {
  1: bool boolean_field;
  2: byte byte_field;
  3: i16 short_field;
  4: i32 int_field;
  5: i64 long_field;
  6: float float_field;
  7: double double_field;
  8: string string_field;
  //9: TestEnum testEnum_field;
  10: binary b1;
  11: binary b2;
  12: binary b3;
  13: i64 date_field;
  20: list<i32> intList_field;
  21: list<binary> binaryList_field;
  22: list<list<i32>> listIntList_field;
  23: set<i32> intSet_field;
  24: set<binary> binarySet_field;
  25: map<i32, i32> intMap_field;
  26: map<i32, binary> intBinaryMap_field;
  27: map<i32, string> intStringMap_field;
  28: map<i32, string (java.swift.binary_string)> intBinaryStringMap_field;
  29: map<i32, list<binary>> intBinaryListMap_field;
  201: bool boolean_field2;
  202: list<binary> binaryList_field2;
  204: i32 int_field2;
  205: i32 int_default2 = 3000;
  206: i32 doubleTypedefInt_field;
  207: i32 multipleTypedefInt_field;
}

// Adapted version of TestUnion.
union AdaptedTestUnion {
  1: adaptedBoolean adaptedBoolean_field;
  2: adaptedByte adaptedByte_field;
  3: adaptedShort adaptedShort_field;
  4: adaptedInt adaptedInt_field;
  5: adaptedLong adaptedLong_field;
  6: adaptedFloat adaptedFloat_field;
  7: adaptedDouble adaptedDouble_field;
  8: adaptedString adaptedString_field;
  //9: TestEnum testEnum_field;
  10: SlicedByteBuf b1;
  11: CopiedByteBuf b2;
  12: UnpooledByteBuf b3;
  13: Date date_field;
  20: adaptedIntList adaptedIntList_field;
  21: adaptedBinaryList adaptedBinaryList_field;
  22: adaptedListIntList adaptedListIntList_field;
  23: adaptedIntSet adaptedIntSet_field;
  24: adaptedBinarySet adaptedBinarySet_field;
  25: adaptedIntMap adaptedIntMap_field;
  26: adaptedIntBinaryMap adaptedIntBinaryMap_field;
  27: adaptedIntStringMap adaptedIntStringMap_field;
  28: adaptedIntBinaryStringMap adaptedIntBinaryStringMap_field;
  29: adaptedIntBinaryListMap adaptedIntBinaryListMap_field;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.BooleanToStringTypeAdapter",
    typeClassName = "java.lang.String",
  }
  201: bool adaptedBoolean_field2;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.ListToHexTypeAdapter",
    typeClassName = "java.lang.String",
  }
  202: list<binary> adaptedBinaryList_field2;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
    typeClassName = "java.lang.Long",
  }
  204: adaptedInt doubleAdaptedInt_field;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
    typeClassName = "java.lang.Long",
  }
  205: adaptedInt doubleAdaptedInt_default = 3000;
  206: doubleTypedefInt doubleTypedefAdaptedInt_field;
  207: multipleTypedefInt multipleTypedefAdaptedInt_field;
}
