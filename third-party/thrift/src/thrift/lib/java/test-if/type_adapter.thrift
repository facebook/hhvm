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
include "thrift/annotation/thrift.thrift"
include "thrift/lib/thrift/any.thrift"

@thrift.Experimental
package "facebook.com/thrift/test/adapter"

@java.Adapter{
  adapterClassName = "com.facebook.thrift.any.AnyAdapter",
  typeClassName = "com.facebook.thrift.any.Any",
}
typedef any.Any Any

enum TestEnum {
  ZERO = 0,
  ONE = 1,
}

typedef i32 MyInt
typedef MyInt Integer

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
typedef map<i32, string_5578> adaptedIntBinaryStringMap

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.IntBinaryListMapToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef map<i32, list<binary>> adaptedIntBinaryListMap

typedef adaptedBoolean doubleTypedefBoolean
typedef adaptedInt doubleTypedefInt
typedef doubleTypedefInt multipleTypedefInt

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.CopiedPooledByteBufTypeAdapter",
  typeClassName = "io.netty.buffer.ByteBuf",
}
typedef string (java.swift.binary_string) BinaryString

typedef binary data
@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.CopiedPooledByteBufTypeAdapter",
  typeClassName = "io.netty.buffer.ByteBuf",
}
typedef data CopiedByteBufData
typedef CopiedByteBufData NestedBinTypeDef

@java.Adapter{
  adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
  typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<List<String>>',
}
typedef list<adaptedInt> doubleAdaptedIntList

@java.Adapter{
  adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
  typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<com.facebook.thrift.adapter.test.Wrapped<List<String>>>',
}
typedef list<doubleAdaptedIntList> tripleAdaptedIntList
typedef tripleAdaptedIntList tripleAdaptedIntList1
typedef tripleAdaptedIntList1 tripleAdaptedIntList2

@java.Adapter{
  adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
  typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<com.facebook.thrift.adapter.test.Wrapped<com.facebook.thrift.adapter.test.Wrapped<List<String>>>>',
}
typedef list<tripleAdaptedIntList2> quadrupleAdaptedIntList

safe permanent client exception InnerException {
  1: i32 int_field;
}

@java.Adapter{
  adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
  typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<com.facebook.thrift.test.adapter.InnerException>',
}
typedef InnerException myException

union InnerUnion {
  1: i32 int_field;
}

@java.Adapter{
  adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
  typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<com.facebook.thrift.test.adapter.InnerUnion>',
}
typedef InnerUnion myUnion

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
  28: map<i32, string_5578> intBinaryStringMap_field;
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
  300: double generic_adapter_field;
  400: list<i32> listAdaptedInt_field;
  401: list<list<i32>> listOfListAdaptedInt_field;
  402: list<list<list<i32>>> nestedListAdaptedInt_field;
  403: set<i32> setAdaptedInt_field;
  406: list<set<i32>> listOfSetAdaptedInt_field;
  408: map<i32, i16> mapOfIntToShort_field;
  409: map<i32, map<i32, i16>> mapOfIntToMapIntToShort_field;
  410: map<i32, map<i32, map<i32, i16>>> nestedMapIntToShort_field;
  411: map<i32, map<i16, list<i16>>> nestedAdapted_field;

  471: list<bool> adaptedBooleanList_field;
  472: list<byte> adaptedByteList_field;
  473: list<i16> adaptedShortList_field;
  475: list<i64> adaptedLongList_field;
  476: list<float> adaptedFloatList_field;
  477: list<double> adaptedDoubleList_field;
  478: list<string> adaptedStringList_field;
  479: list<TestEnum> testEnumList_field;
  480: list<binary> adaptedBinList_field;
  481: list<list<binary>> adaptedBinList2_field;
  482: list<list<list<binary>>> adaptedBinList3_field;
  483: set<binary> adaptedBinSet_field;
  486: map<i32, binary> adaptedBinMap_field;
  487: map<i32, map<i32, binary>> adaptedBinMap2_field;
  488: map<i32, map<i32, map<binary, binary>>> adaptedBinMap3_field;
  489: list<list<binary>> adaptedBinStringList2_field;
  490: list<list<list<binary>>> adaptedBinStringList3_field;
  491: list<binary> adaptedBinNestedTypeDefList_field;
  492: list<set<i32>> adaptedSetList_field;
  493: list<list<set<i32>>> adaptedSetList2_field;
  494: list<list<list<set<i32>>>> adaptedSetList3_field;
  498: map<i32, set<i32>> adaptedMap7_field;
  499: map<i32, map<i32, set<i32>>> adaptedMap8_field;
  500: map<i32, map<i32, map<i32, set<i32>>>> adaptedMap9_field;
  501: list<i32> doubleAdaptedList_field;
  502: list<set<i32>> doubleAdaptedList2_field;
  503: list<binary> doubleAdaptedList3_field;
  504: map<i32, binary> doubleAdaptedMap1_field;
  506: list<Any> anyList_field;
  507: map<i32, list<Any>> anyMap_field;
  508: list<InnerUnion> unionList_field;
  509: list<InnerException> exceptionList_field;
  @thrift.TerseWrite
  510: list<i64> terseAdaptedLongList_field;
  511: optional list<i64> optionalAdaptedLongList_field;
  @thrift.TerseWrite
  512: list<set<i32>> terseAdaptedIntSetList_field;
  513: optional list<set<i32>> optionalAdaptedIntSetList_field;
  514: list<list<list<list<i32>>>> quadAdapted_field;
  515: map<i32, map<i16, list<list<list<list<i32>>>>>> quadAdaptedMap_field;
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

  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped',
  }
  300: double generic_adapter_field;

  400: list<adaptedInt> listAdaptedInt_field;
  401: list<list<adaptedInt>> listOfListAdaptedInt_field;
  402: list<list<list<adaptedInt>>> nestedListAdaptedInt_field;
  403: set<adaptedInt> setAdaptedInt_field;
  406: list<set<adaptedInt>> listOfSetAdaptedInt_field;
  408: map<adaptedInt, adaptedShort> mapOfIntToShort_field;
  409: map<
    adaptedInt,
    map<adaptedInt, adaptedShort>
  > mapOfIntToMapIntToShort_field;
  410: map<
    adaptedInt,
    map<adaptedInt, map<adaptedInt, adaptedShort>>
  > nestedMapIntToShort_field;
  411: map<
    adaptedInt,
    map<adaptedShort, list<adaptedShort>>
  > nestedAdapted_field;
  450: list<adaptedInt> listAdaptedInt_default = [5, 6, 7];
  451: list<list<adaptedInt>> listOfListAdaptedInt_default = [
    [5, 6, 7],
    [3, 4, 5],
  ];
  452: list<list<list<adaptedInt>>> nestedListAdaptedInt_default = [
    [[5, 6, 7], [3, 4, 5]],
  ];
  453: set<adaptedInt> setAdaptedInt_default = [5, 6, 7];
  458: map<adaptedInt, adaptedShort> mapOfIntToShort_default = {8: 9};
  459: map<
    adaptedInt,
    map<adaptedInt, adaptedShort>
  > mapOfIntToMapIntToShort_default = {7: {8: 9}};
  460: map<
    adaptedInt,
    map<adaptedInt, map<adaptedInt, adaptedShort>>
  > nestedMapIntToShort_default = {7: {8: {9: 10}}};
  461: map<
    adaptedInt,
    map<adaptedShort, list<adaptedShort>>
  > nestedAdapted_default = {7: {3: [1, 2]}};
  471: list<adaptedBoolean> adaptedBooleanList_field;
  472: list<adaptedByte> adaptedByteList_field;
  473: list<adaptedShort> adaptedShortList_field;
  475: list<adaptedLong> adaptedLongList_field;
  476: list<adaptedFloat> adaptedFloatList_field;
  477: list<adaptedDouble> adaptedDoubleList_field;
  478: list<adaptedString> adaptedStringList_field;

  479: list<TestEnum> testEnumList_field;
  480: list<SlicedByteBuf> adaptedBinList_field;
  481: list<list<SlicedByteBuf>> adaptedBinList2_field;
  482: list<list<list<SlicedByteBuf>>> adaptedBinList3_field;
  483: set<SlicedByteBuf> adaptedBinSet_field;
  486: map<i32, SlicedByteBuf> adaptedBinMap_field;
  487: map<i32, map<i32, SlicedByteBuf>> adaptedBinMap2_field;
  488: map<
    i32,
    map<i32, map<SlicedByteBuf, SlicedByteBuf>>
  > adaptedBinMap3_field;
  489: list<list<BinaryString>> adaptedBinStringList2_field;
  490: list<list<list<BinaryString>>> adaptedBinStringList3_field;
  491: list<NestedBinTypeDef> adaptedBinNestedTypeDefList_field;
  492: list<adaptedIntSet> adaptedSetList_field;
  493: list<list<adaptedIntSet>> adaptedSetList2_field;
  494: list<list<list<adaptedIntSet>>> adaptedSetList3_field;
  498: map<i32, adaptedIntSet> adaptedMap7_field;
  499: map<i32, map<i32, adaptedIntList>> adaptedMap8_field;
  500: map<i32, map<i32, map<i32, adaptedIntMap>>> adaptedMap9_field;

  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<List<String>>',
  }
  501: list<adaptedInt> doubleAdaptedList_field;
  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<List<String>>',
  }
  502: list<adaptedIntSet> doubleAdaptedList2_field;
  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<List<io.netty.buffer.ByteBuf>>',
  }
  503: list<SlicedByteBuf> doubleAdaptedList3_field;
  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<Map<String, io.netty.buffer.ByteBuf>>',
  }
  504: map<adaptedInt, SlicedByteBuf> doubleAdaptedMap1_field;
  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<com.facebook.thrift.adapter.test.Wrapped<List<String>>>',
  }
  505: doubleAdaptedIntList doubleAdaptedIntList2_field;
  506: list<Any> anyList_field;
  507: map<i32, list<Any>> anyMap_field;
  508: list<myUnion> unionList_field;
  509: list<myException> exceptionList_field;
  @thrift.TerseWrite
  510: list<adaptedLong> terseAdaptedLongList_field;
  511: optional list<adaptedLong> optionalAdaptedLongList_field;
  @thrift.TerseWrite
  512: list<adaptedIntSet> terseAdaptedIntSetList_field;
  513: optional list<adaptedIntSet> optionalAdaptedIntSetList_field;
  514: quadrupleAdaptedIntList quadAdapted_field;
  515: map<
    adaptedInt,
    map<adaptedShort, quadrupleAdaptedIntList>
  > quadAdaptedMap_field;
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
  28: map<i32, string_5578> intBinaryStringMap_field;
  29: map<i32, list<binary>> intBinaryListMap_field;
  201: bool boolean_field2;
  202: list<binary> binaryList_field2;
  204: i32 int_field2;
  205: i32 int_default2 = 3000;
  206: i32 doubleTypedefInt_field;
  207: i32 multipleTypedefInt_field;
  208: i32 multipleTypedefInt_default;
  300: double generic_adapter_field;
  400: list<i32> listAdaptedInt_field;
  401: list<list<i32>> listOfListAdaptedInt_field;
  402: list<list<list<i32>>> nestedListAdaptedInt_field;
  403: set<i32> setAdaptedInt_field;
  406: list<set<i32>> listOfSetAdaptedInt_field;
  408: map<i32, i16> mapOfIntToShort_field;
  409: map<i32, map<i32, i16>> mapOfIntToMapIntToShort_field;
  410: map<i32, map<i32, map<i32, i16>>> nestedMapIntToShort_field;
  411: map<i32, map<i16, list<i16>>> nestedAdapted_field;

  471: list<bool> adaptedBooleanList_field;
  472: list<byte> adaptedByteList_field;
  473: list<i16> adaptedShortList_field;
  475: list<i64> adaptedLongList_field;
  476: list<float> adaptedFloatList_field;
  477: list<double> adaptedDoubleList_field;
  478: list<string> adaptedStringList_field;
  479: list<TestEnum> testEnumList_field;
  480: list<binary> adaptedBinList_field;
  481: list<list<binary>> adaptedBinList2_field;
  482: list<list<list<binary>>> adaptedBinList3_field;
  483: set<binary> adaptedBinSet_field;
  486: map<i32, binary> adaptedBinMap_field;
  487: map<i32, map<i32, binary>> adaptedBinMap2_field;
  488: map<i32, map<i32, map<binary, binary>>> adaptedBinMap3_field;
  489: list<list<binary>> adaptedBinStringList2_field;
  490: list<list<list<binary>>> adaptedBinStringList3_field;
  491: list<binary> adaptedBinNestedTypeDefList_field;
  492: list<set<i32>> adaptedSetList_field;
  493: list<list<set<i32>>> adaptedSetList2_field;
  494: list<list<list<set<i32>>>> adaptedSetList3_field;
  498: map<i32, set<i32>> adaptedMap7_field;
  499: map<i32, map<i32, set<i32>>> adaptedMap8_field;
  500: map<i32, map<i32, map<i32, set<i32>>>> adaptedMap9_field;
  501: list<i32> doubleAdaptedList_field;
  502: list<set<i32>> doubleAdaptedList2_field;
  503: list<binary> doubleAdaptedList3_field;
  504: map<i32, binary> doubleAdaptedMap1_field;
  506: list<Any> anyList_field;
  507: map<i32, list<Any>> anyMap_field;
  508: list<InnerUnion> unionList_field;
  509: list<InnerException> exceptionList_field;
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

  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped',
  }
  300: double generic_adapter_field;

  400: list<adaptedInt> listAdaptedInt_field;
  401: list<list<adaptedInt>> listOfListAdaptedInt_field;
  402: list<list<list<adaptedInt>>> nestedListAdaptedInt_field;
  403: set<adaptedInt> setAdaptedInt_field;
  406: list<set<adaptedInt>> listOfSetAdaptedInt_field;
  408: map<adaptedInt, adaptedShort> mapOfIntToShort_field;
  409: map<
    adaptedInt,
    map<adaptedInt, adaptedShort>
  > mapOfIntToMapIntToShort_field;
  410: map<
    adaptedInt,
    map<adaptedInt, map<adaptedInt, adaptedShort>>
  > nestedMapIntToShort_field;
  411: map<
    adaptedInt,
    map<adaptedShort, list<adaptedShort>>
  > nestedAdapted_field;
  450: list<adaptedInt> listAdaptedInt_default = [5, 6, 7];
  451: list<list<adaptedInt>> listOfListAdaptedInt_default = [
    [5, 6, 7],
    [3, 4, 5],
  ];
  452: list<list<list<adaptedInt>>> nestedListAdaptedInt_default = [
    [[5, 6, 7], [3, 4, 5]],
  ];
  453: set<adaptedInt> setAdaptedInt_default = [5, 6, 7];
  458: map<adaptedInt, adaptedShort> mapOfIntToShort_default = {8: 9};
  459: map<
    adaptedInt,
    map<adaptedInt, adaptedShort>
  > mapOfIntToMapIntToShort_default = {7: {8: 9}};
  460: map<
    adaptedInt,
    map<adaptedInt, map<adaptedInt, adaptedShort>>
  > nestedMapIntToShort_default = {7: {8: {9: 10}}};
  461: map<
    adaptedInt,
    map<adaptedShort, list<adaptedShort>>
  > nestedAdapted_default = {7: {3: [1, 2]}};
  471: list<adaptedBoolean> adaptedBooleanList_field;
  472: list<adaptedByte> adaptedByteList_field;
  473: list<adaptedShort> adaptedShortList_field;
  475: list<adaptedLong> adaptedLongList_field;
  476: list<adaptedFloat> adaptedFloatList_field;
  477: list<adaptedDouble> adaptedDoubleList_field;
  478: list<adaptedString> adaptedStringList_field;
  479: list<TestEnum> testEnumList_field;
  480: list<SlicedByteBuf> adaptedBinList_field;
  481: list<list<SlicedByteBuf>> adaptedBinList2_field;
  482: list<list<list<SlicedByteBuf>>> adaptedBinList3_field;
  483: set<SlicedByteBuf> adaptedBinSet_field;
  486: map<i32, SlicedByteBuf> adaptedBinMap_field;
  487: map<i32, map<i32, SlicedByteBuf>> adaptedBinMap2_field;
  488: map<
    i32,
    map<i32, map<SlicedByteBuf, SlicedByteBuf>>
  > adaptedBinMap3_field;
  489: list<list<BinaryString>> adaptedBinStringList2_field;
  490: list<list<list<BinaryString>>> adaptedBinStringList3_field;
  491: list<NestedBinTypeDef> adaptedBinNestedTypeDefList_field;
  492: list<adaptedIntSet> adaptedSetList_field;
  493: list<list<adaptedIntSet>> adaptedSetList2_field;
  494: list<list<list<adaptedIntSet>>> adaptedSetList3_field;
  498: map<i32, adaptedIntSet> adaptedMap7_field;
  499: map<i32, map<i32, adaptedIntList>> adaptedMap8_field;
  500: map<i32, map<i32, map<i32, adaptedIntMap>>> adaptedMap9_field;

  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<List<String>>',
  }
  501: list<adaptedInt> doubleAdaptedList_field;
  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<List<String>>',
  }
  502: list<adaptedIntSet> doubleAdaptedList2_field;
  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<List<io.netty.buffer.ByteBuf>>',
  }
  503: list<SlicedByteBuf> doubleAdaptedList3_field;
  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<Map<String, io.netty.buffer.ByteBuf>>',
  }
  504: map<adaptedInt, SlicedByteBuf> doubleAdaptedMap1_field;

  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<com.facebook.thrift.adapter.test.Wrapped<List<String>>>',
  }
  505: doubleAdaptedIntList doubleAdaptedIntList2_field;
  506: list<Any> anyList_field;
  507: map<i32, list<Any>> anyMap_field;
  508: list<myUnion> unionList_field;
  509: list<myException> exceptionList_field;
  510: list<adaptedLong> terseAdaptedLongList_field;
  514: quadrupleAdaptedIntList quadAdapted_field;
  515: map<
    adaptedInt,
    map<adaptedShort, quadrupleAdaptedIntList>
  > quadAdaptedMap_field;
}

// Adapted version of TestStruct.
safe permanent client exception AdaptedTestException {
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

  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped',
  }
  300: double generic_adapter_field;

  400: list<adaptedInt> listAdaptedInt_field;
  401: list<list<adaptedInt>> listOfListAdaptedInt_field;
  402: list<list<list<adaptedInt>>> nestedListAdaptedInt_field;
  403: set<adaptedInt> setAdaptedInt_field;
  406: list<set<adaptedInt>> listOfSetAdaptedInt_field;
  408: map<adaptedInt, adaptedShort> mapOfIntToShort_field;
  409: map<
    adaptedInt,
    map<adaptedInt, adaptedShort>
  > mapOfIntToMapIntToShort_field;
  410: map<
    adaptedInt,
    map<adaptedInt, map<adaptedInt, adaptedShort>>
  > nestedMapIntToShort_field;
  411: map<
    adaptedInt,
    map<adaptedShort, list<adaptedShort>>
  > nestedAdapted_field;
  450: list<adaptedInt> listAdaptedInt_default = [5, 6, 7];
  451: list<list<adaptedInt>> listOfListAdaptedInt_default = [
    [5, 6, 7],
    [3, 4, 5],
  ];
  452: list<list<list<adaptedInt>>> nestedListAdaptedInt_default = [
    [[5, 6, 7], [3, 4, 5]],
  ];
  453: set<adaptedInt> setAdaptedInt_default = [5, 6, 7];
  458: map<adaptedInt, adaptedShort> mapOfIntToShort_default = {8: 9};
  459: map<
    adaptedInt,
    map<adaptedInt, adaptedShort>
  > mapOfIntToMapIntToShort_default = {7: {8: 9}};
  460: map<
    adaptedInt,
    map<adaptedInt, map<adaptedInt, adaptedShort>>
  > nestedMapIntToShort_default = {7: {8: {9: 10}}};
  461: map<
    adaptedInt,
    map<adaptedShort, list<adaptedShort>>
  > nestedAdapted_default = {7: {3: [1, 2]}};
  471: list<adaptedBoolean> adaptedBooleanList_field;
  472: list<adaptedByte> adaptedByteList_field;
  473: list<adaptedShort> adaptedShortList_field;
  475: list<adaptedLong> adaptedLongList_field;
  476: list<adaptedFloat> adaptedFloatList_field;
  477: list<adaptedDouble> adaptedDoubleList_field;
  478: list<adaptedString> adaptedStringList_field;
  479: list<TestEnum> testEnumList_field;
  480: list<SlicedByteBuf> adaptedBinList_field;
  481: list<list<SlicedByteBuf>> adaptedBinList2_field;
  482: list<list<list<SlicedByteBuf>>> adaptedBinList3_field;
  483: set<SlicedByteBuf> adaptedBinSet_field;
  486: map<i32, SlicedByteBuf> adaptedBinMap_field;
  487: map<i32, map<i32, SlicedByteBuf>> adaptedBinMap2_field;
  488: map<
    i32,
    map<i32, map<SlicedByteBuf, SlicedByteBuf>>
  > adaptedBinMap3_field;
  489: list<list<BinaryString>> adaptedBinStringList2_field;
  490: list<list<list<BinaryString>>> adaptedBinStringList3_field;
  491: list<NestedBinTypeDef> adaptedBinNestedTypeDefList_field;
  492: list<adaptedIntSet> adaptedSetList_field;
  493: list<list<adaptedIntSet>> adaptedSetList2_field;
  494: list<list<list<adaptedIntSet>>> adaptedSetList3_field;
  498: map<i32, adaptedIntSet> adaptedMap7_field;
  499: map<i32, map<i32, adaptedIntList>> adaptedMap8_field;
  500: map<i32, map<i32, map<i32, adaptedIntMap>>> adaptedMap9_field;

  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<List<String>>',
  }
  501: list<adaptedInt> doubleAdaptedList_field;
  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<List<String>>',
  }
  502: list<adaptedIntSet> doubleAdaptedList2_field;
  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<List<io.netty.buffer.ByteBuf>>',
  }
  503: list<SlicedByteBuf> doubleAdaptedList3_field;
  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<Map<String, io.netty.buffer.ByteBuf>>',
  }
  504: map<adaptedInt, SlicedByteBuf> doubleAdaptedMap1_field;

  @java.Adapter{
    adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
    typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<com.facebook.thrift.adapter.test.Wrapped<List<String>>>',
  }
  505: doubleAdaptedIntList doubleAdaptedIntList2_field;
  506: list<Any> anyList_field;
  507: map<i32, list<Any>> anyMap_field;
  508: list<myUnion> unionList_field;
  509: list<myException> exceptionList_field;
  @thrift.TerseWrite
  510: list<adaptedLong> terseAdaptedLongList_field;
  511: optional list<adaptedLong> optionalAdaptedLongList_field;
  @thrift.TerseWrite
  512: list<adaptedIntSet> terseAdaptedIntSetList_field;
  513: optional list<adaptedIntSet> optionalAdaptedIntSetList_field;
  514: quadrupleAdaptedIntList quadAdapted_field;
  515: map<
    adaptedInt,
    map<adaptedShort, quadrupleAdaptedIntList>
  > quadAdaptedMap_field;
  516: adaptedInt adaptedInt;
}

// The following were automatically generated and may benefit from renaming.
typedef string (java.swift.binary_string = "1") string_5578
