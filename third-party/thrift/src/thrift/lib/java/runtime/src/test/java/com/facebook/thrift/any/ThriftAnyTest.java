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

import static org.junit.jupiter.api.Assertions.*;

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
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

public class ThriftAnyTest {

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
    Any<Date> any = createAny(new Date());
    IllegalArgumentException ex =
        Assertions.assertThrows(
            IllegalArgumentException.class,
            () -> {
              getType(any);
            });
    Assertions.assertTrue(ex.getMessage().contains("Unsupported type"));
  }

  @Test
  public void testDeserializeInvalidUN() {
    AnyStruct any =
        new AnyStruct.Builder()
            .setType(
                new TypeStruct.Builder()
                    .setName(TypeName.fromStructType(TypeUri.fromUri("invalid")))
                    .build())
            .setProtocol(ProtocolUnion.fromStandard(StandardProtocol.BINARY))
            .build();

    Any<TestEnum> received = Any.wrap(any);
    InvalidUniversalNameURIException ex =
        Assertions.assertThrows(
            InvalidUniversalNameURIException.class,
            () -> {
              received.get();
            });
    Assertions.assertTrue(ex.getMessage().contains("Invalid universal name"));
  }

  @Test
  public void testDeserializeUnregisteredUri() {
    AnyStruct any =
        new AnyStruct.Builder()
            .setType(
                new TypeStruct.Builder()
                    .setName(TypeName.fromStructType(TypeUri.fromUri("foo.com/a/b/notexist")))
                    .build())
            .setProtocol(ProtocolUnion.fromStandard(StandardProtocol.BINARY))
            .build();

    Any<TestEnum> received = Any.wrap(any);
    ObjectNotRegisteredException ex =
        Assertions.assertThrows(
            ObjectNotRegisteredException.class,
            () -> {
              received.get();
            });
    Assertions.assertTrue(ex.getMessage().contains("Unable to find type"));
  }

  @Test
  public void testDeserializeUnregisteredHash() {
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
    ObjectNotRegisteredException ex =
        Assertions.assertThrows(
            ObjectNotRegisteredException.class,
            () -> {
              received.get();
            });
    Assertions.assertTrue(ex.getMessage().contains("Unable to find type"));
  }

  @Test
  public void testSerializeUnsupportedElementType() {
    List<Date> list = new ArrayList<>();
    list.add(new Date());

    Any<List<Date>> any = createAny(list, Date.class);
    IllegalArgumentException ex =
        Assertions.assertThrows(
            IllegalArgumentException.class,
            () -> {
              getType(any);
            });
    Assertions.assertTrue(ex.getMessage().contains("Unsupported type"));
  }

  private static class UnregisteredTestStruct implements ThriftSerializable {
    @Override
    public void write0(TProtocol protocol) throws TException {}
  }

  @Test
  public void testUnregisteredObject() {
    Any<UnregisteredTestStruct> any =
        new Any.Builder<>(new UnregisteredTestStruct())
            .setProtocol(StandardProtocol.COMPACT)
            .build();
    ObjectNotRegisteredException ex =
        Assertions.assertThrows(
            ObjectNotRegisteredException.class,
            () -> {
              getType(any);
            });
    Assertions.assertTrue(
        ex.getMessage().contains("Unable to find type for any to serialize object"));
  }

  @Test
  public void testNullValue() {
    NullPointerException ex =
        Assertions.assertThrows(
            NullPointerException.class,
            () -> {
              createAny(null);
            });
    Assertions.assertTrue(ex.getMessage().contains("Any must have a value"));
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
    IllegalStateException ex =
        Assertions.assertThrows(
            IllegalStateException.class,
            () -> {
              new Any.Builder<>("foo").useUri().useHashPrefix().build();
            });
    Assertions.assertTrue(ex.getMessage().contains("Can not set both useHashPrefix and useUri"));
  }

  @Test
  public void testSetInvalidHashPrefix() {
    IllegalArgumentException ex =
        Assertions.assertThrows(
            IllegalArgumentException.class,
            () -> {
              new Any.Builder<>("foo").useHashPrefix(0).build();
            });
    Assertions.assertTrue(ex.getMessage().contains("Number of hash prefix must be at least 1"));
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
    Any<String> any = new Any.Builder<>("foo").setCustomProtocol(0L).build();
    IllegalArgumentException ex =
        Assertions.assertThrows(
            IllegalArgumentException.class,
            () -> {
              any.getAny();
            });
    Assertions.assertTrue(ex.getMessage().contains("Custom protocol serializer is not registered"));
  }

  @Test
  public void testListWithoutType() {
    IllegalArgumentException ex =
        Assertions.assertThrows(
            IllegalArgumentException.class,
            () -> {
              createAny(new ArrayList<>());
            });
    Assertions.assertTrue(ex.getMessage().contains("List element type class is not provided"));
  }

  @Test
  public void testSetWithoutType() {
    IllegalArgumentException ex =
        Assertions.assertThrows(
            IllegalArgumentException.class,
            () -> {
              createAny(new HashSet<>());
            });
    Assertions.assertTrue(ex.getMessage().contains("Set element type class is not provided"));
  }

  @Test
  public void testMapWithoutType() {
    IllegalArgumentException ex =
        Assertions.assertThrows(
            IllegalArgumentException.class,
            () -> {
              createAny(new HashMap<>());
            });
    Assertions.assertTrue(
        ex.getMessage().contains("Map key and value element type classes are not provided"));
  }

  @Test
  public void testMissingTypeInfo() {
    Any<HashMap<String, Integer>> any = createAny(new HashMap<>(), String.class);
    IllegalArgumentException ex =
        Assertions.assertThrows(
            IllegalArgumentException.class,
            () -> {
              any.getAny();
            });
    Assertions.assertTrue(ex.getMessage().contains("Type class is not provided for nested value"));
  }

  @Test
  public void testInvalidTypeInfo() {
    Any<HashMap<String, Integer>> any = createAny(new HashMap<>(), String.class, Date.class);
    IllegalArgumentException ex =
        Assertions.assertThrows(
            IllegalArgumentException.class,
            () -> {
              any.getAny();
            });
    Assertions.assertTrue(ex.getMessage().contains("Unsupported type"));
  }
}
