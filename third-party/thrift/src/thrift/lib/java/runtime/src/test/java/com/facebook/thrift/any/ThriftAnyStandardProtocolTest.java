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
import com.facebook.thrift.standard_type.StandardProtocol;
import com.facebook.thrift.standard_type.TypeName;
import com.facebook.thrift.test.thrift.any.TestEnum;
import com.facebook.thrift.test.thrift.any.TestException;
import com.facebook.thrift.test.thrift.any.TestOpenEnum;
import com.facebook.thrift.test.thrift.any.TestStruct;
import com.facebook.thrift.test.thrift.any.TestUnion;
import com.facebook.thrift.type_swift.AnyStruct;
import com.facebook.thrift.type_swift.TypeStruct;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializationProtocolUtil;
import com.facebook.thrift.util.SerializerUtil;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class ThriftAnyStandardProtocolTest {

  @Rule public ExpectedException expectedException = ExpectedException.none();

  @Parameterized.Parameters
  public static Collection<Object> data() {
    return Arrays.asList(
        StandardProtocol.COMPACT,
        StandardProtocol.BINARY,
        StandardProtocol.JSON,
        StandardProtocol.SIMPLE_JSON);
  }

  private final StandardProtocol standardProtocol;

  public ThriftAnyStandardProtocolTest(StandardProtocol standardProtocol) {
    this.standardProtocol = standardProtocol;
  }

  private <T> Any<T> createAny(T o) {
    return new Any.Builder<>(o).setProtocol(standardProtocol).build();
  }

  private <T> Any<T> createAny(T o, Class... clazz) {
    return new Any.Builder<>(o, clazz).setProtocol(standardProtocol).build();
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

  private <T> TypeName getParams(Any<T> any, int inx) {
    return any.getAny().getType().getParams().get(inx).getName();
  }

  private <T> TypeStruct getTypeStruct(Any<T> any, int inx) {
    return any.getAny().getType().getParams().get(inx);
  }

  @Test
  public void testBool() {
    Any<Boolean> any = createAny(true);
    assertEquals(true, any.get());
    assertTrue(getType(any).isSetBoolType());
    assertParamsEmpty(any);

    Any<Boolean> received = Any.wrap(any.getAny());
    assertEquals(true, received.get());
    assertTrue(getType(received).isSetBoolType());
    assertParamsEmpty(received);
  }

  @Test
  public void testByte() {
    Any<Byte> any = createAny((byte) 3);
    assertEquals(3, (byte) any.get());
    assertTrue(getType(any).isSetByteType());
    assertParamsEmpty(any);

    Any<Byte> received = Any.wrap(any.getAny());
    assertEquals(3, (byte) received.get());
    assertTrue(getType(received).isSetByteType());
    assertParamsEmpty(received);
  }

  @Test
  public void testShort() {
    Any<Short> any = createAny((short) 1000);
    assertEquals(1000, (short) any.get());
    assertTrue(getType(any).isSetI16Type());
    assertParamsEmpty(any);

    Any<Short> received = Any.wrap(any.getAny());
    assertEquals(1000, (short) received.get());
    assertTrue(getType(received).isSetI16Type());
    assertParamsEmpty(received);
  }

  @Test
  public void testInteger() {
    Any<Integer> any = createAny(500);
    assertEquals(500, (int) any.get());
    assertTrue(getType(any).isSetI32Type());
    assertParamsEmpty(any);

    Any<Integer> received = Any.wrap(any.getAny());
    assertEquals(500, (int) received.get());
    assertTrue(getType(received).isSetI32Type());
    assertParamsEmpty(received);
  }

  @Test
  public void testLong() {
    Any<Long> any = createAny((long) Integer.MAX_VALUE);
    assertEquals(Integer.MAX_VALUE, (long) any.get());
    assertTrue(getType(any).isSetI64Type());
    assertParamsEmpty(any);

    Any<Long> received = Any.wrap(any.getAny());
    assertEquals(Integer.MAX_VALUE, (long) received.get());
    assertTrue(getType(received).isSetI64Type());
    assertParamsEmpty(received);
  }

  @Test
  public void testFloat() {
    Any<Float> any = createAny(100.77f);
    assertEquals(100.77f, any.get(), 0.1);
    assertTrue(getType(any).isSetFloatType());
    assertParamsEmpty(any);

    Any<Float> received = Any.wrap(any.getAny());
    assertEquals(100.77f, received.get(), 0.1);
    assertTrue(getType(received).isSetFloatType());
    assertParamsEmpty(received);
  }

  @Test
  public void testDouble() {
    Any<Double> any = createAny(200.3d);
    assertEquals(200.3d, any.get(), 0.1);
    assertTrue(getType(any).isSetDoubleType());
    assertParamsEmpty(any);

    Any<Double> received = Any.wrap(any.getAny());
    assertEquals(200.3d, received.get(), 0.1);
    assertTrue(getType(received).isSetDoubleType());
    assertParamsEmpty(received);
  }

  @Test
  public void testString() {
    Any<String> any = createAny("foo");
    assertEquals("foo", any.get());
    assertTrue(getType(any).isSetStringType());
    assertParamsEmpty(any);

    Any<String> received = Any.wrap(any.getAny());
    assertEquals("foo", received.get());
    assertTrue(getType(received).isSetStringType());
    assertParamsEmpty(received);
  }

  private String hexDump(ByteBuf byteBuf) {
    return ByteBufUtil.hexDump(byteBuf);
  }

  private String hexDump(byte[] bytes) {
    return ByteBufUtil.hexDump(bytes);
  }

  private void assertReceivedByteBuf(AnyStruct anyStruct) {
    Any<ByteBuf> received = Any.wrap(anyStruct);
    assertEquals(hexDump("foo".getBytes()), hexDump(received.get()));
    assertTrue(getType(received).isSetBinaryType());
    assertParamsEmpty(received);
  }

  @Test
  public void testByteArrayBinary() {
    Any<byte[]> any = createAny("foo".getBytes());
    assertArrayEquals("foo".getBytes(), any.get());
    assertTrue(getType(any).isSetBinaryType());
    assertParamsEmpty(any);

    assertReceivedByteBuf(any.getAny());
  }

  @Test
  public void testByteBufferBinary() {
    Any<ByteBuffer> any = createAny(ByteBuffer.wrap("foo".getBytes()));
    assertArrayEquals("foo".getBytes(), any.get().array());
    assertTrue(getType(any).isSetBinaryType());
    assertParamsEmpty(any);

    assertReceivedByteBuf(any.getAny());
  }

  @Test
  public void testByteBufBinary() {
    ByteBuf byteBuf = Unpooled.wrappedBuffer("foo".getBytes());
    Any<ByteBuf> any = createAny(byteBuf);
    assertEquals(hexDump("foo".getBytes()), hexDump(any.get()));
    assertTrue(getType(any).isSetBinaryType());
    assertParamsEmpty(any);

    assertReceivedByteBuf(any.getAny());
  }

  private void assertListOfString(Any<List<String>> any) {
    assertEquals(3, any.get().size());
    assertArrayEquals(new String[] {"foo", "bar", "baz"}, any.get().toArray());
    assertTrue(getType(any).isSetListType());
    assertTrue(getParams(any, 0).isSetStringType());
  }

  @Test
  public void testListOfString() {
    Any<List<String>> any = createAny(Arrays.asList("foo", "bar", "baz"), String.class);
    assertListOfString(any);

    Any<List<String>> received = Any.wrap(any.getAny());
    assertListOfString(received);
  }

  private void assertListOfInteger(Any<List<Integer>> any) {
    assertEquals(3, any.get().size());
    assertArrayEquals(new Integer[] {5, 6, 7}, any.get().toArray());
    assertTrue(getType(any).isSetListType());
    assertTrue(getParams(any, 0).isSetI32Type());
  }

  @Test
  public void testListOfInteger() {
    Any<List<Integer>> any = createAny(Arrays.asList(5, 6, 7), Integer.class);
    assertListOfInteger(any);

    Any<List<Integer>> received = Any.wrap(any.getAny());
    assertListOfInteger(received);
  }

  @Test
  public void testListOfFloat() {
    Any<List<Float>> any = createAny(Arrays.asList(5.1f, 6.1f, 7.1f), Float.class);

    Any<List<Float>> received = Any.wrap(any.getAny());
    assertEquals(3, received.get().size());
    assertArrayEquals(new Float[] {5.1f, 6.1f, 7.1f}, any.get().toArray());
  }

  @Test
  public void testEmptyList() {
    Any<List<String>> any = createAny(Collections.EMPTY_LIST, String.class);
    assertEquals(0, any.get().size());
    assertEquals(Collections.EMPTY_LIST, any.get());
    assertTrue(getType(any).isSetListType());
    assertTrue(getParams(any, 0).isSetStringType());

    Any<List<String>> received = Any.wrap(any.getAny());
    assertEquals(0, received.get().size());
    assertEquals(Collections.EMPTY_LIST, received.get());
    assertTrue(getType(received).isSetListType());
    assertTrue(getParams(received, 0).isSetStringType());
  }

  private void assertSetOfInteger(Any<Set<Integer>> any) {
    assertEquals(3, any.get().size());
    Integer[] arr = any.get().toArray(new Integer[] {});
    Arrays.sort(arr);
    assertArrayEquals(new Integer[] {3, 4, 5}, arr);
    assertTrue(getType(any).isSetSetType());
    assertTrue(getParams(any, 0).isSetI32Type());
  }

  @Test
  public void testSetOfInteger() {
    Any<Set<Integer>> any = createAny(new HashSet<>(Arrays.asList(3, 4, 5)), Integer.class);
    assertSetOfInteger(any);

    Any<Set<Integer>> received = Any.wrap(any.getAny());
    assertSetOfInteger(received);
  }

  private void assertMap(Any<Map<Short, Long>> any) {
    assertEquals(2, any.get().size());
    assertEquals(13L, (long) any.get().get((short) 5));
    assertEquals(20L, (long) any.get().get((short) 7));

    assertTrue(getType(any).isSetMapType());
    assertTrue(getParams(any, 0).isSetI16Type());
    assertTrue(getParams(any, 1).isSetI64Type());
  }

  private void assertIntegerMap(Any<Map<Integer, Integer>> any) {
    assertEquals(2, any.get().size());
    assertEquals(13, (long) any.get().get(5));
    assertEquals(20, (long) any.get().get(7));

    assertTrue(getType(any).isSetMapType());
    assertTrue(getParams(any, 0).isSetI32Type());
    assertTrue(getParams(any, 1).isSetI32Type());
  }

  @Test
  public void testMap() {
    Map<Short, Long> map = new HashMap<>();
    map.put((short) 5, 13L);
    map.put((short) 7, 20L);

    Any<Map<Short, Long>> any = createAny(map, Short.class, Long.class);
    assertMap(any);

    Any<Map<Short, Long>> received = Any.wrap(any.getAny());
    assertMap(received);
  }

  @Test
  public void testIntegerMap() {
    Map<Integer, Integer> map = new HashMap<>();
    map.put(5, 13);
    map.put(7, 20);

    Any<Map<Integer, Integer>> any = createAny(map, Integer.class, Integer.class);
    assertIntegerMap(any);

    Any<Map<Integer, Integer>> received = Any.wrap(any.getAny());
    assertIntegerMap(received);
  }

  private void assertDoubleMap(Any<Map<Double, String>> any) {
    assertEquals(2, any.get().size());
    assertEquals("bar", any.get().get(1d));
    assertEquals("baz", any.get().get(2d));

    assertTrue(getType(any).isSetMapType());
    assertTrue(getParams(any, 0).isSetDoubleType());
    assertTrue(getParams(any, 1).isSetStringType());
  }

  @Test
  public void testDoubleMap() {
    Map<Double, String> map = new HashMap<>();
    map.put(1d, "bar");
    map.put(2d, "baz");

    Any<Map<Double, String>> any = createAny(map, Double.class, String.class);
    assertDoubleMap(any);

    Any<Map<Double, String>> received = Any.wrap(any.getAny());
    assertDoubleMap(received);
  }

  private void assertListOfList(Any<List<List<String>>> any) {
    assertEquals(2, any.get().size());
    assertArrayEquals(new String[] {"a", "b"}, any.get().get(0).toArray());
    assertArrayEquals(new String[] {"foo", "bar", "baz"}, any.get().get(1).toArray());
    assertTrue(getType(any).isSetListType());
    assertTrue(getParams(any, 0).isSetListType());
    assertTrue(getTypeStruct(any, 0).getParams().get(0).getName().isSetStringType());
  }

  @Test
  public void testListOfList() {
    Any<List<List<String>>> any =
        createAny(
            Arrays.asList(Arrays.asList("a", "b"), Arrays.asList("foo", "bar", "baz")),
            List.class,
            String.class);
    assertListOfList(any);

    Any<List<List<String>>> received = Any.wrap(any.getAny());
    assertListOfList(received);
  }

  private void assertMapListOfList(Any<Map<Float, List<List<String>>>> any) {
    assertEquals(2, any.get().size());
    assertArrayEquals(new String[] {"a", "b", "c"}, any.get().get(5f).get(0).toArray());
    assertArrayEquals(new String[] {"foo", "bar", "baz"}, any.get().get(5f).get(1).toArray());
    assertArrayEquals(new String[] {"d"}, any.get().get(6f).get(0).toArray());
    assertArrayEquals(new String[] {"e"}, any.get().get(6f).get(1).toArray());

    assertTrue(getType(any).isSetMapType());
    assertTrue(getParams(any, 0).isSetFloatType());
    assertTrue(getParams(any, 1).isSetListType());

    assertTrue(getTypeStruct(any, 1).getParams().get(0).getName().isSetListType());
    assertTrue(
        getTypeStruct(any, 1).getParams().get(0).getParams().get(0).getName().isSetStringType());
  }

  @Test
  public void testMapListOfList() {
    Map<Float, List<List<String>>> map = new HashMap<>();
    map.put(5f, Arrays.asList(Arrays.asList("a", "b", "c"), Arrays.asList("foo", "bar", "baz")));
    map.put(6f, Arrays.asList(Arrays.asList("d"), Arrays.asList("e")));

    Any<Map<Float, List<List<String>>>> any =
        createAny(map, Float.class, List.class, List.class, String.class);
    assertMapListOfList(any);

    Any<Map<Float, List<List<String>>>> received = Any.wrap(any.getAny());
    assertMapListOfList(received);
  }

  private void assertMapOfMap(Any<Map<Boolean, Map<String, Integer>>> any) {
    assertEquals(2, any.get().size());
    assertEquals(4, (long) any.get().get(true).get("foo"));
    assertEquals(5, (long) any.get().get(true).get("bar"));

    assertEquals(14, (long) any.get().get(false).get("foo"));
    assertEquals(15, (long) any.get().get(false).get("bar"));
    assertEquals(16, (long) any.get().get(false).get("baz"));

    assertTrue(getType(any).isSetMapType());
    assertTrue(getParams(any, 0).isSetBoolType());
    assertTrue(getParams(any, 1).isSetMapType());
  }

  @Test
  public void testMapOfMap() {
    Map<Boolean, Map<String, Integer>> map = new HashMap<>();
    Map<String, Integer> map1 = new HashMap<>();
    map1.put("foo", 4);
    map1.put("bar", 5);

    Map<String, Integer> map2 = new HashMap<>();
    map2.put("foo", 14);
    map2.put("bar", 15);
    map2.put("baz", 16);

    map.put(true, map1);
    map.put(false, map2);

    Any<Map<Boolean, Map<String, Integer>>> any =
        createAny(map, Boolean.class, Map.class, String.class, Integer.class);
    assertMapOfMap(any);

    Any<Map<Boolean, Map<String, Integer>>> received = Any.wrap(any.getAny());
    assertMapOfMap(received);
  }

  @Test
  public void testStruct() {
    List<Integer> list = new ArrayList<>();
    list.add(5);
    TestStruct st =
        new TestStruct.Builder().setBoolField(true).setInfField(9).setListField(list).build();
    TestStruct copy =
        new TestStruct.Builder().setBoolField(true).setInfField(9).setListField(list).build();

    ByteBuf dest = RpcResources.getUnpooledByteBufAllocator().buffer();
    ByteBufTProtocol protocol =
        SerializerUtil.toByteBufProtocol(SerializationProtocol.TSimpleJSON, dest);
    st.write0(protocol);

    Any<TestStruct> any = createAny(st);
    assertEquals(copy, any.get());
    assertTrue(getType(any).isSetStructType());
    assertParamsEmpty(any);

    Any<TestStruct> received = Any.wrap(any.getAny());
    assertEquals(copy, received.get());
    assertTrue(getType(received).isSetStructType());
    assertParamsEmpty(received);
  }

  @Test
  public void testListOfStruct() {
    List<TestStruct> list = new ArrayList<>();
    list.add(new TestStruct.Builder().setInfField(10).build());
    list.add(new TestStruct.Builder().setInfField(11).build());

    Any<List<TestStruct>> any = createAny(list, TestStruct.class);
    assertEquals(10, any.get().get(0).getInfField());
    assertEquals(11, any.get().get(1).getInfField());
    assertTrue(getType(any).isSetListType());
    assertTrue(getParams(any, 0).isSetStructType());

    Any<List<TestStruct>> received = Any.wrap(any.getAny());
    assertEquals(10, received.get().get(0).getInfField());
    assertEquals(11, received.get().get(1).getInfField());
    assertTrue(getType(received).isSetListType());
    assertTrue(getParams(received, 0).isSetStructType());
  }

  @Test
  public void testException() {
    TestException st = new TestException.Builder().setBoolField(true).setInfField(12).build();

    ByteBuf dest = RpcResources.getUnpooledByteBufAllocator().buffer();
    ByteBufTProtocol protocol =
        SerializerUtil.toByteBufProtocol(SerializationProtocol.TSimpleJSON, dest);
    st.write0(protocol);

    Any<TestException> any = createAny(st);
    assertEquals(12, any.get().getInfField());
    assertTrue(getType(any).isSetExceptionType());
    assertParamsEmpty(any);

    Any<TestException> received = Any.wrap(any.getAny());
    assertEquals(12, received.get().getInfField());
    assertTrue(getType(received).isSetExceptionType());
    assertParamsEmpty(received);
  }

  @Test
  public void testUnion() {
    TestUnion st = TestUnion.fromInfField(13);

    Any<TestUnion> any = createAny(st);
    assertEquals(13, any.get().getInfField());
    assertTrue(getType(any).isSetUnionType());
    assertParamsEmpty(any);

    Any<TestUnion> received = Any.wrap(any.getAny());
    assertEquals(13, received.get().getInfField());
    assertTrue(getType(received).isSetUnionType());
    assertParamsEmpty(received);
  }

  @Test
  public void testEnum() {
    TestEnum e = TestEnum.TWO;

    Any<TestEnum> any = createAny(e);
    assertEquals(TestEnum.TWO, any.get());
    assertTrue(getType(any).isSetEnumType());
    assertParamsEmpty(any);

    Any<TestEnum> received = Any.wrap(any.getAny());
    assertEquals(TestEnum.TWO, received.get());
    assertTrue(getType(any).isSetEnumType());
    assertParamsEmpty(received);
  }

  @Test
  public void testOpenEnum() {
    TestOpenEnum e = TestOpenEnum.TWO;

    Any<TestOpenEnum> any = createAny(e);
    assertEquals(TestOpenEnum.TWO, any.get());
    assertTrue(getType(any).isSetEnumType());
    assertParamsEmpty(any);

    Any<TestOpenEnum> received = Any.wrap(any.getAny());
    assertEquals(TestOpenEnum.TWO, received.get());
    assertTrue(getType(any).isSetEnumType());
    assertParamsEmpty(received);
  }

  @Test
  public void testAnyAdapter() {
    Any<List<Integer>> any = createAny(Arrays.asList(5, 6, 7), Integer.class);
    TestStruct st = new TestStruct.Builder().setAnyField(any).build();

    byte[] bytes =
        SerializerUtil.toByteArray(st, SerializationProtocolUtil.getProtocol(standardProtocol));
    TestStruct received =
        SerializerUtil.fromByteArray(
            TestStruct.asReader(), bytes, SerializationProtocolUtil.getProtocol(standardProtocol));
    assertArrayEquals(
        new Integer[] {5, 6, 7}, ((List<Integer>) received.getAnyField().get()).toArray());
  }
}
