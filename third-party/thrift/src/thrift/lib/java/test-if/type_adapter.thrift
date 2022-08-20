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

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.SlicedPooledByteBufTypeTypeAdapter",
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

struct TypeAdapterTestStruct {
  1: SlicedByteBuf b1;
  2: CopiedByteBuf b2;
  3: UnpooledByteBuf b3;
}
