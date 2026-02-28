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

import static org.junit.Assert.*;
import static org.junit.Assert.assertArrayEquals;

import com.facebook.thrift.any.Any;
import com.facebook.thrift.test.adapter.AdaptedTestUnion;
import com.facebook.thrift.test.adapter.TestUnion;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import java.util.Arrays;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ThreadLocalRandom;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class TypeAdapterUnionTest {

  @Parameterized.Parameters
  public static Collection<Object> data() {
    return Arrays.asList(SerializationProtocol.TCompact, SerializationProtocol.TBinary);
  }

  private final SerializationProtocol protocol;

  public TypeAdapterUnionTest(SerializationProtocol protocol) {
    this.protocol = protocol;
  }

  @Test
  public void testSerializeTypeAdapterWithEmptyByteBufs() {
    AdaptedTestUnion struct = AdaptedTestUnion.defaultInstance();
    byte[] bytes = SerializerUtil.toByteArray(struct, protocol);
    AdaptedTestUnion fromBytes =
        SerializerUtil.fromByteArray(AdaptedTestUnion.asReader(), bytes, protocol);
    assertEquals(struct, fromBytes);
  }

  private static ByteBuf generateByteBuf() {
    byte[] bytes = new byte[32];
    ThreadLocalRandom.current().nextBytes(bytes);
    return Unpooled.wrappedBuffer(bytes);
  }

  private byte[] serializeAdapted(AdaptedTestUnion union) {
    return SerializerUtil.toByteArray(union, protocol);
  }

  private AdaptedTestUnion deserializeAdapted(byte[] bytes) {
    return SerializerUtil.fromByteArray(AdaptedTestUnion.asReader(), bytes, protocol);
  }

  private TestUnion deserialize(byte[] bytes) {
    return SerializerUtil.fromByteArray(TestUnion.asReader(), bytes, protocol);
  }

  private String hexDump(ByteBuf byteBuf) {
    return ByteBufUtil.hexDump(byteBuf);
  }

  private String hexDump(byte[] bytes) {
    return ByteBufUtil.hexDump(bytes);
  }

  @Test
  public void testBooleanToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedBooleanField("true"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("true", adapted.isAdaptedBooleanField());
    assertEquals(true, received.isBooleanField());
  }

  @Test
  public void testByteToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedByteField("100"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("100", adapted.getAdaptedByteField());
    assertEquals(100, received.getByteField());
  }

  @Test
  public void testShortToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedShortField("100"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("100", adapted.getAdaptedShortField());
    assertEquals(100, received.getShortField());
  }

  @Test
  public void testIntToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedIntField("100"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("100", adapted.getAdaptedIntField());
    assertEquals(100, received.getIntField());
  }

  @Test
  public void testLongToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedLongField("1000000000000"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("1000000000000", adapted.getAdaptedLongField());
    assertEquals(1000000000000L, received.getLongField());
  }

  @Test
  public void testFloatToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedFloatField("1700.15"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("1700.15", adapted.getAdaptedFloatField());
    assertEquals(1700.15, received.getFloatField(), 0.1);
  }

  @Test
  public void testDoubleToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedDoubleField("9283.332"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("9283.332", adapted.getAdaptedDoubleField());
    assertEquals(9283.332, received.getDoubleField(), 0.1);
  }

  @Test
  public void testStringTypeAdapter() {
    byte[] bytes =
        serializeAdapted(
            AdaptedTestUnion.fromAdaptedStringField(Unpooled.wrappedBuffer("foo".getBytes())));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals(Unpooled.wrappedBuffer("foo".getBytes()), adapted.getAdaptedStringField());
    assertEquals("foo", received.getStringField());
  }

  @Test
  public void testSlicedByteBufTypeAdapter() {
    final ByteBuf b = generateByteBuf();
    final String s = hexDump(b);

    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromB1(b));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals(s, hexDump(adapted.getB1()));
    assertEquals(s, hexDump(received.getB1()));
  }

  @Test
  public void testCopiedByteBufTypeAdapter() {
    final ByteBuf b = generateByteBuf();
    final String s = hexDump(b);

    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromB2(b));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals(s, hexDump(adapted.getB2()));
    assertEquals(s, hexDump(received.getB2()));
  }

  @Test
  public void testUnpooledByteBufTypeAdapter() {
    final ByteBuf b = generateByteBuf();
    final String s = hexDump(b);

    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromB3(b));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals(s, hexDump(adapted.getB3()));
    assertEquals(s, hexDump(received.getB3()));
  }

  @Test
  public void testDateTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromDateField(new Date(6700)));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals(6700, adapted.getDateField().getTime());
    assertEquals(6700, received.getDateField());
  }

  @Test
  public void testIntListToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedIntListField("1,2,3,4"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("1,2,3,4", adapted.getAdaptedIntListField());
    assertEquals(Arrays.asList(1, 2, 3, 4), received.getIntListField());
  }

  @Test
  public void testHexListTypeAdapter() {
    byte[] bytes =
        serializeAdapted(AdaptedTestUnion.fromAdaptedBinaryListField("ab0503:ddee:a1a2a3a4"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("ab0503:ddee:a1a2a3a4", adapted.getAdaptedBinaryListField());
    assertEquals("ab0503", hexDump(received.getBinaryListField().get(0)));
    assertEquals("ddee", hexDump(received.getBinaryListField().get(1)));
    assertEquals("a1a2a3a4", hexDump(received.getBinaryListField().get(2)));
  }

  @Test
  public void testStringListStringTypeAdapter() {
    byte[] bytes =
        serializeAdapted(AdaptedTestUnion.fromAdaptedListIntListField("1,2,3:4,4:7,8,9,10"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("1,2,3:4,4:7,8,9,10", adapted.getAdaptedListIntListField());
    assertEquals(Arrays.asList(1, 2, 3), received.getListIntListField().get(0));
    assertEquals(Arrays.asList(4, 4), received.getListIntListField().get(1));
    assertEquals(Arrays.asList(7, 8, 9, 10), received.getListIntListField().get(2));
  }

  @Test
  public void testIntSetToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedIntSetField("7,7,6,7"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertTrue(adapted.getAdaptedIntSetField().indexOf("6") >= 0);
    assertTrue(adapted.getAdaptedIntSetField().indexOf("7") >= 0);
    assertEquals(new HashSet<>(Arrays.asList(7, 6)), received.getIntSetField());
  }

  @Test
  public void testBinarySetToStringTypeAdapter() {
    byte[] bytes =
        serializeAdapted(AdaptedTestUnion.fromAdaptedBinarySetField("ab0503,ddee,a1a2a3"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertTrue(adapted.getAdaptedBinarySetField().indexOf("ab0503") >= 0);
    assertTrue(adapted.getAdaptedBinarySetField().indexOf("ddee") >= 0);
    assertTrue(adapted.getAdaptedBinarySetField().indexOf("a1a2a3") >= 0);

    for (String s :
        received.getBinarySetField().stream()
            .map(ByteBufUtil::hexDump)
            .collect(Collectors.toList())) {
      assertTrue("ab0503,ddee,a1a2a3".indexOf(s) >= 0);
    }
  }

  @Test
  public void testIntMapToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedIntMapField("5=9,7=12"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertTrue(adapted.getAdaptedIntMapField().indexOf("5=9") >= 0);
    assertTrue(adapted.getAdaptedIntMapField().indexOf("7=12") >= 0);
    assertEquals(
        Stream.of(new int[][] {{5, 9}, {7, 12}}).collect(Collectors.toMap(d -> d[0], d -> d[1])),
        received.getIntMapField());
  }

  @Test
  public void testIntBinaryMapToStringTypeAdapter() {
    byte[] bytes =
        serializeAdapted(AdaptedTestUnion.fromAdaptedIntBinaryMapField("3=aabb,4=12abef"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertTrue(adapted.getAdaptedIntBinaryMapField().indexOf("3=aabb") >= 0);
    assertTrue(adapted.getAdaptedIntBinaryMapField().indexOf("4=12abef") >= 0);
    assertEquals("aabb", hexDump(received.getIntBinaryMapField().get(3)));
    assertEquals("12abef", hexDump(received.getIntBinaryMapField().get(4)));
  }

  @Test
  public void testIntStringMapToStringTypeAdapter() {
    byte[] bytes =
        serializeAdapted(AdaptedTestUnion.fromAdaptedIntStringMapField("3=aabb,4=12abef"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertTrue(adapted.getAdaptedIntStringMapField().indexOf("3=aabb") >= 0);
    assertTrue(adapted.getAdaptedIntStringMapField().indexOf("4=12abef") >= 0);
    assertEquals("aabb", received.getIntStringMapField().get(3));
    assertEquals("12abef", received.getIntStringMapField().get(4));
  }

  @Test
  public void testIntBinaryStringMapToStringTypeAdapter() {
    byte[] bytes =
        serializeAdapted(AdaptedTestUnion.fromAdaptedIntBinaryStringMapField("3=aabb,4=12abef"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertTrue(adapted.getAdaptedIntBinaryStringMapField().indexOf("3=aabb") >= 0);
    assertTrue(adapted.getAdaptedIntBinaryStringMapField().indexOf("4=12abef") >= 0);
    assertEquals("aabb", hexDump(received.getIntBinaryStringMapField().get(3)));
    assertEquals("12abef", hexDump(received.getIntBinaryStringMapField().get(4)));
  }

  @Test
  public void testIntBinaryListMapToStringTypeAdapter() {
    byte[] bytes =
        serializeAdapted(AdaptedTestUnion.fromAdaptedIntBinaryListMapField("3=aabb:de,9=12:22:31"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertTrue(adapted.getAdaptedIntBinaryListMapField().indexOf("3=aabb:de") >= 0);
    assertTrue(adapted.getAdaptedIntBinaryListMapField().indexOf("9=12:22:31") >= 0);
    assertEquals("aabb", hexDump(received.getIntBinaryListMapField().get(3).get(0)));
    assertEquals("de", hexDump(received.getIntBinaryListMapField().get(3).get(1)));
    assertEquals("31", hexDump(received.getIntBinaryListMapField().get(9).get(2)));
  }

  @Test
  public void testBooleanToStringInlineTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedBooleanField2("true"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("true", adapted.isAdaptedBooleanField2());
    assertEquals(true, received.isBooleanField2());
  }

  @Test
  public void testHexListInlineTypeAdapter() {
    byte[] bytes =
        serializeAdapted(AdaptedTestUnion.fromAdaptedBinaryListField2("ab0503:ddee:a1a2a3a4"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("ab0503:ddee:a1a2a3a4", adapted.getAdaptedBinaryListField2());
    assertEquals("ab0503", hexDump(received.getBinaryListField2().get(0)));
    assertEquals("ddee", hexDump(received.getBinaryListField2().get(1)));
    assertEquals("a1a2a3a4", hexDump(received.getBinaryListField2().get(2)));
  }

  @Test
  public void testDoubleAdaptedIntTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromDoubleAdaptedIntField(5000L));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals((Long) 5000L, (Long) adapted.getDoubleAdaptedIntField());
    assertEquals(5000, received.getIntField2());
  }

  @Test
  public void testDoubleTypeDefIntTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromDoubleTypedefAdaptedIntField("75"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("75", adapted.getDoubleTypedefAdaptedIntField());
    assertEquals(75, received.getDoubleTypedefIntField());
  }

  @Test
  public void testMultipleTypeDefIntTypeAdapter() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromMultipleTypedefAdaptedIntField("60"));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("60", adapted.getMultipleTypedefAdaptedIntField());
    assertEquals(60, received.getMultipleTypedefIntField());
  }

  @Test
  public void testAdaptedSetList() {
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedSetListField(Arrays.asList("7")));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertTrue(adapted.getAdaptedSetListField().get(0).contains("7"));
    assertTrue(received.getAdaptedSetListField().get(0).contains(7));
  }

  @Test
  public void testListOfAdaptedInt() {
    List<String> list = Arrays.asList("1", "2", "3");
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromListAdaptedIntField(list));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertArrayEquals(list.toArray(), adapted.getListAdaptedIntField().toArray());
    assertArrayEquals(new Integer[] {1, 2, 3}, received.getListAdaptedIntField().toArray());
  }

  @Test
  public void testMapOfMapAdaptedInt() {
    Map<String, Map<String, String>> map = new HashMap<>();
    Map<String, String> map1 = new HashMap<>();
    map1.put("5", "13");
    map.put("7", map1);

    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromMapOfIntToMapIntToShortField(map));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("13", adapted.getMapOfIntToMapIntToShortField().get("7").get("5"));
    assertEquals(13L, (long) received.getMapOfIntToMapIntToShortField().get(7).get(5));
  }

  @Test
  public void testListOfAdaptedShort() {
    List<String> list = Arrays.asList("10", "11");
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedShortListField(list));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertArrayEquals(list.toArray(), adapted.getAdaptedShortListField().toArray());
    assertArrayEquals(new Short[] {10, 11}, received.getAdaptedShortListField().toArray());
  }

  @Test
  public void testNestedListOfAdaptedBinary() {
    byte[] foo = "foo".getBytes();
    byte[] bar = "bar".getBytes();

    List<List<List<ByteBuf>>> list =
        Arrays.asList(
            Arrays.asList(Arrays.asList(Unpooled.wrappedBuffer(foo), Unpooled.wrappedBuffer(bar))));
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedBinList3Field(list));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals(hexDump(foo), hexDump(adapted.getAdaptedBinList3Field().get(0).get(0).get(0)));
    assertEquals(hexDump(bar), hexDump(adapted.getAdaptedBinList3Field().get(0).get(0).get(1)));
    assertArrayEquals(foo, received.getAdaptedBinList3Field().get(0).get(0).get(0));
    assertArrayEquals(bar, received.getAdaptedBinList3Field().get(0).get(0).get(1));
  }

  @Test
  public void testListOfMultiLevelTypeDef() {
    byte[] foo = "foo".getBytes();
    byte[] bar = "bar".getBytes();

    List<ByteBuf> list = Arrays.asList(Unpooled.wrappedBuffer(foo), Unpooled.wrappedBuffer(bar));
    byte[] bytes = serializeAdapted(AdaptedTestUnion.fromAdaptedBinNestedTypeDefListField(list));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals(hexDump(foo), hexDump(adapted.getAdaptedBinNestedTypeDefListField().get(0)));
    assertEquals(hexDump(bar), hexDump(adapted.getAdaptedBinNestedTypeDefListField().get(1)));
    assertArrayEquals(foo, received.getAdaptedBinNestedTypeDefListField().get(0));
    assertArrayEquals(bar, received.getAdaptedBinNestedTypeDefListField().get(1));
  }

  @Test
  public void testStructList() {
    byte[] bytes =
        serializeAdapted(
            AdaptedTestUnion.fromAnyListField(Arrays.asList(new Any.Builder("foo").build())));
    AdaptedTestUnion adapted = deserializeAdapted(bytes);
    TestUnion received = deserialize(bytes);

    assertEquals("foo", adapted.getAnyListField().get(0).get());
    assertEquals("foo", received.getAnyListField().get(0).get());
  }
}
