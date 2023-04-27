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

package com.facebook.thrift.compatibility;

import static org.junit.Assert.*;
import static org.junit.Assert.assertArrayEquals;

import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.test.utf8.TestException;
import com.facebook.thrift.test.utf8.TestExceptionCompat;
import com.facebook.thrift.test.utf8.TestStruct;
import com.facebook.thrift.test.utf8.TestStructCompat;
import com.facebook.thrift.test.utf8.TestStructString;
import com.facebook.thrift.test.utf8.TestUnion;
import com.facebook.thrift.test.utf8.TestUnionCompat;
import com.facebook.thrift.test.utf8.Utf8TestStructBin;
import com.facebook.thrift.test.utf8.Utf8TestStructLegacy;
import com.facebook.thrift.test.utf8.Utf8TestStructReport;
import com.facebook.thrift.test.utf8.pkg.TestExceptionPkg;
import com.facebook.thrift.test.utf8.pkg.TestStructPkg;
import com.facebook.thrift.test.utf8.pkg.TestStructReportPkg;
import com.facebook.thrift.test.utf8.pkg.TestUnionPkg;
import com.facebook.thrift.util.IntrinsicDefaults;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import org.apache.thrift.protocol.TProtocolException;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class StringCompatibilityTest {

  @Rule public ExpectedException expectedException = ExpectedException.none();
  public static byte[] malformedUtfBytes = new byte[] {50, -1, -1, 52, 53};
  public static byte[] replacedUtfBytes =
      new byte[] {
        50, (byte) 0xef, (byte) 0xbf, (byte) 0xbd, (byte) 0xef, (byte) 0xbf, (byte) 0xbd, 52, 53
      };
  public static String replacedUtfString = new String(replacedUtfBytes);
  public static byte[] utfBytes = new byte[] {50, 52, 53};

  private static class Param {
    protected SerializationProtocol protocol;
    protected boolean offHeap;

    public Param(SerializationProtocol protocol, boolean offHeap) {
      this.protocol = protocol;
      this.offHeap = offHeap;
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
  private final boolean offHeap;

  public StringCompatibilityTest(Param param) {
    this.protocol = param.protocol;
    this.offHeap = param.offHeap;
  }

  private byte[] serialize(TestStruct struct) {
    return SerializerUtil.toByteArray(struct, protocol);
  }

  private <T> T fromByteArray(Reader<T> reader, byte[] src, SerializationProtocol protocol) {
    if (offHeap) {
      ByteBuf buf = Unpooled.directBuffer(src.length);
      buf.writeBytes(src);
      ByteBufTProtocol byteBufTProtocol = SerializerUtil.toByteBufProtocol(protocol, buf);
      return reader.read(byteBufTProtocol);
    }
    ByteBufTProtocol byteBufTProtocol =
        SerializerUtil.toByteBufProtocol(protocol, Unpooled.wrappedBuffer(src));
    return reader.read(byteBufTProtocol);
  }

  private TestStructString deserializeString(byte[] bytes) {
    return fromByteArray(TestStructString.asReader(), bytes, protocol);
  }

  private Utf8TestStructLegacy deserializeLegacy(byte[] bytes) {
    return fromByteArray(Utf8TestStructLegacy.asReader(), bytes, protocol);
  }

  private Utf8TestStructReport deserializeReport(byte[] bytes) {
    return fromByteArray(Utf8TestStructReport.asReader(), bytes, protocol);
  }

  private Utf8TestStructBin deserializeBin(byte[] bytes) {
    return fromByteArray(Utf8TestStructBin.asReader(), bytes, protocol);
  }

  private TestStructCompat deserializeCompat(byte[] bytes) {
    return fromByteArray(TestStructCompat.asReader(), bytes, protocol);
  }

  private Set<byte[]> createSet(byte[] bytes) {
    Set<byte[]> set = new HashSet<>();
    set.add(bytes);
    return set;
  }

  private Map<Integer, byte[]> createMap(int key, byte[] bytes) {
    Map<Integer, byte[]> map = new HashMap<>();
    map.put(key, bytes);
    return map;
  }

  private Map<byte[], Map<byte[], byte[]>> createMap(byte[] key, byte[] key1, byte[] value1) {
    Map<byte[], Map<byte[], byte[]>> map = new HashMap<>();
    Map<byte[], byte[]> map1 = new HashMap<>();
    map1.put(key1, value1);
    map.put(key, map1);
    return map;
  }

  private void expectMalformedUtfException() {
    expectedException.expect(TProtocolException.class);
    expectedException.expectMessage("Malformed UTF8 string: 32ffff3435");
  }

  private TestStruct testStruct() {
    return new TestStruct.Builder()
        .setStrField(malformedUtfBytes)
        .setStrList(Arrays.asList(malformedUtfBytes))
        .setStrSet(createSet(malformedUtfBytes))
        .setStrMap(createMap(5, malformedUtfBytes))
        .setStrListOfList(Arrays.asList(Arrays.asList(malformedUtfBytes)))
        .setAdaptedStr(utfBytes)
        .setComplexField(createMap(malformedUtfBytes, malformedUtfBytes, malformedUtfBytes))
        .setUtf8Str(malformedUtfBytes)
        .setAdaptedUtf8Str(utfBytes)
        .build();
  }

  private TestStruct testStructValidUtf8() {
    return new TestStruct.Builder()
        .setStrField("±foo±".getBytes())
        .setStrList(Arrays.asList("bar".getBytes()))
        .setStrSet(createSet("baz".getBytes()))
        .setStrMap(createMap(5, "val".getBytes()))
        .setStrListOfList(Arrays.asList(Arrays.asList("bar".getBytes())))
        .setAdaptedStr(null)
        .setComplexField(createMap("test".getBytes(), "key".getBytes(), "value".getBytes()))
        .setUtf8Str("test2".getBytes())
        .setAdaptedUtf8Str("123".getBytes())
        .build();
  }

  @Test
  public void testReadAsString() {
    TestStructString received = deserializeString(serialize(testStructValidUtf8()));
    assertEquals("±foo±", received.getStrField());
    assertEquals("bar", received.getStrList().get(0));
    assertEquals("baz", received.getStrSet().iterator().next());
    assertEquals("val", received.getStrMap().get(5));
    assertEquals("bar", received.getStrListOfList().get(0).get(0));
    assertEquals(null, received.getAdaptedStr());
    assertEquals(IntrinsicDefaults.defaultString(), received.getTerseStr());
    assertEquals("value", received.getComplexField().get("test").get("key"));
    assertEquals("test2", received.getUtf8Str());
    assertEquals((Long) 123L, received.getAdaptedUtf8Str());
  }

  @Test
  public void testUtf8Legacy() {
    Utf8TestStructLegacy received = deserializeLegacy(serialize(testStruct()));
    assertArrayEquals(replacedUtfBytes, received.getStrField().getBytes());
    assertArrayEquals(replacedUtfBytes, received.getStrList().get(0).getBytes());
    assertArrayEquals(replacedUtfBytes, received.getStrSet().iterator().next().getBytes());
    assertArrayEquals(replacedUtfBytes, received.getStrMap().get(5).getBytes());
    assertArrayEquals(replacedUtfBytes, received.getStrListOfList().get(0).get(0).getBytes());
    assertEquals((Long) 245L, received.getAdaptedStr());
    assertEquals(IntrinsicDefaults.defaultString(), received.getTerseStr());
    assertEquals(
        replacedUtfString,
        received.getComplexField().get(replacedUtfString).get(replacedUtfString));
    // typedef has precedence
    assertEquals(replacedUtfString, received.getUtf8Str());
    assertEquals((Long) 245L, received.getAdaptedUtf8Str());
  }

  private void assertErrorRaised(TestStruct st) {
    try {
      deserializeReport(serialize(st));
      fail();
    } catch (TProtocolException e) {
      assertEquals("Malformed UTF8 string: 32ffff3435", e.getMessage());
    }
  }

  @Test
  public void testReadMalformedUtfReport() {
    TestStruct st = testStructValidUtf8();
    assertErrorRaised(new TestStruct.Builder(st).setStrField(malformedUtfBytes).build());
    assertErrorRaised(
        new TestStruct.Builder(st).setStrList(Arrays.asList(malformedUtfBytes)).build());
    assertErrorRaised(new TestStruct.Builder(st).setStrSet(createSet(malformedUtfBytes)).build());
    assertErrorRaised(
        new TestStruct.Builder(st).setStrMap(createMap(5, malformedUtfBytes)).build());
    assertErrorRaised(
        new TestStruct.Builder(st)
            .setStrListOfList(Arrays.asList(Arrays.asList(malformedUtfBytes)))
            .build());
    assertErrorRaised(new TestStruct.Builder(st).setAdaptedStr(malformedUtfBytes).build());
    assertErrorRaised(new TestStruct.Builder(st).setTerseStr(malformedUtfBytes).build());
    assertErrorRaised(
        new TestStruct.Builder(st)
            .setComplexField(createMap(malformedUtfBytes, malformedUtfBytes, malformedUtfBytes))
            .build());
    assertErrorRaised(new TestStruct.Builder(st).setUtf8Str(malformedUtfBytes).build());
    assertErrorRaised(new TestStruct.Builder(st).setAdaptedUtf8Str(malformedUtfBytes).build());
  }

  @Test
  public void testUnion() {
    expectMalformedUtfException();
    TestUnion u = TestUnion.fromField1(malformedUtfBytes);

    fromByteArray(TestUnionCompat.asReader(), SerializerUtil.toByteArray(u, protocol), protocol);
  }

  @Test
  public void testUnionTypedef() {
    expectMalformedUtfException();
    TestUnion u = TestUnion.fromUtf8Str(malformedUtfBytes);

    fromByteArray(TestUnionCompat.asReader(), SerializerUtil.toByteArray(u, protocol), protocol);
  }

  @Test
  public void testBinaryString() {
    Utf8TestStructBin received = deserializeBin(serialize(testStruct()));
    assertArrayEquals(malformedUtfBytes, received.getStrField());
    assertArrayEquals(malformedUtfBytes, received.getStrList().get(0));
    assertArrayEquals(malformedUtfBytes, received.getStrSet().iterator().next());
    assertArrayEquals(malformedUtfBytes, received.getStrMap().get(5));
    assertArrayEquals(malformedUtfBytes, received.getStrListOfList().get(0).get(0));
    assertArrayEquals(utfBytes, received.getAdaptedStr().array());
    assertArrayEquals(IntrinsicDefaults.defaultString().getBytes(), received.getTerseStr());
    byte[] key = received.getComplexField().keySet().iterator().next();
    assertArrayEquals(malformedUtfBytes, key);
    byte[] key1 = received.getComplexField().get(key).keySet().iterator().next();
    assertArrayEquals(malformedUtfBytes, key1);
    byte[] value1 = received.getComplexField().get(key).get(key1);
    assertArrayEquals(malformedUtfBytes, value1);
  }

  @Test
  public void testException() {
    expectMalformedUtfException();
    TestException e =
        new TestException.Builder().setErrCode((byte) 5).setErrMsg(malformedUtfBytes).build();

    SerializerUtil.toByteArray(e, protocol);
    fromByteArray(
        TestExceptionCompat.asReader(), SerializerUtil.toByteArray(e, protocol), protocol);
  }

  @Test
  public void testDefaultStringAnnotation() {
    expectMalformedUtfException();
    TestStruct st = new TestStruct.Builder().setStrField(malformedUtfBytes).build();
    deserializeCompat(serialize(st));
  }

  @Test
  public void testPackageLevelAnnotation() {
    expectMalformedUtfException();
    TestStruct e = new TestStruct.Builder().setStrField(malformedUtfBytes).build();

    fromByteArray(
        TestStructReportPkg.asReader(), SerializerUtil.toByteArray(e, protocol), protocol);
  }

  @Test
  public void testPackageLevelAnnotationOverride() {
    TestStruct e = new TestStruct.Builder().setStrField(malformedUtfBytes).build();

    TestStructPkg received =
        fromByteArray(TestStructPkg.asReader(), SerializerUtil.toByteArray(e, protocol), protocol);
    assertEquals(replacedUtfString, received.getStrField());
  }

  @Test
  public void testPackageLevelAnnotationOverrideUnion() {
    TestUnion u = TestUnion.fromField1(malformedUtfBytes);

    TestUnionPkg received =
        fromByteArray(TestUnionPkg.asReader(), SerializerUtil.toByteArray(u, protocol), protocol);
    assertEquals(replacedUtfString, received.getField1());
  }

  @Test
  public void testPackageLevelAnnotationOverrideException() {
    TestException u = new TestException.Builder().setErrMsg(malformedUtfBytes).build();

    TestExceptionPkg e =
        fromByteArray(
            TestExceptionPkg.asReader(), SerializerUtil.toByteArray(u, protocol), protocol);
    assertEquals(replacedUtfString, e.getErrMsg());
  }

  @Test
  public void testTypedefPrecedence() {
    expectMalformedUtfException();
    TestStruct st = new TestStruct.Builder().setUtf8Str(malformedUtfBytes).build();
    deserializeCompat(serialize(st));
  }

  @Test
  public void testNestedElementType() {
    expectMalformedUtfException();
    TestStruct st =
        new TestStruct.Builder()
            .setComplexField(createMap(malformedUtfBytes, malformedUtfBytes, malformedUtfBytes))
            .build();
    // UTF-8 settings on nested element types are not supported.
    deserializeCompat(serialize(st));
  }
}
