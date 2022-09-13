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

package com.facebook.thrift.any;

import static org.junit.Assert.assertEquals;

import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.test.universalname.TestRequest;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import java.util.Arrays;
import java.util.Collection;
import org.apache.thrift.conformance.Any;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class LazyAnyStandardProtocolTest {

  @Parameterized.Parameters
  public static Collection<Object> data() {
    return Arrays.asList(
        SerializationProtocol.TCompact,
        SerializationProtocol.TBinary,
        SerializationProtocol.TSimpleJSONBase64,
        SerializationProtocol.TSimpleJSON
        /** SerializationProtocol.TJSON * */
        );
  }

  private final SerializationProtocol serializationProtocol;

  public LazyAnyStandardProtocolTest(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
  }

  @Test
  public void testStandardProtocol() {
    TestRequest req =
        new TestRequest.Builder().setABool(true).setAString("test").setALong(1050).build();
    LazyAny any = new LazyAny.Builder<>(req).setProtocol(serializationProtocol).build();
    assertEquals(req, any.get());

    ByteBuf dest = RpcResources.getUnpooledByteBufAllocator().buffer();
    ByteBufTProtocol protocol =
        SerializerUtil.toByteBufProtocol(SerializationProtocol.TBinary, dest);

    any.getAny().write0(protocol);
    TestRequest received = (TestRequest) LazyAny.wrap(Any.read0(protocol)).get();

    assertEquals(req.getAString(), received.getAString());
    assertEquals(req.getALong(), received.getALong());
    assertEquals(req.isABool(), received.isABool());
  }
}
