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

import com.facebook.thrift.adapter.test.Wrapped;
import com.facebook.thrift.any.Any;
import com.facebook.thrift.test.adapter.AdaptedTestException;
import com.facebook.thrift.test.adapter.AdaptedTestStruct;
import com.facebook.thrift.test.adapter.AdaptedTestStructWithoutDefaults;
import com.facebook.thrift.test.adapter.InnerException;
import com.facebook.thrift.test.adapter.InnerUnion;
import com.facebook.thrift.test.adapter.TestStruct;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ThreadLocalRandom;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class TypeAdapterStructTest {

  private static class Param {
    protected SerializationProtocol protocol;
    protected boolean exception;

    public Param(SerializationProtocol protocol, boolean exception) {
      this.protocol = protocol;
      this.exception = exception;
    }
  }

  @Parameterized.Parameters
  public static Collection<Param> data() {
    return Arrays.asList(
        new Param(SerializationProtocol.TCompact, false),
        new Param(SerializationProtocol.TCompact, true),
        new Param(SerializationProtocol.TBinary, false),
        new Param(SerializationProtocol.TBinary, true));
  }

  private final SerializationProtocol protocol;
  private final boolean exception;

  public TypeAdapterStructTest(Param param) {
    this.protocol = param.protocol;
    this.exception = param.exception;
  }

  private static ByteBuf generateByteBuf() {
    byte[] bytes = new byte[32];
    ThreadLocalRandom.current().nextBytes(bytes);
    return Unpooled.wrappedBuffer(bytes);
  }

  private AdaptedTestStruct.Builder defaultInstance() {
    return new AdaptedTestStruct.Builder();
  }

  private byte[] serializeAdapted(AdaptedTestStruct struct) {
    return SerializerUtil.toByteArray(struct, protocol);
  }

  private AdaptedTestStruct copy(AdaptedTestException from) {
    AdaptedTestStruct.Builder to = new AdaptedTestStruct.Builder();

    for (Field f : from.getClass().getDeclaredFields()) {
      try {
        Field t = to.getClass().getDeclaredField(f.getName());
        f.setAccessible(true);
        t.setAccessible(true);
        t.set(to, f.get(from));
      } catch (Throwable t) {
        // ignore
      }
    }

    return to.build();
  }

  private AdaptedTestStruct deserializeAdapted(byte[] bytes) {
    // test both Struct and Exception
    if (exception) {
      return copy(SerializerUtil.fromByteArray(AdaptedTestException.asReader(), bytes, protocol));
    }

    return SerializerUtil.fromByteArray(AdaptedTestStruct.asReader(), bytes, protocol);
  }

  private TestStruct deserialize(byte[] bytes) {
    return SerializerUtil.fromByteArray(TestStruct.asReader(), bytes, protocol);
  }

  private String hexDump(ByteBuf byteBuf) {
    return ByteBufUtil.hexDump(byteBuf);
  }

  private String hexDump(byte[] bytes) {
    return ByteBufUtil.hexDump(bytes);
  }

  private String hexDump(String s) {
    return ByteBufUtil.hexDump(s.getBytes());
  }

  @Test
  public void testBooleanToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedBooleanField("true").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("true", adapted.isAdaptedBooleanField());
    assertEquals(true, received.isBooleanField());
    assertEquals("true", adapted.isAdaptedBooleanDefault());
    assertEquals(true, received.isBooleanDefault());
  }

  @Test
  public void testByteToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedByteField("100").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("100", adapted.getAdaptedByteField());
    assertEquals(100, received.getByteField());
    assertEquals("9", adapted.getAdaptedByteDefault());
    assertEquals(9, received.getByteDefault());
  }

  @Test
  public void testShortToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedShortField("100").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("100", adapted.getAdaptedShortField());
    assertEquals(100, received.getShortField());
    assertEquals("101", adapted.getAdaptedShortDefault());
    assertEquals(101, received.getShortDefault());
  }

  @Test
  public void testIntToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedIntField("100").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("100", adapted.getAdaptedIntField());
    assertEquals(100, received.getIntField());
    assertEquals("1024", adapted.getAdaptedIntDefault());
    assertEquals(1024, received.getIntDefault());
  }

  @Test
  public void testLongToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedLongField("1000000000000").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("1000000000000", adapted.getAdaptedLongField());
    assertEquals(1000000000000L, received.getLongField());
    assertEquals("5000", adapted.getAdaptedLongDefault());
    assertEquals(5000L, received.getLongDefault());
  }

  @Test
  public void testFloatToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedFloatField("1700.15").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("1700.15", adapted.getAdaptedFloatField());
    assertEquals(1700.15, received.getFloatField(), 0.1);
    assertEquals("2.3", adapted.getAdaptedFloatDefault());
    assertEquals(2.3, received.getFloatDefault(), 0.1);
  }

  @Test
  public void testDoubleToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedDoubleField("9283.332").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("9283.332", adapted.getAdaptedDoubleField());
    assertEquals(9283.332, received.getDoubleField(), 0.1);
    assertEquals("5.67", adapted.getAdaptedDoubleDefault());
    assertEquals(5.67, received.getDoubleDefault(), 0.1);
  }

  @Test
  public void testStringTypeAdapter() {
    byte[] bytes =
        serializeAdapted(
            defaultInstance()
                .setAdaptedStringField(Unpooled.wrappedBuffer("foo".getBytes()))
                .build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(Unpooled.wrappedBuffer("foo".getBytes()), adapted.getAdaptedStringField());
    assertEquals("foo", received.getStringField());
    assertEquals(Unpooled.wrappedBuffer("test".getBytes()), adapted.getAdaptedStringDefault());
    assertEquals("test", received.getStringDefault());
  }

  @Test
  public void testSlicedByteBufTypeAdapter() {
    final ByteBuf b = generateByteBuf();
    final String s = hexDump(b);

    byte[] bytes = serializeAdapted(defaultInstance().setB1(b).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(s, hexDump(adapted.getB1()));
    assertEquals(s, hexDump(received.getB1()));
    assertEquals(Unpooled.wrappedBuffer("b1b1".getBytes()), adapted.getB1Default());
    assertEquals(hexDump("b1b1".getBytes()), hexDump(received.getB1Default()));
  }

  @Test
  public void testCopiedByteBufTypeAdapter() {
    final ByteBuf b = generateByteBuf();
    final String s = hexDump(b);

    byte[] bytes = serializeAdapted(defaultInstance().setB2(b).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(s, hexDump(adapted.getB2()));
    assertEquals(s, hexDump(received.getB2()));
    assertEquals(Unpooled.wrappedBuffer("b2b2".getBytes()), adapted.getB2Default());
    assertEquals(hexDump("b2b2".getBytes()), hexDump(received.getB2Default()));
  }

  @Test
  public void testUnpooledByteBufTypeAdapter() {
    final ByteBuf b = generateByteBuf();
    final String s = hexDump(b);

    byte[] bytes = serializeAdapted(defaultInstance().setB3(b).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(s, hexDump(adapted.getB3()));
    assertEquals(s, hexDump(received.getB3()));
    assertEquals(Unpooled.wrappedBuffer("b3b3".getBytes()), adapted.getB3Default());
    assertEquals(hexDump("b3b3".getBytes()), hexDump(received.getB3Default()));
  }

  @Test
  public void testDateTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().setDateField(new Date(6700)).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(6700, adapted.getDateField().getTime());
    assertEquals(6700, received.getDateField());
  }

  @Test
  public void testIntListToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedIntListField("1,2,3,4").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("1,2,3,4", adapted.getAdaptedIntListField());
    assertEquals(Arrays.asList(1, 2, 3, 4), received.getIntListField());
    assertEquals("2,4", adapted.getAdaptedIntListDefault());
    assertEquals(Arrays.asList(2, 4), received.getIntListDefault());
  }

  @Test
  public void testHexListTypeAdapter() {
    byte[] bytes =
        serializeAdapted(
            defaultInstance().setAdaptedBinaryListField("ab0503:ddee:a1a2a3a4").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("ab0503:ddee:a1a2a3a4", adapted.getAdaptedBinaryListField());
    assertEquals("ab0503", hexDump(received.getBinaryListField().get(0)));
    assertEquals("ddee", hexDump(received.getBinaryListField().get(1)));
    assertEquals("a1a2a3a4", hexDump(received.getBinaryListField().get(2)));
    assertEquals(hexDump("aa") + ":" + hexDump("bb"), adapted.getAdaptedBinaryListDefault());
    assertTrue(Arrays.equals("aa".getBytes(), received.getBinaryListDefault().get(0)));
  }

  @Test
  public void testStringListStringTypeAdapter() {
    byte[] bytes =
        serializeAdapted(
            defaultInstance().setAdaptedListIntListField("1,2,3:4,4:7,8,9,10").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("1,2,3:4,4:7,8,9,10", adapted.getAdaptedListIntListField());
    assertEquals(Arrays.asList(1, 2, 3), received.getListIntListField().get(0));
    assertEquals(Arrays.asList(4, 4), received.getListIntListField().get(1));
    assertEquals(Arrays.asList(7, 8, 9, 10), received.getListIntListField().get(2));
  }

  @Test
  public void testIntSetToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedIntSetField("7,7,6,7").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertTrue(adapted.getAdaptedIntSetField().indexOf("6") >= 0);
    assertTrue(adapted.getAdaptedIntSetField().indexOf("7") >= 0);
    assertEquals(new HashSet<>(Arrays.asList(7, 6)), received.getIntSetField());
    assertTrue(adapted.getAdaptedIntSetDefault().indexOf("10") >= 0);
    assertTrue(adapted.getAdaptedIntSetDefault().indexOf("20") >= 0);
    assertEquals(new HashSet<>(Arrays.asList(10, 20)), received.getIntSetDefault());
  }

  @Test
  public void testBinarySetToStringTypeAdapter() {
    byte[] bytes =
        serializeAdapted(defaultInstance().setAdaptedBinarySetField("ab0503,ddee,a1a2a3").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertTrue(adapted.getAdaptedBinarySetField().indexOf("ab0503") >= 0);
    assertTrue(adapted.getAdaptedBinarySetField().indexOf("ddee") >= 0);
    assertTrue(adapted.getAdaptedBinarySetField().indexOf("a1a2a3") >= 0);
    assertTrue(adapted.getAdaptedBinarySetDefault().indexOf(hexDump("foo")) >= 0);
    assertTrue(adapted.getAdaptedBinarySetDefault().indexOf(hexDump("bar")) >= 0);

    for (String s :
        received.getBinarySetField().stream()
            .map(ByteBufUtil::hexDump)
            .collect(Collectors.toList())) {
      assertTrue("ab0503,ddee,a1a2a3".indexOf(s) >= 0);
    }
    for (String s :
        received.getBinarySetDefault().stream()
            .map(ByteBufUtil::hexDump)
            .collect(Collectors.toList())) {
      assertTrue(hexDump("foobar").indexOf(s) >= 0);
    }
  }

  @Test
  public void testIntMapToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedIntMapField("5=9,7=12").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertTrue(adapted.getAdaptedIntMapField().indexOf("5=9") >= 0);
    assertTrue(adapted.getAdaptedIntMapField().indexOf("7=12") >= 0);
    assertEquals(
        Stream.of(new int[][] {{5, 9}, {7, 12}}).collect(Collectors.toMap(d -> d[0], d -> d[1])),
        received.getIntMapField());
    assertTrue(adapted.getAdaptedIntMapDefault().indexOf("1=7") >= 0);
    assertEquals(Collections.singletonMap(1, 7), received.getIntMapDefault());
  }

  @Test
  public void testIntBinaryMapToStringTypeAdapter() {
    byte[] bytes =
        serializeAdapted(defaultInstance().setAdaptedIntBinaryMapField("3=aabb,4=12abef").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertTrue(adapted.getAdaptedIntBinaryMapField().indexOf("3=aabb") >= 0);
    assertTrue(adapted.getAdaptedIntBinaryMapField().indexOf("4=12abef") >= 0);
    assertEquals("aabb", hexDump(received.getIntBinaryMapField().get(3)));
    assertEquals("12abef", hexDump(received.getIntBinaryMapField().get(4)));
    assertEquals(hexDump("foo"), hexDump(received.getIntBinaryMapDefault().get(8)));
    assertEquals(hexDump("foo"), hexDump(received.getIntBinaryMapDefault().get(8)));
  }

  @Test
  public void testIntStringMapToStringTypeAdapter() {
    byte[] bytes =
        serializeAdapted(defaultInstance().setAdaptedIntStringMapField("3=aabb,4=12abef").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertTrue(adapted.getAdaptedIntStringMapField().indexOf("3=aabb") >= 0);
    assertTrue(adapted.getAdaptedIntStringMapField().indexOf("4=12abef") >= 0);
    assertEquals("aabb", received.getIntStringMapField().get(3));
    assertEquals("12abef", received.getIntStringMapField().get(4));
  }

  @Test
  public void testIntBinaryStringMapToStringTypeAdapter() {
    byte[] bytes =
        serializeAdapted(
            defaultInstance().setAdaptedIntBinaryStringMapField("3=aabb,4=12abef").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertTrue(adapted.getAdaptedIntBinaryStringMapField().indexOf("3=aabb") >= 0);
    assertTrue(adapted.getAdaptedIntBinaryStringMapField().indexOf("4=12abef") >= 0);
    assertEquals("aabb", hexDump(received.getIntBinaryStringMapField().get(3)));
    assertEquals("12abef", hexDump(received.getIntBinaryStringMapField().get(4)));
  }

  @Test
  public void testIntBinaryListMapToStringTypeAdapter() {
    byte[] bytes =
        serializeAdapted(
            defaultInstance().setAdaptedIntBinaryListMapField("3=aabb:de,9=12:22:31").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertTrue(adapted.getAdaptedIntBinaryListMapField().indexOf("3=aabb:de") >= 0);
    assertTrue(adapted.getAdaptedIntBinaryListMapField().indexOf("9=12:22:31") >= 0);
    assertEquals("aabb", hexDump(received.getIntBinaryListMapField().get(3).get(0)));
    assertEquals("de", hexDump(received.getIntBinaryListMapField().get(3).get(1)));
    assertEquals("31", hexDump(received.getIntBinaryListMapField().get(9).get(2)));
    assertEquals(
        "7=" + hexDump("foo") + ":" + hexDump("bar"), adapted.getAdaptedIntBinaryListMapDefault());
    assertEquals(hexDump("bar"), hexDump(received.getIntBinaryListMapDefault().get(7).get(1)));
  }

  @Test
  public void testOptionalBooleanToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(null, adapted.isOptionalAdaptedBooleanField());
    assertEquals(null, received.isOptionalBooleanField());
  }

  @Test
  public void testOptionalBinaryToStringTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(null, adapted.getOptionalB1());
    assertEquals(null, received.getOptionalB1());
  }

  @Test
  public void testBooleanToStringInlineTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedBooleanField2("true").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);
    assertEquals("true", adapted.isAdaptedBooleanField2());
    assertEquals(true, received.isBooleanField2());
  }

  @Test
  public void testHexListInlineTypeAdapter() {
    byte[] bytes =
        serializeAdapted(
            defaultInstance().setAdaptedBinaryListField2("ab0503:ddee:a1a2a3a4").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("ab0503:ddee:a1a2a3a4", adapted.getAdaptedBinaryListField2());
    assertEquals("ab0503", hexDump(received.getBinaryListField2().get(0)));
    assertEquals("ddee", hexDump(received.getBinaryListField2().get(1)));
    assertEquals("a1a2a3a4", hexDump(received.getBinaryListField2().get(2)));
  }

  private AdaptedTestStructWithoutDefaults createAdaptedTestStructWithoutDefaults() {
    return new AdaptedTestStructWithoutDefaults.Builder()
        .setAdaptedBinaryListField("ab0503:ddee:a1a2a3a4")
        .setAdaptedBinaryListField2("abddee")
        .setAdaptedByteField("9")
        .setAdaptedBooleanField("true")
        .setDateField(new Date())
        .setAdaptedBooleanField2("true")
        .setDoubleAdaptedIntField(2000L)
        .setAdaptedIntMapField("7=12")
        .setAdaptedIntBinaryMapField("3=aabb")
        .setAdaptedIntListField("5,6,7")
        .build();
  }

  @Test
  public void testEquals() {
    AdaptedTestStructWithoutDefaults adapted = createAdaptedTestStructWithoutDefaults();
    byte[] bytes = SerializerUtil.toByteArray(adapted, protocol);
    AdaptedTestStructWithoutDefaults adapted2 =
        SerializerUtil.fromByteArray(AdaptedTestStructWithoutDefaults.asReader(), bytes, protocol);
    assertEquals(adapted, adapted2);

    AdaptedTestStructWithoutDefaults adapted3 =
        new AdaptedTestStructWithoutDefaults.Builder(adapted2).setDoubleAdaptedIntField(5L).build();
    assertNotEquals(adapted, adapted3);

    AdaptedTestStructWithoutDefaults adapted4 =
        new AdaptedTestStructWithoutDefaults.Builder(adapted)
            .setB1(Unpooled.wrappedBuffer("foo".getBytes()))
            .setB1(Unpooled.wrappedBuffer("bar".getBytes()))
            .setB1(Unpooled.wrappedBuffer("baz".getBytes()))
            .build();

    AdaptedTestStructWithoutDefaults adapted5 =
        new AdaptedTestStructWithoutDefaults.Builder(adapted)
            .setB1(Unpooled.wrappedBuffer("foo".getBytes()))
            .setB1(Unpooled.wrappedBuffer("bar".getBytes()))
            .setB1(Unpooled.wrappedBuffer("baz".getBytes()))
            .build();
    assertEquals(adapted4, adapted5);
  }

  @Test
  public void testHashCode() {
    AdaptedTestStructWithoutDefaults adapted = createAdaptedTestStructWithoutDefaults();
    byte[] bytes = SerializerUtil.toByteArray(adapted, protocol);
    AdaptedTestStructWithoutDefaults adapted2 =
        SerializerUtil.fromByteArray(AdaptedTestStructWithoutDefaults.asReader(), bytes, protocol);
    assertEquals(adapted.hashCode(), adapted2.hashCode());

    AdaptedTestStructWithoutDefaults adapted3 =
        new AdaptedTestStructWithoutDefaults.Builder(adapted2).setDoubleAdaptedIntField(5L).build();
    assertNotEquals(adapted.hashCode(), adapted3.hashCode());
  }

  @Test
  public void testDoubleAdaptedIntTypeAdapter() {
    byte[] bytes = serializeAdapted(defaultInstance().setDoubleAdaptedIntField(5000L).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals((Long) 5000L, (Long) adapted.getDoubleAdaptedIntField());
    assertEquals(5000, received.getIntField2());
    assertEquals((Long) 3000L, (Long) adapted.getDoubleAdaptedIntDefault());
    assertEquals(3000, received.getIntDefault2());
  }

  @Test
  public void testDoubleTypeDefIntTypeAdapter() {
    byte[] bytes =
        serializeAdapted(defaultInstance().setDoubleTypedefAdaptedIntField("75").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("75", adapted.getDoubleTypedefAdaptedIntField());
    assertEquals(75, received.getDoubleTypedefIntField());
  }

  @Test
  public void testMultipleTypeDefIntTypeAdapter() {
    byte[] bytes =
        serializeAdapted(defaultInstance().setMultipleTypedefAdaptedIntField("60").build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("60", adapted.getMultipleTypedefAdaptedIntField());
    assertEquals(60, received.getMultipleTypedefIntField());
    assertEquals("50", adapted.getMultipleTypedefAdaptedIntDefault());
    assertEquals(50, received.getMultipleTypedefIntDefault());
  }

  @Test
  public void testGenericTypeAdapter() {
    byte[] bytes =
        serializeAdapted(defaultInstance().setGenericAdapterField(new Wrapped(23.45d)).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(23.45d, adapted.getGenericAdapterField().getValue());
  }

  @Test
  public void testListOfAdaptedInt() {
    List<String> list = Arrays.asList("1", "2", "3");
    byte[] bytes = serializeAdapted(defaultInstance().setListAdaptedIntField(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertArrayEquals(list.toArray(), adapted.getListAdaptedIntField().toArray());
    assertArrayEquals(new Integer[] {1, 2, 3}, received.getListAdaptedIntField().toArray());
    assertArrayEquals(new String[] {"5", "6", "7"}, adapted.getListAdaptedIntDefault().toArray());
  }

  @Test
  public void testListOfAdaptedIntDefault() {
    AdaptedTestStruct adapted = deserializeAdapted(serializeAdapted(defaultInstance().build()));
    assertArrayEquals(new String[] {"5", "6", "7"}, adapted.getListAdaptedIntDefault().toArray());
  }

  @Test
  public void testListOfListOfAdaptedInt() {
    List<List<String>> list = Arrays.asList(Arrays.asList("1", "2", "3"));
    byte[] bytes = serializeAdapted(defaultInstance().setListOfListAdaptedIntField(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertArrayEquals(
        list.get(0).toArray(), adapted.getListOfListAdaptedIntField().get(0).toArray());
    assertArrayEquals(
        new Integer[] {1, 2, 3}, received.getListOfListAdaptedIntField().get(0).toArray());
  }

  @Test
  public void testListOfListOfAdaptedIntDefault() {
    AdaptedTestStruct adapted = deserializeAdapted(serializeAdapted(defaultInstance().build()));
    assertArrayEquals(
        new String[] {"3", "4", "5"}, adapted.getListOfListAdaptedIntDefault().get(1).toArray());
  }

  @Test
  public void testNestedListOfAdaptedInt() {
    List<List<List<String>>> list = Arrays.asList(Arrays.asList(Arrays.asList("1", "2", "3")));
    byte[] bytes = serializeAdapted(defaultInstance().setNestedListAdaptedIntField(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertArrayEquals(
        list.get(0).get(0).toArray(),
        adapted.getNestedListAdaptedIntField().get(0).get(0).toArray());
    assertArrayEquals(
        new Integer[] {1, 2, 3}, received.getNestedListAdaptedIntField().get(0).get(0).toArray());
  }

  @Test
  public void testNestedListOfAdaptedIntDefault() {
    AdaptedTestStruct adapted = deserializeAdapted(serializeAdapted(defaultInstance().build()));
    assertArrayEquals(
        new String[] {"3", "4", "5"},
        adapted.getNestedListAdaptedIntDefault().get(0).get(1).toArray());
  }

  private Set<String> createSet(String... item) {
    Set<String> set = new HashSet<>();
    for (String s : item) {
      set.add(s);
    }
    return set;
  }

  @Test
  public void testSetOfAdaptedInt() {
    Set<String> set = createSet("1", "2", "3");
    byte[] bytes = serializeAdapted(defaultInstance().setSetAdaptedIntField(set).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(3, adapted.getSetAdaptedIntField().size());
    assertEquals(3, received.getSetAdaptedIntField().size());
  }

  @Test
  public void testSetOfAdaptedIntDefault() {
    AdaptedTestStruct adapted = deserializeAdapted(serializeAdapted(defaultInstance().build()));
    assertTrue(adapted.getSetAdaptedIntDefault().contains("7"));
  }

  @Test
  public void testMapOfAdaptedInt() {
    Map<String, String> map = new HashMap<>();
    map.put("5", "13");

    byte[] bytes = serializeAdapted(defaultInstance().setMapOfIntToShortField(map).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("13", adapted.getMapOfIntToShortField().get("5"));
    assertEquals(13L, (long) received.getMapOfIntToShortField().get(5));
  }

  @Test
  public void testMapOfAdaptedIntDefault() {
    AdaptedTestStruct adapted = deserializeAdapted(serializeAdapted(defaultInstance().build()));
    assertEquals("9", adapted.getMapOfIntToShortDefault().get("8"));
  }

  @Test
  public void testMapOfMapAdaptedInt() {
    Map<String, Map<String, String>> map = new HashMap<>();
    Map<String, String> map1 = new HashMap<>();
    map1.put("5", "13");
    map.put("7", map1);

    byte[] bytes = serializeAdapted(defaultInstance().setMapOfIntToMapIntToShortField(map).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("13", adapted.getMapOfIntToMapIntToShortField().get("7").get("5"));
    assertEquals(13L, (long) received.getMapOfIntToMapIntToShortField().get(7).get(5));
  }

  @Test
  public void testMapOfMapAdaptedIntDefault() {
    AdaptedTestStruct adapted = deserializeAdapted(serializeAdapted(defaultInstance().build()));
    assertEquals("9", adapted.getMapOfIntToMapIntToShortDefault().get("7").get("8"));
  }

  @Test
  public void testLisfOfSetAdaptedInt() {
    List<Set<String>> list = Arrays.asList(createSet("1", "2", "3"));
    byte[] bytes = serializeAdapted(defaultInstance().setListOfSetAdaptedIntField(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(1, adapted.getListOfSetAdaptedIntField().size());
    assertEquals(3, adapted.getListOfSetAdaptedIntField().get(0).size());
    assertEquals(3, received.getListOfSetAdaptedIntField().get(0).size());
  }

  @Test
  public void testNestedMapOfAdaptedTypes() {
    Map<String, Map<String, List<String>>> map = new HashMap<>();
    Map<String, List<String>> map1 = new HashMap<>();
    map1.put("3", Arrays.asList("8"));
    map.put("7", map1);

    byte[] bytes = serializeAdapted(defaultInstance().setNestedAdaptedField(map).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("8", adapted.getNestedAdaptedField().get("7").get("3").get(0));
    assertEquals((short) 8, (short) received.getNestedAdaptedField().get(7).get((short) 3).get(0));
  }

  @Test
  public void testNestedMapOfAdaptedTypesDefault() {
    AdaptedTestStruct adapted = deserializeAdapted(serializeAdapted(defaultInstance().build()));
    assertEquals("10", adapted.getNestedMapIntToShortDefault().get("7").get("8").get("9"));
  }

  @Test
  public void testNestedAdaptedTypesDefault() {
    AdaptedTestStruct adapted = deserializeAdapted(serializeAdapted(defaultInstance().build()));
    assertArrayEquals(
        new String[] {"1", "2"}, adapted.getNestedAdaptedDefault().get("7").get("3").toArray());
  }

  @Test
  public void testListOfAdaptedBool() {
    List<String> list = Arrays.asList("true", "false");
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedBooleanListField(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertArrayEquals(list.toArray(), adapted.getAdaptedBooleanListField().toArray());
    assertArrayEquals(new Boolean[] {true, false}, received.getAdaptedBooleanListField().toArray());
  }

  @Test
  public void testListOfAdaptedByte() {
    List<String> list = Arrays.asList("10", "11");
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedByteListField(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertArrayEquals(list.toArray(), adapted.getAdaptedByteListField().toArray());
    assertArrayEquals(new Byte[] {10, 11}, received.getAdaptedByteListField().toArray());
  }

  @Test
  public void testListOfAdaptedShort() {
    List<String> list = Arrays.asList("10", "11");
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedShortListField(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertArrayEquals(list.toArray(), adapted.getAdaptedShortListField().toArray());
    assertArrayEquals(new Short[] {10, 11}, received.getAdaptedShortListField().toArray());
  }

  @Test
  public void testListOfAdaptedLong() {
    List<String> list = Arrays.asList("10", "11");
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedLongListField(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertArrayEquals(list.toArray(), adapted.getAdaptedLongListField().toArray());
    assertArrayEquals(new Long[] {10L, 11L}, received.getAdaptedLongListField().toArray());
  }

  @Test
  public void testListOfAdaptedFloat() {
    List<String> list = Arrays.asList("10", "11");
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedFloatListField(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertArrayEquals(new String[] {"10.0", "11.0"}, adapted.getAdaptedFloatListField().toArray());
    assertArrayEquals(new Float[] {10f, 11f}, received.getAdaptedFloatListField().toArray());
  }

  @Test
  public void testListOfAdaptedDouble() {
    List<String> list = Arrays.asList("10", "11");
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedDoubleListField(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertArrayEquals(new String[] {"10.0", "11.0"}, adapted.getAdaptedDoubleListField().toArray());
    assertArrayEquals(new Double[] {10d, 11d}, received.getAdaptedDoubleListField().toArray());
  }

  @Test
  public void testListOfAdaptedString() {
    List<ByteBuf> list =
        Arrays.asList(
            Unpooled.wrappedBuffer("foo".getBytes()), Unpooled.wrappedBuffer("bar".getBytes()));
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedStringListField(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertArrayEquals(list.toArray(), adapted.getAdaptedStringListField().toArray());
    assertArrayEquals(new String[] {"foo", "bar"}, received.getAdaptedStringListField().toArray());
  }

  @Test
  public void testListOfAdaptedBinary() {
    byte[] foo = "foo".getBytes();
    byte[] bar = "bar".getBytes();

    List<ByteBuf> list = Arrays.asList(Unpooled.wrappedBuffer(foo), Unpooled.wrappedBuffer(bar));
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedBinListField(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(hexDump(foo), hexDump(adapted.getAdaptedBinListField().get(0)));
    assertEquals(hexDump(bar), hexDump(adapted.getAdaptedBinListField().get(1)));
    assertArrayEquals(foo, received.getAdaptedBinListField().get(0));
    assertArrayEquals(bar, received.getAdaptedBinListField().get(1));
  }

  @Test
  public void testListOfListOfAdaptedBinary() {
    byte[] foo = "foo".getBytes();
    byte[] bar = "bar".getBytes();

    List<List<ByteBuf>> list =
        Arrays.asList(Arrays.asList(Unpooled.wrappedBuffer(foo), Unpooled.wrappedBuffer(bar)));
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedBinList2Field(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(hexDump(foo), hexDump(adapted.getAdaptedBinList2Field().get(0).get(0)));
    assertEquals(hexDump(bar), hexDump(adapted.getAdaptedBinList2Field().get(0).get(1)));
    assertArrayEquals(foo, received.getAdaptedBinList2Field().get(0).get(0));
    assertArrayEquals(bar, received.getAdaptedBinList2Field().get(0).get(1));
  }

  @Test
  public void testNestedListOfAdaptedBinary() {
    byte[] foo = "foo".getBytes();
    byte[] bar = "bar".getBytes();

    List<List<List<ByteBuf>>> list =
        Arrays.asList(
            Arrays.asList(Arrays.asList(Unpooled.wrappedBuffer(foo), Unpooled.wrappedBuffer(bar))));
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedBinList3Field(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(hexDump(foo), hexDump(adapted.getAdaptedBinList3Field().get(0).get(0).get(0)));
    assertEquals(hexDump(bar), hexDump(adapted.getAdaptedBinList3Field().get(0).get(0).get(1)));
    assertArrayEquals(foo, received.getAdaptedBinList3Field().get(0).get(0).get(0));
    assertArrayEquals(bar, received.getAdaptedBinList3Field().get(0).get(0).get(1));
  }

  @Test
  public void testMapOfAdaptedBinary() {
    byte[] foo = "foo".getBytes();

    Map<Integer, ByteBuf> map = new HashMap<>();
    map.put(5, Unpooled.wrappedBuffer(foo));
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedBinMapField(map).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(hexDump(foo), hexDump(adapted.getAdaptedBinMapField().get(5)));
    assertArrayEquals(foo, received.getAdaptedBinMapField().get(5));
  }

  @Test
  public void testMapOfMapOfAdaptedBinary() {
    byte[] foo = "foo".getBytes();

    Map<Integer, ByteBuf> map1 = new HashMap<>();
    map1.put(5, Unpooled.wrappedBuffer(foo));
    Map<Integer, Map<Integer, ByteBuf>> map = new HashMap<>();
    map.put(7, map1);

    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedBinMap2Field(map).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(hexDump(foo), hexDump(adapted.getAdaptedBinMap2Field().get(7).get(5)));
    assertArrayEquals(foo, received.getAdaptedBinMap2Field().get(7).get(5));
  }

  @Test
  public void testListOfListOfAdaptedBinaryString() {
    byte[] foo = "foo".getBytes();
    byte[] bar = "bar".getBytes();

    List<List<ByteBuf>> list =
        Arrays.asList(Arrays.asList(Unpooled.wrappedBuffer(foo), Unpooled.wrappedBuffer(bar)));
    byte[] bytes = serializeAdapted(defaultInstance().setAdaptedBinStringList2Field(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(hexDump(foo), hexDump(adapted.getAdaptedBinStringList2Field().get(0).get(0)));
    assertEquals(hexDump(bar), hexDump(adapted.getAdaptedBinStringList2Field().get(0).get(1)));
    assertArrayEquals(foo, received.getAdaptedBinStringList2Field().get(0).get(0));
    assertArrayEquals(bar, received.getAdaptedBinStringList2Field().get(0).get(1));
  }

  @Test
  public void testListOfMultiLevelTypeDef() {
    byte[] foo = "foo".getBytes();
    byte[] bar = "bar".getBytes();

    List<ByteBuf> list = Arrays.asList(Unpooled.wrappedBuffer(foo), Unpooled.wrappedBuffer(bar));
    byte[] bytes =
        serializeAdapted(defaultInstance().setAdaptedBinNestedTypeDefListField(list).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(hexDump(foo), hexDump(adapted.getAdaptedBinNestedTypeDefListField().get(0)));
    assertEquals(hexDump(bar), hexDump(adapted.getAdaptedBinNestedTypeDefListField().get(1)));
    assertArrayEquals(foo, received.getAdaptedBinNestedTypeDefListField().get(0));
    assertArrayEquals(bar, received.getAdaptedBinNestedTypeDefListField().get(1));
  }

  @Test
  public void testAdaptedSetList() {
    byte[] bytes =
        serializeAdapted(defaultInstance().setAdaptedSetListField(Arrays.asList("7")).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertTrue(adapted.getAdaptedSetListField().get(0).contains("7"));
    assertTrue(received.getAdaptedSetListField().get(0).contains(7));
  }

  @Test
  public void testDoubleAdaptedList() {
    byte[] bytes =
        serializeAdapted(
            defaultInstance().setDoubleAdaptedListField(new Wrapped<>(Arrays.asList("7"))).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertTrue(adapted.getDoubleAdaptedListField().getValue().get(0).contains("7"));
    assertEquals(7L, (long) received.getDoubleAdaptedListField().get(0));
  }

  @Test
  public void testStructList() {
    byte[] bytes =
        serializeAdapted(
            defaultInstance()
                .setAnyListField(Arrays.asList(new Any.Builder("foo").build()))
                .build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("foo", adapted.getAnyListField().get(0).get());
    assertEquals("foo", received.getAnyListField().get(0).get());
  }

  @Test
  public void testStructMap() {
    Map<Integer, List<Any>> map = new HashMap<>();
    map.put(1, Arrays.asList(new Any.Builder("foo").build()));
    byte[] bytes = serializeAdapted(defaultInstance().setAnyMapField(map).build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals("foo", adapted.getAnyMapField().get(1).get(0).get());
    assertEquals("foo", received.getAnyMapField().get(1).get(0).get());
  }

  @Test
  public void testUnionList() {
    byte[] bytes =
        serializeAdapted(
            defaultInstance()
                .setUnionListField(Arrays.asList(new Wrapped(InnerUnion.fromIntField(5))))
                .build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(5, adapted.getUnionListField().get(0).getValue().getIntField());
    assertEquals(5, received.getUnionListField().get(0).getIntField());
  }

  @Test
  public void testExceptionList() {
    byte[] bytes =
        serializeAdapted(
            defaultInstance()
                .setExceptionListField(
                    Arrays.asList(new Wrapped(new InnerException.Builder().setIntField(5).build())))
                .build());
    AdaptedTestStruct adapted = deserializeAdapted(bytes);
    TestStruct received = deserialize(bytes);

    assertEquals(5, adapted.getExceptionListField().get(0).getValue().getIntField());
    assertEquals(5, received.getExceptionListField().get(0).getIntField());
  }

  @Test
  public void testEqualsAndHashCodeAdaptedInt() {
    List<String> list = Arrays.asList("1", "2", "3");
    byte[] bytes = serializeAdapted(defaultInstance().setListAdaptedIntField(list).build());
    AdaptedTestStruct adapted1 = deserializeAdapted(bytes);
    AdaptedTestStruct adapted2 = deserializeAdapted(bytes);

    assertEquals(adapted1.hashCode(), adapted2.hashCode());
    assertEquals(adapted1, adapted2);
  }
}
