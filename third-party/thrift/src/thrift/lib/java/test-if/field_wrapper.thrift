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
include "thrift/lib/thrift/any.thrift"

package "test.dev/thrift/lib/java/test/wrapper"

namespace java.swift com.facebook.thrift.test.wrapper

@java.Adapter{
  adapterClassName = "com.facebook.thrift.any.AnyAdapter",
  typeClassName = "com.facebook.thrift.any.Any",
}
typedef any.Any Any

struct TestStruct {
  1001: i32 context;
  3: bool wrappedBoolean_field;
  4: i32 wrappedInt_field;
  10: binary wrappedByteBuf_field;
  11: binary wrappedByteBuf_field2;
  12: i32 wrappedDoubleAdaptedInt_field;
  13: i32 wrappedSingleAdaptedInt_field;
  14: list<i32> listAdaptedInt_field;
  15: map<i32, map<i16, list<i16>>> nestedAdapted_field;
  16: list<Any> anyList_field;
  17: list<i32> wrappedDoubleAdaptedIntList_field;
}

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.RetainedSlicedPooledByteBufTypeAdapter",
  typeClassName = "io.netty.buffer.ByteBuf",
}
typedef binary SlicedByteBuf

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.IntToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef i32 adaptedInt

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.ShortToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef i16 adaptedShort

@java.Adapter{
  adapterClassName = 'com.facebook.thrift.adapter.test.GenericTypeAdapter',
  typeClassName = 'com.facebook.thrift.adapter.test.Wrapped<List<String>>',
}
typedef list<adaptedInt> doubleAdaptedIntList

struct WrappedTestStruct {
  1001: i32 context;
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Boolean>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Boolean>",
  }
  3: bool wrappedBoolean_field;
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Integer>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Integer>",
  }
  4: i32 wrappedInt_field;
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<io.netty.buffer.ByteBuf>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<io.netty.buffer.ByteBuf>",
  }
  10: SlicedByteBuf wrappedByteBuf_field;

  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.common.RetainedSlicedPooledByteBufTypeAdapter",
    typeClassName = "io.netty.buffer.ByteBuf",
  }
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<io.netty.buffer.ByteBuf>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<io.netty.buffer.ByteBuf>",
  }
  11: binary wrappedByteBuf_field2;

  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
    typeClassName = "java.lang.Long",
  }
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Long>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Long>",
  }
  12: adaptedInt wrappedDoubleAdaptedInt_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<String>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<String>",
  }
  13: adaptedInt wrappedSingleAdaptedInt_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<List<String>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<List<String>>",
  }
  14: list<adaptedInt> listAdaptedInt_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Map<String, Map<String, List<String>>>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Map<String, Map<String, List<String>>>>",
  }
  15: map<
    adaptedInt,
    map<adaptedShort, list<adaptedShort>>
  > nestedAdapted_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<List<com.facebook.thrift.any.Any>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<List<com.facebook.thrift.any.Any>>",
  }
  16: list<Any> anyList_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<com.facebook.thrift.adapter.test.Wrapped<List<String>>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<com.facebook.thrift.adapter.test.Wrapped<List<String>>>",
  }
  17: doubleAdaptedIntList wrappedDoubleAdaptedIntList_field;
}

@thrift.Experimental
@thrift.TerseWrite
struct TerseWrappedTestStruct {
  1001: i32 context;
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Boolean>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Boolean>",
  }
  3: bool wrappedBoolean_field;
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Integer>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Integer>",
  }
  4: i32 wrappedInt_field;
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<io.netty.buffer.ByteBuf>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<io.netty.buffer.ByteBuf>",
  }
  10: SlicedByteBuf wrappedByteBuf_field;

  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.common.RetainedSlicedPooledByteBufTypeAdapter",
    typeClassName = "io.netty.buffer.ByteBuf",
  }
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<io.netty.buffer.ByteBuf>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<io.netty.buffer.ByteBuf>",
  }
  11: binary wrappedByteBuf_field2;

  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
    typeClassName = "java.lang.Long",
  }
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Long>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Long>",
  }
  12: adaptedInt wrappedDoubleAdaptedInt_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<String>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<String>",
  }
  13: adaptedInt wrappedSingleAdaptedInt_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<List<String>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<List<String>>",
  }
  14: list<adaptedInt> listAdaptedInt_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Map<String, Map<Short, List<String>>>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Map<String, Map<Short, List<String>>>>",
  }
  15: map<adaptedInt, map<i16, list<adaptedShort>>> nestedAdapted_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<List<com.facebook.thrift.any.Any>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<List<com.facebook.thrift.any.Any>>",
  }
  16: list<Any> anyList_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<com.facebook.thrift.adapter.test.Wrapped<List<String>>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<com.facebook.thrift.adapter.test.Wrapped<List<String>>>",
  }
  17: doubleAdaptedIntList wrappedDoubleAdaptedIntList_field;
}

@java.Mutable
@thrift.Experimental
@thrift.TerseWrite
struct MutableTerseWrappedTestStruct {
  1001: i32 context;
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Boolean>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Boolean>",
  }
  3: bool wrappedBoolean_field;
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Integer>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Integer>",
  }
  4: i32 wrappedInt_field;
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<io.netty.buffer.ByteBuf>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<io.netty.buffer.ByteBuf>",
  }
  10: SlicedByteBuf wrappedByteBuf_field;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.common.RetainedSlicedPooledByteBufTypeAdapter",
    typeClassName = "io.netty.buffer.ByteBuf",
  }
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<io.netty.buffer.ByteBuf>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<io.netty.buffer.ByteBuf>",
  }
  11: binary wrappedByteBuf_field2;

  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
    typeClassName = "java.lang.Long",
  }
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Long>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Long>",
  }
  12: adaptedInt wrappedDoubleAdaptedInt_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<String>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<String>",
  }
  13: adaptedInt wrappedSingleAdaptedInt_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<List<String>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<List<String>>",
  }
  14: list<adaptedInt> listAdaptedInt_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Map<String, Map<String, List<String>>>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Map<String, Map<String, List<String>>>>",
  }
  15: map<
    adaptedInt,
    map<adaptedShort, list<adaptedShort>>
  > nestedAdapted_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<List<com.facebook.thrift.any.Any>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<List<com.facebook.thrift.any.Any>>",
  }
  16: list<Any> anyList_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<com.facebook.thrift.adapter.test.Wrapped<List<String>>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<com.facebook.thrift.adapter.test.Wrapped<List<String>>>",
  }
  17: doubleAdaptedIntList wrappedDoubleAdaptedIntList_field;
}

safe permanent client exception WrappedTestException {
  1001: i32 context;
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Boolean>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Boolean>",
  }
  3: optional bool wrappedBoolean_field;
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Integer>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Integer>",
  }
  4: i32 wrappedInt_field;
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<io.netty.buffer.ByteBuf>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<io.netty.buffer.ByteBuf>",
  }
  10: SlicedByteBuf wrappedByteBuf_field;

  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.common.RetainedSlicedPooledByteBufTypeAdapter",
    typeClassName = "io.netty.buffer.ByteBuf",
  }
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<io.netty.buffer.ByteBuf>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<io.netty.buffer.ByteBuf>",
  }
  11: binary wrappedByteBuf_field2;

  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
    typeClassName = "java.lang.Long",
  }
  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Long>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Long>",
  }
  12: adaptedInt wrappedDoubleAdaptedInt_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<String>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<String>",
  }
  13: optional adaptedInt wrappedSingleAdaptedInt_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<List<String>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<List<String>>",
  }
  14: list<adaptedInt> listAdaptedInt_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<Map<String, Map<String, List<String>>>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<Map<String, Map<String, List<String>>>>",
  }
  15: map<
    adaptedInt,
    map<adaptedShort, list<adaptedShort>>
  > nestedAdapted_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<List<com.facebook.thrift.any.Any>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<List<com.facebook.thrift.any.Any>>",
  }
  16: list<Any> anyList_field;

  @java.Wrapper{
    wrapperClassName = "com.facebook.thrift.wrapper.test.FieldWrapper<com.facebook.thrift.adapter.test.Wrapped<List<String>>>",
    typeClassName = "com.facebook.thrift.wrapper.test.PoliciedField<com.facebook.thrift.adapter.test.Wrapped<List<String>>>",
  }
  17: doubleAdaptedIntList wrappedDoubleAdaptedIntList_field;
}
