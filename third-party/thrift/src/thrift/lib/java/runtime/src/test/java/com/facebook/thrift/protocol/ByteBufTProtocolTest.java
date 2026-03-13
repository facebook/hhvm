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

package com.facebook.thrift.protocol;

import static org.junit.jupiter.api.Assertions.assertEquals;

import com.facebook.thrift.test.EveryLayout;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import java.util.stream.Stream;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.Arguments;
import org.junit.jupiter.params.provider.MethodSource;

public class ByteBufTProtocolTest {

  static Stream<Arguments> data() {
    return Stream.of(
        Arguments.of(SerializationProtocol.TCompact),
        Arguments.of(SerializationProtocol.TBinary),
        Arguments.of(SerializationProtocol.TSimpleJSONBase64),
        Arguments.of(SerializationProtocol.TSimpleJSON),
        Arguments.of(SerializationProtocol.TJSON));
  }

  private SerializationProtocol serializationProtocol;

  @ParameterizedTest
  @MethodSource("data")
  public void testProtocol(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    EveryLayout struct =
        new EveryLayout.Builder().setAInt(5).setALong(1000).setAString("test").build();

    ByteBuf dest = RpcResources.getUnpooledByteBufAllocator().buffer();
    ByteBufTProtocol protocol = SerializerUtil.toByteBufProtocol(serializationProtocol, dest);

    struct.write0(protocol);
    EveryLayout received = EveryLayout.read0(protocol);

    assertEquals(struct.getAInt(), received.getAInt());
    assertEquals(struct.getALong(), received.getALong());
    assertEquals(struct.getAString(), received.getAString());
  }

  @ParameterizedTest
  @MethodSource("data")
  public void testUnicodeCharterEncodingAndEscape(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    EveryLayout struct =
        new EveryLayout.Builder().setAInt(5).setALong(1000).setAString("\n\u0001😀").build();

    ByteBuf dest = RpcResources.getUnpooledByteBufAllocator().buffer();
    ByteBufTProtocol protocol = SerializerUtil.toByteBufProtocol(serializationProtocol, dest);

    struct.write0(protocol);
    EveryLayout received = EveryLayout.read0(protocol);

    assertEquals(struct.getAInt(), received.getAInt());
    assertEquals(struct.getALong(), received.getALong());
    assertEquals(struct.getAString(), received.getAString());
  }
}
