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

package com.facebook.thrift.adapter;

import com.facebook.thrift.test.adapter.TypeAdapterTestStruct;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import java.util.Arrays;
import java.util.Collection;
import java.util.concurrent.ThreadLocalRandom;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class TypeAdapterTest {
  @Parameterized.Parameters
  public static Collection<Object> data() {
    return Arrays.asList(SerializationProtocol.TCompact, SerializationProtocol.TBinary);
  }

  private final SerializationProtocol protocol;

  public TypeAdapterTest(SerializationProtocol protocol) {
    this.protocol = protocol;
  }

  @Test
  public void testSerializeTypeAdapterWithEmptyByteBufs() {
    TypeAdapterTestStruct struct = TypeAdapterTestStruct.defaultInstance();
    byte[] bytes = SerializerUtil.toByteArray(struct, protocol);
    TypeAdapterTestStruct fromBytes =
        SerializerUtil.fromByteArray(TypeAdapterTestStruct.asReader(), bytes, protocol);
    Assert.assertEquals(struct, fromBytes);
  }

  private static ByteBuf generateByteBuf() {
    byte[] bytes = new byte[32];
    ThreadLocalRandom.current().nextBytes(bytes);
    return Unpooled.wrappedBuffer(bytes);
  }

  @Test
  public void testSerializeTypeAdapter() {
    final ByteBuf b1 = generateByteBuf();
    final String s1 = ByteBufUtil.hexDump(b1);

    final ByteBuf b2 = generateByteBuf();
    final String s2 = ByteBufUtil.hexDump(b2);

    final ByteBuf b3 = generateByteBuf();
    final String s3 = ByteBufUtil.hexDump(b3);

    TypeAdapterTestStruct struct =
        new TypeAdapterTestStruct.Builder().setB1(b1).setB2(b2).setB3(b3).build();

    byte[] bytes = SerializerUtil.toByteArray(struct, protocol);

    TypeAdapterTestStruct fromBytes =
        SerializerUtil.fromByteArray(TypeAdapterTestStruct.asReader(), bytes, protocol);

    Assert.assertEquals(s1, ByteBufUtil.hexDump(fromBytes.getB1()));
    Assert.assertEquals(s2, ByteBufUtil.hexDump(fromBytes.getB2()));
    Assert.assertEquals(s3, ByteBufUtil.hexDump(fromBytes.getB3()));
  }
}
