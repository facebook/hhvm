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

import static org.junit.Assert.*;

import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.test.universalname.TestRequest;
import com.facebook.thrift.type.Type;
import com.facebook.thrift.type.TypeRegistry;
import com.facebook.thrift.type.UniversalName;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import org.apache.thrift.conformance.Any;
import org.apache.thrift.conformance.StandardProtocol;
import org.apache.thrift.protocol.TProtocol;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

public class AnyTest {

  @Rule public ExpectedException expectedException = ExpectedException.none();

  private ByteBufTProtocol createTProtocol(SerializationProtocol serializationProtocol) {
    ByteBuf dest = RpcResources.getUnpooledByteBufAllocator().buffer();
    return SerializerUtil.toByteBufProtocol(serializationProtocol, dest);
  }

  private ByteBufTProtocol cloneTProtocol(
      ByteBufTProtocol source, SerializationProtocol serializationProtocol) {
    return SerializerUtil.toByteBufProtocol(serializationProtocol, source.getByteBuf().copy());
  }

  private TestRequest createSampleRequest() {
    return new TestRequest.Builder().setABool(true).setAString("test").setALong(1050).build();
  }

  private ByteBufTProtocol serialize(LazyAny lazyAny, SerializationProtocol protocol) {
    LazyAnyAdapter adapter = new LazyAnyAdapter();
    ByteBufTProtocol prot = createTProtocol(protocol);
    adapter.toThrift(lazyAny, prot);

    return prot;
  }

  private ByteBufTProtocol serialize(LazyAny lazyAny) {
    return serialize(lazyAny, SerializationProtocol.TBinary);
  }

  private Object deserialize(TProtocol protocol) {
    LazyAnyAdapter adapter = new LazyAnyAdapter();
    return adapter.fromThrift(protocol).get();
  }

  @Test
  public void testSimpleAnyRequest() {
    TestRequest req = createSampleRequest();
    LazyAny<TestRequest> lazyAny = new LazyAny.Builder<>(req).build();
    assertEquals(req, lazyAny.get());

    LazyAnyAdapter adapter = new LazyAnyAdapter();
    ByteBufTProtocol protocol = createTProtocol(SerializationProtocol.TBinary);
    adapter.toThrift(lazyAny, protocol);

    ByteBufTProtocol readerProtocol = cloneTProtocol(protocol, SerializationProtocol.TBinary);
    TestRequest received = (TestRequest) adapter.fromThrift(readerProtocol).get();

    assertEquals(req.getAString(), received.getAString());
    assertEquals(req.getALong(), received.getALong());
    assertEquals(req.isABool(), received.isABool());
  }

  @Test
  public void testUseUri() {
    TestRequest req = createSampleRequest();
    LazyAny lazyAny =
        new LazyAny.Builder<>()
            .setValue(req)
            .setProtocol(SerializationProtocol.TCompact)
            .useUri()
            .build();
    assertEquals(req, lazyAny.get());

    LazyAnyAdapter adapter = new LazyAnyAdapter();
    ByteBufTProtocol protocol = createTProtocol(SerializationProtocol.TBinary);
    adapter.toThrift(lazyAny, protocol);

    Any any = Any.read0(protocol);
    assertEquals("test.dev/thrift/lib/java/my_request", any.getType());
    assertEquals(null, any.getTypeHashPrefixSha2256());
    assertEquals(null, any.getCustomProtocol());
    assertEquals(StandardProtocol.COMPACT, any.getProtocol());

    protocol.getByteBuf().resetReaderIndex();
    TestRequest received = (TestRequest) adapter.fromThrift(protocol).get();

    assertEquals(req.getAString(), received.getAString());
    assertEquals(req.getALong(), received.getALong());
    assertEquals(req.isABool(), received.isABool());
  }

  @Test
  public void testUseHashPrefix() {
    TestRequest req = createSampleRequest();
    LazyAny lazyAny =
        new LazyAny.Builder<>()
            .setValue(req)
            .setProtocol(SerializationProtocol.TCompact)
            .useHashPrefix(10)
            .build();
    assertEquals(req, lazyAny.get());

    ByteBufTProtocol protocol = serialize(lazyAny, SerializationProtocol.TBinary);

    Any any = Any.read0(protocol);
    assertEquals(null, any.getType());
    assertEquals("ddbc4da47505da3a6931", ByteBufUtil.hexDump(any.getTypeHashPrefixSha2256()));
    assertEquals(null, any.getCustomProtocol());
    assertEquals(StandardProtocol.COMPACT, any.getProtocol());

    protocol.getByteBuf().resetReaderIndex();
    TestRequest received = (TestRequest) deserialize(protocol);

    assertEquals(req.getAString(), received.getAString());
    assertEquals(req.getALong(), received.getALong());
    assertEquals(req.isABool(), received.isABool());
  }

  @Test
  public void testDefaultValues() {
    TestRequest req = createSampleRequest();
    LazyAny lazyAny = new LazyAny.Builder<>().setValue(req).build();
    assertEquals(req, lazyAny.get());

    ByteBufTProtocol protocol = serialize(lazyAny, SerializationProtocol.TBinary);

    Any any = Any.read0(protocol);
    assertEquals(null, any.getType());
    assertEquals("ddbc4da47505da3a", ByteBufUtil.hexDump(any.getTypeHashPrefixSha2256()));
    assertEquals(null, any.getCustomProtocol());
    assertEquals(StandardProtocol.COMPACT, any.getProtocol());

    protocol.getByteBuf().resetReaderIndex();
    TestRequest received = (TestRequest) deserialize(protocol);

    assertEquals(req.getAString(), received.getAString());
    assertEquals(req.getALong(), received.getALong());
    assertEquals(req.isABool(), received.isABool());
  }

  @Test
  public void testUseMinHashPrefix() {
    LazyAny lazyAny =
        new LazyAny.Builder<>()
            .setValue(createSampleRequest())
            .setProtocol(SerializationProtocol.TCompact)
            .useHashPrefix(2)
            .build();

    ByteBufTProtocol protocol = serialize(lazyAny, SerializationProtocol.TBinary);

    Any any = Any.read0(protocol);
    assertEquals(null, any.getType());
    assertEquals("ddbc4da47505da3a", ByteBufUtil.hexDump(any.getTypeHashPrefixSha2256()));
  }

  private static ByteBuf customSerializer(Object o) {
    byte[] bytes = new byte[7];
    TestRequest r = (TestRequest) o;
    bytes[0] = r.isABool() ? (byte) 1 : (byte) 0;
    bytes[1] = (byte) ((r.getALong() % 255));
    bytes[2] = (byte) ((r.getALong() / 255));
    System.arraycopy(r.getAString().getBytes(), 0, bytes, 3, 4);

    return Unpooled.wrappedBuffer(bytes);
  }

  private static Object customDeserializer(Class clazz, ByteBuf byteBuf) {
    byte[] bytes = ByteBufUtil.getBytes(byteBuf);
    boolean bool = bytes[0] == 0 ? false : true;
    long longVal = bytes[1] + ((int) bytes[2]) * 255;
    String st = new String(bytes, 3, 4);
    return new TestRequest(bool, longVal, st);
  }

  private static ByteBuf customLongSerializer(Object o) {
    return Unpooled.wrappedBuffer(String.valueOf(o).getBytes());
  }

  @Test
  public void testCustomProtocol() {
    LazyAny.registerSerializer("custom-protocol", AnyTest::customSerializer);

    LazyAny lazyAny =
        new LazyAny.Builder<>()
            .setValue(createSampleRequest())
            .setCustomProtocol("custom-protocol")
            .build();

    ByteBufTProtocol protocol = serialize(lazyAny);

    Any any = Any.read0(protocol);
    assertNull(any.getType());
    assertNotNull(any.getTypeHashPrefixSha2256());
    assertEquals("custom-protocol", any.getCustomProtocol());
    assertEquals(true, new String(any.getData().array(), 3, 4).contains("test"));
  }

  @Test
  public void testCustomProtocolDeserialization() {
    LazyAny.registerSerializer("custom-protocol", AnyTest::customSerializer);
    LazyAny.registerDeserializer("custom-protocol", AnyTest::customDeserializer);

    LazyAny lazyAny =
        new LazyAny.Builder<>()
            .setValue(createSampleRequest())
            .setCustomProtocol("custom-protocol")
            .build();

    ByteBufTProtocol protocol = serialize(lazyAny);

    TestRequest received = (TestRequest) deserialize(protocol);

    assertEquals(1050, received.getALong());
    assertEquals("test", received.getAString());
  }

  @Test
  public void testNoCustomProtocol() {
    expectedException.expect(IllegalStateException.class);
    expectedException.expectMessage("Custom protocol deserializer not registered");
    LazyAny.registerSerializer("cp1", AnyTest::customSerializer);
    LazyAny lazyAny =
        new LazyAny.Builder<>().setValue(createSampleRequest()).setCustomProtocol("cp1").build();

    ByteBufTProtocol protocol = serialize(lazyAny);
    deserialize(protocol);
  }

  @Test
  public void testMissingValue() {
    expectedException.expect(NullPointerException.class);
    expectedException.expectMessage("value");
    new LazyAny.Builder<>().build();
  }

  @Test
  public void testMissingCustomProtocol() {
    expectedException.expect(IllegalArgumentException.class);
    expectedException.expectMessage("serializer");
    new LazyAny.Builder<>(createSampleRequest()).setCustomProtocol("test-protocol").build();
  }

  @Test
  public void testMissingType() {
    expectedException.expect(IllegalArgumentException.class);
    expectedException.expectMessage("String");
    new LazyAny.Builder<>(new String("test")).build();
  }

  @Test
  public void testTypeAndUri() {
    expectedException.expect(IllegalStateException.class);
    expectedException.expectMessage("not both");
    new LazyAny.Builder<>().setValue(createSampleRequest()).useUri().useHashPrefix().build();
  }

  @Test
  public void testPreferUri() {
    LazyAny lazyAny = new LazyAny.Builder<>().setValue(createSampleRequest()).build();

    ByteBufTProtocol protocol = serialize(lazyAny);

    Any any = Any.read0(protocol);
    assertNull(any.getType());
    assertNotNull(any.getTypeHashPrefixSha2256());
  }

  @Test
  public void testMissingCustomSerializer() {
    expectedException.expect(IllegalArgumentException.class);
    expectedException.expectMessage("You must register a serializer");
    TypeRegistry.add(new Type(new UniversalName("foo.com/a/b"), Integer.class, null));
    new LazyAny.Builder<>().setValue(Integer.MAX_VALUE).build();
  }

  @Test
  public void testPreferType() {
    LazyAny.registerSerializer("c1", AnyTest::customLongSerializer);
    TypeRegistry.add(new Type(new UniversalName("f.c/a/b"), Long.class, null));
    LazyAny lazyAny =
        new LazyAny.Builder<>().setValue(Long.MAX_VALUE).setCustomProtocol("c1").build();

    ByteBufTProtocol protocol = serialize(lazyAny);

    Any any = Any.read0(protocol);
    assertNotNull(any.getType());
    assertNull(any.getTypeHashPrefixSha2256());
  }

  @Test
  public void testCachedValue() {
    TestRequest req = createSampleRequest();
    LazyAny lazyAny = new LazyAny.Builder<>().setValue(req).build();

    LazyAnyAdapter adapter = new LazyAnyAdapter();
    ByteBuf dest = RpcResources.getUnpooledByteBufAllocator().buffer();
    ByteBufTProtocol protocol =
        SerializerUtil.toByteBufProtocol(SerializationProtocol.TBinary, dest);
    adapter.toThrift(lazyAny, protocol);

    // deserialize it
    SerializedLazyAny any = (SerializedLazyAny) adapter.fromThrift(protocol);
    TestRequest received = (TestRequest) any.get();
    assertEquals(null, any.getAny().getCustomProtocol());

    assertEquals(req, received);
    // Check if the references are the same with the cached one
    assertTrue(any.get() == received);
    dest.clear();
    assertTrue(any.get() == received);
  }

  @Test
  public void testEquals() {
    TestRequest req = createSampleRequest();
    LazyAny lazyAny1 = new LazyAny.Builder<>().setValue(req).build();
    LazyAny lazyAny2 = new LazyAny.Builder<>().setValue(req).build();
    assertEquals(lazyAny1, lazyAny1);
    assertEquals(lazyAny1, lazyAny2);
    assertNotEquals(lazyAny1, req);
    assertEquals(lazyAny1.hashCode(), lazyAny2.hashCode());
  }

  @Test
  public void testZeroHashBytes() {
    expectedException.expect(IllegalArgumentException.class);
    expectedException.expectMessage("select one or more bytes");
    new LazyAny.Builder<>(createSampleRequest()).useHashPrefix(0).build();
  }

  @Test
  public void testNullSerializationProtocol() {
    LazyAny lazyAny =
        new LazyAny.Builder<>().setValue(createSampleRequest()).setProtocol(null).build();

    LazyAnyAdapter adapter = new LazyAnyAdapter();
    ByteBufTProtocol protocol = createTProtocol(SerializationProtocol.TBinary);
    adapter.toThrift(lazyAny, protocol);

    Any any = Any.read0(protocol);
    assertEquals(StandardProtocol.COMPACT, any.getProtocol());
  }
}
