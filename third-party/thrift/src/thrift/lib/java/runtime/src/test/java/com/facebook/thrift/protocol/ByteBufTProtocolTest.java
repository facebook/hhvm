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

import static org.junit.Assert.assertEquals;

import com.facebook.thrift.test.EveryLayout;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import java.util.Arrays;
import java.util.Collection;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class ByteBufTProtocolTest {

  @Parameterized.Parameters
  public static Collection<Object> data() {
    return Arrays.asList(
        SerializationProtocol.TCompact,
        SerializationProtocol.TBinary,
        SerializationProtocol.TSimpleJSONBase64,
        SerializationProtocol.TSimpleJSON,
        SerializationProtocol.TJSON);
  }

  private final SerializationProtocol serializationProtocol;

  public ByteBufTProtocolTest(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
  }

  @Test
  public void testProtocol() {
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
}
