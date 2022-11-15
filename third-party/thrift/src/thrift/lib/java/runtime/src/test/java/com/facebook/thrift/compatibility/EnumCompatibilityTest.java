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

import com.facebook.thrift.payload.ThriftSerializable;
import com.facebook.thrift.test.enums.TestEnumLegacy1;
import com.facebook.thrift.test.enums.TestEnumLegacy2;
import com.facebook.thrift.test.enums.TestEnumOpen1;
import com.facebook.thrift.test.enums.TestEnumOpen2;
import com.facebook.thrift.test.enums.TestExceptionOpen;
import com.facebook.thrift.test.enums.TestStruct;
import com.facebook.thrift.test.enums.TestStructAdapted;
import com.facebook.thrift.test.enums.TestStructAdaptedOptional;
import com.facebook.thrift.test.enums.TestStructComplex;
import com.facebook.thrift.test.enums.TestStructLegacy;
import com.facebook.thrift.test.enums.TestStructLegacyTerse;
import com.facebook.thrift.test.enums.TestStructOpen;
import com.facebook.thrift.test.enums.TestStructOpenTerse;
import com.facebook.thrift.test.enums.TestUnionOpen;
import com.facebook.thrift.test.enums.pkg.TestEnumOpenPkg;
import com.facebook.thrift.test.enums.pkg.TestEnumOverridePkg;
import com.facebook.thrift.test.enums.pkg.TestStructOpenPkg;
import com.facebook.thrift.util.EnumUtil;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.junit.Test;

public class EnumCompatibilityTest {

  private TestStruct createTestStruct(Integer field1, Integer field2) {
    return new TestStruct.Builder().setIntField1(field1).setIntField2(field2).build();
  }

  private byte[] serialize(TestStruct struct) {
    return SerializerUtil.toByteArray(struct, SerializationProtocol.TCompact);
  }

  private TestStruct deserialize(ThriftSerializable struct) {
    byte[] bytes = SerializerUtil.toByteArray(struct, SerializationProtocol.TCompact);
    return SerializerUtil.fromByteArray(
        TestStruct.asReader(), bytes, SerializationProtocol.TCompact);
  }

  private TestStructLegacy deserializeLegacy(Integer field1, Integer field2) {
    TestStruct st = createTestStruct(field1, field2);
    byte[] bytes = serialize(st);
    return SerializerUtil.fromByteArray(
        TestStructLegacy.asReader(), bytes, SerializationProtocol.TCompact);
  }

  private TestStructLegacyTerse deserializeLegacyTerse(Integer field1, Integer field2) {
    TestStruct st = createTestStruct(field1, field2);
    byte[] bytes = serialize(st);
    return SerializerUtil.fromByteArray(
        TestStructLegacyTerse.asReader(), bytes, SerializationProtocol.TCompact);
  }

  private TestStructOpen deserializeOpen(Integer field1, Integer field2) {
    TestStruct st = createTestStruct(field1, field2);
    byte[] bytes = serialize(st);
    return SerializerUtil.fromByteArray(
        TestStructOpen.asReader(), bytes, SerializationProtocol.TCompact);
  }

  private TestStructOpenTerse deserializeOpenTerse(Integer field1, Integer field2) {
    TestStruct st = createTestStruct(field1, field2);
    byte[] bytes = serialize(st);
    return SerializerUtil.fromByteArray(
        TestStructOpenTerse.asReader(), bytes, SerializationProtocol.TCompact);
  }

  private TestStructAdapted deserializeAdapted(Integer field1, Integer field2) {
    TestStruct st = createTestStruct(field1, field2);
    byte[] bytes = serialize(st);
    return SerializerUtil.fromByteArray(
        TestStructAdapted.asReader(), bytes, SerializationProtocol.TCompact);
  }

  private TestStructAdaptedOptional deserializeAdaptedOptional(Integer field1, Integer field2) {
    TestStruct st = createTestStruct(field1, field2);
    byte[] bytes = serialize(st);
    return SerializerUtil.fromByteArray(
        TestStructAdaptedOptional.asReader(), bytes, SerializationProtocol.TCompact);
  }

  private TestUnionOpen deserializeUnion(Integer field1, Integer field2) {
    TestStruct st = createTestStruct(field1, field2);
    byte[] bytes = serialize(st);
    return SerializerUtil.fromByteArray(
        TestUnionOpen.asReader(), bytes, SerializationProtocol.TCompact);
  }

  private TestExceptionOpen deserializeException(Integer field1, Integer field2) {
    TestStruct st = createTestStruct(field1, field2);
    byte[] bytes = serialize(st);
    return SerializerUtil.fromByteArray(
        TestExceptionOpen.asReader(), bytes, SerializationProtocol.TCompact);
  }

  private void assertTestStruct(ThriftSerializable struct, Integer field1, Integer field2) {
    TestStruct st = deserialize(struct);
    if (field1 == null) {
      assertNull(st.getIntField1());
    } else {
      assertEquals((long) field1, (long) st.getIntField1());
    }
    if (field2 == null) {
      assertNull(st.getIntField2());
    } else {
      assertEquals((long) field2, (long) st.getIntField2());
    }
  }

  @Test
  public void testLegacyEnumDefinedValue() {
    TestStructLegacy received = deserializeLegacy(1, 3);
    assertEquals(TestEnumLegacy1.ONE, received.getEnumField1());
    assertEquals(TestEnumLegacy2.THREE, received.getEnumField2());
    assertEquals(1, received.getEnumField1().getValue());
    assertEquals(3, received.getEnumField2().getValue());
    assertEquals(1, received.getEnumField1().ordinal());
    assertEquals(2, received.getEnumField2().ordinal());

    assertTestStruct(received, 1, 3);
  }

  @Test
  public void testLegacyEnumUnrecognizedValue() {
    TestStructLegacy received = deserializeLegacy(0, 200);
    assertEquals(null, received.getEnumField2());

    assertTestStruct(received, 0, 0);
  }

  @Test
  public void testLegacyEnumDefaultValue() {
    TestStructLegacy received = deserializeLegacy(null, null);
    assertEquals(TestEnumLegacy1.ZERO, received.getEnumField1());
    assertEquals(null, received.getEnumField2());

    assertTestStruct(received, 0, 0);
  }

  @Test
  public void testLegacyEnumTerseValue() {
    TestStructLegacyTerse received = deserializeLegacyTerse(null, null);
    assertEquals(TestEnumLegacy1.ZERO, received.getEnumField1());
    assertEquals(null, received.getEnumField2());
  }

  @Test
  public void testOpenEnumDefinedValue() {
    TestStructOpen received = deserializeOpen(1, 3);
    assertEquals(TestEnumOpen1.ONE, received.getEnumField1());
    assertEquals(TestEnumOpen2.THREE, received.getEnumField2());
    assertEquals(1, (long) received.getEnumField1().getValue());
    assertEquals(3, (long) received.getEnumField2().getValue());
    assertEquals(1, received.getEnumField1().ordinal());
    assertEquals(2, received.getEnumField2().ordinal());
    assertEquals("ONE", received.getEnumField1().name());
    assertEquals("THREE", received.getEnumField2().name());
    assertEquals(0, received.getEnumField1().getUnrecognizedValue());
    assertEquals(0, received.getEnumField2().getUnrecognizedValue());

    assertTestStruct(received, 1, 3);
  }

  @Test
  public void testOpenEnumUnrecognizedValue() {
    TestStructOpen received = deserializeOpen(200, 200);
    assertEquals(200, received.getEnumField1().getUnrecognizedValue());
    assertEquals(200, received.getEnumField2().getUnrecognizedValue());
    assertEquals(TestEnumOpen1.UNRECOGNIZED_VALUE, (long) received.getEnumField1().getValue());
    assertEquals(TestEnumOpen2.UNRECOGNIZED_VALUE, (long) received.getEnumField2().getValue());
    assertEquals(4, received.getEnumField1().ordinal());
    assertEquals(3, received.getEnumField2().ordinal());
    assertEquals("UNRECOGNIZED", received.getEnumField1().name());
    assertEquals("UNRECOGNIZED", received.getEnumField2().name());

    assertTestStruct(received, 200, 200);
  }

  @Test
  public void testOpenEnumDefaultValue() {
    TestStructOpen received = deserializeOpen(null, null);
    assertEquals(TestEnumOpen1.ZERO, received.getEnumField1());
    assertEquals(TestEnumOpen2.UNRECOGNIZED_VALUE, (long) received.getEnumField2().getValue());
    assertEquals(0, (long) received.getEnumField2().getUnrecognizedValue());

    assertTestStruct(received, 0, 0);
  }

  @Test
  public void testOpenEnumTerseValue() {
    TestStructOpenTerse received = deserializeOpenTerse(null, null);
    assertEquals(TestEnumOpen1.ZERO, received.getEnumField1());
    assertEquals(TestEnumOpen2.UNRECOGNIZED_VALUE, (long) received.getEnumField2().getValue());
    assertEquals(0, (long) received.getEnumField2().getUnrecognizedValue());

    assertTestStruct(received, null, null);
  }

  @Test
  public void testAdaptedEnumDefinedValue() {
    TestStructAdapted received = deserializeAdapted(1, 2);
    assertEquals("1", received.getEnumField1());
    assertEquals("2", received.getEnumField2());

    assertTestStruct(received, 1, 2);
  }

  @Test
  public void testAdaptedEnumUnrecognizedValue() {
    TestStructAdapted received = deserializeAdapted(100, 200);
    assertEquals(null, received.getEnumField1());
    assertEquals("200", received.getEnumField2());

    assertTestStruct(received, null, 200);
  }

  @Test
  public void testAdaptedEnumDefaultValue() {
    TestStructAdapted received = deserializeAdapted(null, null);
    assertEquals(null, received.getEnumField1());
    assertEquals(null, received.getEnumField2());

    assertTestStruct(received, null, null);
  }

  @Test
  public void testAdaptedOptionalEnumDefinedValue() {
    TestStructAdaptedOptional received = deserializeAdaptedOptional(1, 2);
    assertEquals("1", received.getEnumField1());
    assertEquals("2", received.getEnumField2());

    assertTestStruct(received, 1, 2);
  }

  @Test
  public void testAdaptedOptionalEnumDefaultValue() {
    TestStructAdaptedOptional received = deserializeAdaptedOptional(null, null);
    assertEquals(null, received.getEnumField1());
    assertEquals(null, received.getEnumField2());

    assertTestStruct(received, null, null);
  }

  @Test
  public void testOpenEnumTerseFields() {
    TestStructOpenTerse st = new TestStructOpenTerse.Builder().build();
    assertEquals(TestEnumOpen1.ZERO, st.getEnumField1());
    assertEquals(TestEnumOpen2.UNRECOGNIZED_VALUE, st.getEnumField2().getValue());
    assertEquals(0, st.getEnumField2().getUnrecognizedValue());
    assertEquals(0, EnumUtil.getValue(st.getEnumField2()));
  }

  @Test
  public void testOpenEnumEquals() {
    TestStructOpenTerse st =
        new TestStructOpenTerse.Builder()
            .setEnumField1(TestEnumOpen1.fromInteger(1))
            .setEnumField2(TestEnumOpen2.fromInteger(2))
            .build();
    TestStructOpenTerse st2 =
        new TestStructOpenTerse.Builder()
            .setEnumField1(TestEnumOpen1.fromInteger(1))
            .setEnumField2(TestEnumOpen2.fromInteger(2))
            .build();
    assertEquals(st, st2);

    st =
        new TestStructOpenTerse.Builder()
            .setEnumField1(TestEnumOpen1.fromInteger(1))
            .setEnumField2(TestEnumOpen2.fromInteger(1))
            .build();
    st2 =
        new TestStructOpenTerse.Builder()
            .setEnumField1(TestEnumOpen1.fromInteger(1))
            .setEnumField2(TestEnumOpen2.fromInteger(2))
            .build();
    assertNotEquals(st, st2);
  }

  @Test
  public void testOpenEnumEqualsUndeclaredValues() {
    TestStructOpenTerse st =
        new TestStructOpenTerse.Builder()
            .setEnumField1(TestEnumOpen1.fromInteger(100))
            .setEnumField2(TestEnumOpen2.fromInteger(200))
            .build();
    TestStructOpenTerse st2 =
        new TestStructOpenTerse.Builder()
            .setEnumField1(TestEnumOpen1.fromInteger(100))
            .setEnumField2(TestEnumOpen2.fromInteger(200))
            .build();
    assertEquals(st, st2);

    st =
        new TestStructOpenTerse.Builder()
            .setEnumField1(TestEnumOpen1.fromInteger(150))
            .setEnumField2(TestEnumOpen2.fromInteger(101))
            .build();
    st2 =
        new TestStructOpenTerse.Builder()
            .setEnumField1(TestEnumOpen1.fromInteger(150))
            .setEnumField2(TestEnumOpen2.fromInteger(200))
            .build();
    assertNotEquals(st, st2);
  }

  @Test
  public void testOpenEnumClone() {
    TestStructOpenTerse st =
        new TestStructOpenTerse.Builder()
            .setEnumField1(TestEnumOpen1.fromInteger(1))
            .setEnumField2(TestEnumOpen2.fromInteger(200))
            .build();
    TestStructOpenTerse st2 = new TestStructOpenTerse.Builder(st).build();
    assertEquals(st, st2);
  }

  @Test
  public void testUnionOpenEnum() {
    TestUnionOpen received = deserializeUnion(1, null);
    assertEquals(TestEnumOpen1.ONE, received.getEnumField1());

    assertTestStruct(received, 1, null);
  }

  @Test
  public void testUnionOpenEnumUnrecognizedValue() {
    TestUnionOpen received = deserializeUnion(null, 500);
    assertEquals(TestEnumOpen2.UNRECOGNIZED_VALUE, received.getEnumField2().getValue());
    assertEquals(500, received.getEnumField2().getUnrecognizedValue());

    assertTestStruct(received, null, 500);
  }

  @Test
  public void testExceptionOpenEnum() {
    TestExceptionOpen received = deserializeException(1, null);
    assertEquals(TestEnumOpen1.ONE, received.getEnumField1());

    assertTestStruct(received, 1, 0);
  }

  @Test
  public void testExceptionOpenEnumUnrecognizedValue() {
    TestExceptionOpen received = deserializeException(null, 500);
    assertEquals(TestEnumOpen2.UNRECOGNIZED_VALUE, received.getEnumField2().getValue());
    assertEquals(500, received.getEnumField2().getUnrecognizedValue());

    assertTestStruct(received, 0, 500);
  }

  private TestStructComplex deserializeComplex(TestStructComplex st) {
    byte[] bytes = SerializerUtil.toByteArray(st, SerializationProtocol.TCompact);
    return SerializerUtil.fromByteArray(
        TestStructComplex.asReader(), bytes, SerializationProtocol.TCompact);
  }

  @Test
  public void testListOfEnums() {
    List<TestEnumOpen1> list = new ArrayList<>();
    list.add(TestEnumOpen1.ONE);
    list.add(TestEnumOpen1.fromInteger(50));

    TestStructComplex st = new TestStructComplex.Builder().setListField(list).build();
    TestStructComplex received = deserializeComplex(st);

    assertEquals(TestEnumOpen1.ONE, received.getListField().get(0));
    assertEquals(TestEnumOpen1.fromInteger(50), received.getListField().get(1));
  }

  @Test
  public void testSetOfEnums() {
    Set<TestEnumOpen2> set = new HashSet<>();
    set.add(TestEnumOpen2.fromInteger(150));

    TestStructComplex st = new TestStructComplex.Builder().setSetField(set).build();
    TestStructComplex received = deserializeComplex(st);

    TestEnumOpen2 enum2 = received.getSetField().iterator().next();
    assertEquals(TestEnumOpen2.UNRECOGNIZED_VALUE, enum2.getValue());
    assertEquals(150, enum2.getUnrecognizedValue());
  }

  @Test
  public void testNestedObjects() {
    Map<TestEnumOpen1, Map<TestEnumOpen2, List<TestEnumOpen1>>> map = new HashMap<>();
    List<TestEnumOpen1> list1 = new ArrayList<>();
    list1.add(TestEnumOpen1.TWO);
    list1.add(TestEnumOpen1.fromInteger(70));

    List<TestEnumOpen1> list2 = new ArrayList<>();
    list2.add(TestEnumOpen1.TWO);
    list2.add(TestEnumOpen1.fromInteger(80));

    Map<TestEnumOpen2, List<TestEnumOpen1>> map1 = new HashMap<>();
    map1.put(TestEnumOpen2.THREE, list1);
    map1.put(TestEnumOpen2.fromInteger(10), list2);

    map.put(TestEnumOpen1.fromInteger(5), map1);

    TestStructComplex st = new TestStructComplex.Builder().setMapField(map).build();
    TestStructComplex received = deserializeComplex(st);

    Map<TestEnumOpen2, List<TestEnumOpen1>> enumMap =
        received.getMapField().get(TestEnumOpen1.fromInteger(5));
    List<TestEnumOpen1> enumList1 = enumMap.get(TestEnumOpen2.THREE);
    List<TestEnumOpen1> enumList2 = enumMap.get(TestEnumOpen2.fromInteger(10));

    assertArrayEquals(list1.toArray(), enumList1.toArray());
    assertArrayEquals(list2.toArray(), enumList2.toArray());
  }

  @Test
  public void testPackageLevelAnnotation() {
    TestStructOpenPkg st =
        new TestStructOpenPkg.Builder().setEnumField(TestEnumOpenPkg.fromInteger(20)).build();
    assertEquals(TestEnumOpenPkg.UNRECOGNIZED_VALUE, st.getEnumField().getValue());
    assertEquals(20, st.getEnumField().getUnrecognizedValue());
  }

  @Test
  public void testOverridePackageLevelAnnotation() {
    TestEnumOverridePkg e = TestEnumOverridePkg.fromInteger(2);
    assertEquals(TestEnumOverridePkg.TWO, e);

    e = TestEnumOverridePkg.fromInteger(5);
    assertNull(e);
  }
}
