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

import com.facebook.thrift.payload.ThriftSerializable;
import com.facebook.thrift.standard_type.StandardProtocol;
import com.facebook.thrift.standard_type.TypeName;
import com.facebook.thrift.standard_type.TypeUri;
import com.facebook.thrift.test.thrift.any.TestEnum;
import com.facebook.thrift.test.thrift.any.TestShortUriStruct;
import com.facebook.thrift.test.thrift.any.TestStruct;
import com.facebook.thrift.type.InvalidUniversalNameURIException;
import com.facebook.thrift.type.TypeRegistry;
import com.facebook.thrift.type_swift.AnyStruct;
import com.facebook.thrift.type_swift.ProtocolUnion;
import com.facebook.thrift.type_swift.TypeStruct;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TProtocol;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

public class ThriftAnyTest {

  @Rule public ExpectedException expectedException = ExpectedException.none();

  private <T> Any<T> createAny(T o) {
    return new Any.Builder<>(o).setProtocol(StandardProtocol.BINARY).build();
  }

  private <T> Any<T> createAny(T o, Class... clazz) {
    return new Any.Builder<>(o, clazz).setProtocol(StandardProtocol.BINARY).build();
  }

  private <T> TypeName getType(Any<T> any) {
    return Objects.requireNonNull(any.getAny().getType()).getName();
  }

  private <T> void assertParamsEmpty(Any<T> any) {
    if (any instanceof SerializedAny<?>) {
      assertNull(Objects.requireNonNull(any.getAny().getType()).getParams());
    } else {
      assertNull(Objects.requireNonNull(any.getAny().getType()).getParams());
    }
  }

  @Test
  public void testSerializeUnsupportedType() {
    expectedException.expect(IllegalArgumentException.class);
    expectedException.expectMessage("Unsupported type");
    Any<Date> any = createAny(new Date());
    getType(any);
  }

  @Test
  public void testDeserializeInvalidUN() {
    expectedException.expect(InvalidUniversalNameURIException.class);
    expectedException.expectMessage("Invalid universal name");
    AnyStruct any =
        new AnyStruct.Builder()
            .setType(
                new TypeStruct.Builder()
                    .setName(TypeName.fromStructType(TypeUri.fromUri("invalid")))
                    .build())
            .setProtocol(ProtocolUnion.fromStandard(StandardProtocol.BINARY))
            .build();

    Any<TestEnum> received = Any.wrap(any);
    received.get();
  }

  @Test
  public void testDeserializeUnregisteredUri() {
    expectedException.expect(ObjectNotRegisteredException.class);
    expectedException.expectMessage("Unable to find type");
    AnyStruct any =
        new AnyStruct.Builder()
            .setType(
                new TypeStruct.Builder()
                    .setName(TypeName.fromStructType(TypeUri.fromUri("foo.com/a/b/notexist")))
                    .build())
            .setProtocol(ProtocolUnion.fromStandard(StandardProtocol.BINARY))
            .build();

    Any<TestEnum> received = Any.wrap(any);
    received.get();
  }

  @Test
  public void testDeserializeUnregisteredHash() {
    expectedException.expect(ObjectNotRegisteredException.class);
    expectedException.expectMessage("Unable to find type");
    AnyStruct any =
        new AnyStruct.Builder()
            .setType(
                new TypeStruct.Builder()
                    .setName(
                        TypeName.fromStructType(
                            TypeUri.fromTypeHashPrefixSha2256(
                                Unpooled.wrappedBuffer("invalid".getBytes()))))
                    .build())
            .setProtocol(ProtocolUnion.fromStandard(StandardProtocol.BINARY))
            .build();

    Any<TestEnum> received = Any.wrap(any);
    received.get();
  }

  @Test
  public void testSerializeUnsupportedElementType() {
    expectedException.expect(IllegalArgumentException.class);
    expectedException.expectMessage("Unsupported type");
    List<Date> list = new ArrayList<>();
    list.add(new Date());

    Any<List<Date>> any = createAny(list, Date.class);
    getType(any);
  }

  private static class UnregisteredTestStruct implements ThriftSerializable {
    @Override
    public void write0(TProtocol protocol) throws TException {}
  }

  @Test
  public void testUnregisteredObject() {
    expectedException.expect(ObjectNotRegisteredException.class);
    expectedException.expectMessage("Unable to find type for any to serialize object");

    Any<UnregisteredTestStruct> any =
        new Any.Builder<>(new UnregisteredTestStruct())
            .setProtocol(StandardProtocol.COMPACT)
            .build();
    getType(any);
  }

  @Test
  public void testNullValue() {
    expectedException.expect(NullPointerException.class);
    expectedException.expectMessage("Any must have a value");
    createAny(null);
  }

  @Test
  public void testUri() {
    TestStruct st = new TestStruct.Builder().setBoolField(true).setInfField(9).build();
    String uri = "test.dev/thrift/lib/java/test/thrift/any/TestStruct";

    Any<TestStruct> any =
        new Any.Builder<>(st).setProtocol(StandardProtocol.COMPACT).useUri().build();
    assertEquals(uri, getType(any).getStructType().getUri());

    Any<TestStruct> received = Any.wrap(any.getAny());
    assertEquals(st, received.get());
    assertTrue(getType(received).isSetStructType());
    assertParamsEmpty(received);
    assertEquals(uri, getType(received).getStructType().getUri());
  }

  @Test
  public void testHashPrefix() {
    TestStruct st = new TestStruct.Builder().setBoolField(true).setInfField(9).build();
    String hash =
        TypeRegistry.findByClass(TestStruct.class).getUniversalName().getHash().substring(0, 16);

    Any<TestStruct> any =
        new Any.Builder<>(st).setProtocol(StandardProtocol.COMPACT).useHashPrefix().build();
    assertEquals(
        hash, ByteBufUtil.hexDump(getType(any).getStructType().getTypeHashPrefixSha2256()));

    Any<TestStruct> received = Any.wrap(any.getAny());
    assertEquals(st, received.get());
    assertTrue(getType(received).isSetStructType());
    assertParamsEmpty(received);
    assertEquals(
        hash, ByteBufUtil.hexDump(getType(received).getStructType().getTypeHashPrefixSha2256()));
  }

  @Test
  public void testSetBothUriAndHash() {
    expectedException.expect(IllegalStateException.class);
    expectedException.expectMessage("Can not set both useHashPrefix and useUri");
    new Any.Builder<>("foo").useUri().useHashPrefix().build();
  }

  @Test
  public void testSetInvalidHashPrefix() {
    expectedException.expect(IllegalArgumentException.class);
    expectedException.expectMessage("Number of hash prefix must be at least 1");
    new Any.Builder<>("foo").useHashPrefix(0).build();
  }

  private void assertInvalidProtocol(AnyBuilder builder) {
    try {
      builder.build();
    } catch (IllegalStateException ex) {
      assertEquals("Can not set multiple protocol", ex.getMessage());
      return;
    }
    fail("Expected IllegalArgumentException");
  }

  @Test
  public void testMultipleProtocol() {
    assertInvalidProtocol(
        new Any.Builder<>("foo").setProtocol(StandardProtocol.BINARY).setCustomProtocol(5L));
    assertInvalidProtocol(
        new Any.Builder<>("foo").setProtocol(StandardProtocol.BINARY).setCustomProtocol("custom"));
    assertInvalidProtocol(
        new Any.Builder<>("foo").setCustomProtocol(4L).setCustomProtocol("custom"));
    assertInvalidProtocol(
        new Any.Builder<>("foo")
            .setProtocol(StandardProtocol.BINARY)
            .setCustomProtocol("custom")
            .setCustomProtocol(1L));
  }

  @Test
  public void testPreferShortUri() {
    TestShortUriStruct st = new TestShortUriStruct.Builder().build();
    Any<TestShortUriStruct> any = createAny(st);
    assertEquals("a.b/c/d", any.getAny().getType().getName().getStructType().getUri());

    TestStruct st2 = new TestStruct.Builder().build();
    Any<TestStruct> any2 = createAny(st2);
    assertEquals(false, any2.getAny().getType().getName().getStructType().isSetUri());
  }

  @Test
  public void testEquals() {
    TestShortUriStruct st = new TestShortUriStruct.Builder().build();
    Any<TestShortUriStruct> any1 = createAny(st);
    Any<TestShortUriStruct> any2 = createAny(st);
    assertEquals(any1.getAny(), any2.getAny());
    assertEquals(any1, any2);
  }

  @Test
  public void testListEquals() {
    Any<List<Integer>> any1 = createAny(Arrays.asList(5, 6, 7), Integer.class);
    Any<List<Integer>> any2 = createAny(Arrays.asList(5, 6, 7), Integer.class);
    assertEquals(any1, any2);
  }

  @Test
  public void testHashCode() {
    Any<List<Integer>> any1 = createAny(Arrays.asList(5, 6, 7), Integer.class);
    Any<List<Integer>> any2 = createAny(Arrays.asList(5, 6, 7), Integer.class);
    assertEquals(any1.hashCode(), any2.hashCode());
  }

  private static ByteBuf mySerializer(Object o) {
    return Unpooled.wrappedBuffer(((String) o).getBytes());
  }

  private static Object myDeserializer(TypeStruct typeStruct, ByteBuf data) {
    byte[] bytes = new byte[data.readableBytes()];
    data.readBytes(bytes);
    return new String(bytes);
  }

  @Test
  public void testCustomSerializerUri() {
    Any.registerSerializer("my-serializer", ThriftAnyTest::mySerializer);
    Any.registerDeserializer("my-serializer", ThriftAnyTest::myDeserializer);

    Any<String> any = new Any.Builder<>("foo").setCustomProtocol("my-serializer").build();
    assertEquals("foo", any.get());
    assertTrue(getType(any).isSetStringType());
    assertParamsEmpty(any);

    Any<String> received = Any.wrap(any.getAny());
    assertEquals("foo", received.get());
    assertTrue(getType(received).isSetStringType());
    assertParamsEmpty(received);
  }

  @Test
  public void testCustomSerializerId() {
    Any.registerSerializer(3, ThriftAnyTest::mySerializer);
    Any.registerDeserializer(3, ThriftAnyTest::myDeserializer);

    Any<String> any = new Any.Builder<>("foo").setCustomProtocol(3L).build();
    assertEquals("foo", any.get());
    assertTrue(getType(any).isSetStringType());
    assertParamsEmpty(any);

    Any<String> received = Any.wrap(any.getAny());
    assertEquals("foo", received.get());
    assertTrue(getType(received).isSetStringType());
    assertParamsEmpty(received);
  }

  @Test
  public void testUnknownCustomSerializer() {
    expectedException.expect(IllegalArgumentException.class);
    expectedException.expectMessage("Custom protocol serializer is not registered");
    Any<String> any = new Any.Builder<>("foo").setCustomProtocol(0L).build();
    any.getAny();
  }

  @Test
  public void testListWithoutType() {
    expectedException.expect(IllegalArgumentException.class);
    expectedException.expectMessage("List element type class is not provided");

    createAny(new ArrayList<>());
  }

  @Test
  public void testSetWithoutType() {
    expectedException.expect(IllegalArgumentException.class);
    expectedException.expectMessage("Set element type class is not provided");

    createAny(new HashSet<>());
  }

  @Test
  public void testMapWithoutType() {
    expectedException.expect(IllegalArgumentException.class);
    expectedException.expectMessage("Map key and value element type classes are not provided");

    createAny(new HashMap<>());
  }

  @Test
  public void testMissingTypeInfo() {
    expectedException.expect(IllegalArgumentException.class);
    expectedException.expectMessage("Type class is not provided for nested value");

    Any<HashMap<String, Integer>> any = createAny(new HashMap<>(), String.class);
    any.getAny();
  }

  @Test
  public void testInvalidTypeInfo() {
    expectedException.expect(IllegalArgumentException.class);
    expectedException.expectMessage("Unsupported type");

    Any<HashMap<String, Integer>> any = createAny(new HashMap<>(), String.class, Date.class);
    any.getAny();
  }
}
