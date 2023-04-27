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

package com.facebook.thrift;

import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.not;
import static org.hamcrest.core.IsNull.nullValue;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.fail;

import com.facebook.thrift.java.test.MySimpleStruct;
import com.facebook.thrift.java.test.MySimpleUnion;
import com.facebook.thrift.protocol.TBinaryProtocol;
import com.facebook.thrift.protocol.TCompactJSONProtocol;
import com.facebook.thrift.protocol.TCompactProtocol;
import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.protocol.TProtocolException;
import com.facebook.thrift.protocol.TProtocolFactory;
import com.facebook.thrift.transport.TMemoryBuffer;
import com.facebook.thrift.transport.TTransportException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import org.junit.Test;
import thrift.test.proto.Empty;
import thrift.test.union.RandomStuff;
import thrift.test.union.StructWithAUnion;
import thrift.test.union.TestUnion;

public class UnionTest {

  @Test
  public void testBasic() {
    TestUnion union = new TestUnion();
    assertThat(union.isSet(), is(false));
    assertThat(union.getFieldValue(), is(nullValue()));

    int fieldValue = 25;
    union = new TestUnion(TestUnion.I32_FIELD, fieldValue);
    assertThat(union.isSet(), is(true));
    assertThat(union.getFieldValue(), equalTo(fieldValue));
    assertThat(union.getFieldValue(TestUnion.I32_FIELD), equalTo(fieldValue));

    try {
      union.getFieldValue(TestUnion.STRING_FIELD);
      fail("was expecting an exception around wrong set field");
    } catch (IllegalArgumentException e) {
      // cool!
    }

    union = new TestUnion();
    fieldValue = 1;
    union.setI32_field(fieldValue);
    assertThat(union.getI32_field(), equalTo(fieldValue));

    try {
      union.getString_field();
      fail("should have gotten an exception");
    } catch (Exception e) {
      // sweet
    }
  }

  @Test
  public void testEquality() {
    TestUnion union = new TestUnion(TestUnion.I32_FIELD, 25);
    TestUnion otherUnion = new TestUnion(TestUnion.STRING_FIELD, "blah!!!");
    assertThat(union, is(not(equalTo(otherUnion))));

    otherUnion = new TestUnion(TestUnion.I32_FIELD, 400);
    assertThat(union, is(not(equalTo(otherUnion))));

    otherUnion = new TestUnion(TestUnion.OTHER_I32_FIELD, 25);
    assertThat(union, is(not(equalTo(otherUnion))));
  }

  @Test
  public void testSerialization() {
    TestUnion union = new TestUnion(TestUnion.I32_FIELD, 25);

    TMemoryBuffer buf = new TMemoryBuffer(0);
    TProtocol proto = new TBinaryProtocol(buf);
    union.write(proto);

    TestUnion u2 = new TestUnion();
    u2.read(proto);
    assertThat(u2, equalTo(union));

    StructWithAUnion swau = StructWithAUnion.builder().setTest_union(u2).build();
    buf = new TMemoryBuffer(0);
    proto = new TBinaryProtocol(buf);
    swau.write(proto);
    StructWithAUnion swau2 = new StructWithAUnion();
    assertThat(swau2, is(not(equalTo(swau))));

    swau2.read(proto);
    assertThat(swau2, equalTo(swau));

    // this should NOT throw an exception.
    buf = new TMemoryBuffer(0);
    proto = new TBinaryProtocol(buf);
    swau.write(proto);
    new Empty().read(proto);
  }

  @Test
  public void testJSONSerialization() {
    TDeserializer deserializer = new TDeserializer(new TCompactJSONProtocol.Factory());

    TSerializer serializer = new TSerializer(new TCompactJSONProtocol.Factory());

    // Deserialize empty union
    TestUnion emptyUnion = new TestUnion();
    String emptyUnionJSON = "{}";
    TestUnion union = new TestUnion(TestUnion.I32_FIELD, 25);
    deserializer.fromString(union, emptyUnionJSON);
    assertThat(emptyUnion, equalTo(union));

    // Serialize union then deserialize it. Should be the same.
    TestUnion union2 = new TestUnion(TestUnion.I32_FIELD, 25);
    String unionJSON = serializer.toString(union2, "UTF-8");
    TestUnion union3 = new TestUnion();
    deserializer.fromString(union3, unionJSON);
    assertThat(union3, equalTo(union2));

    // Serialize union with inner list then deserialize it. Should be the same.
    List<RandomStuff> randomList = new ArrayList<>();
    randomList.add(
        RandomStuff.builder()
            .setA(1)
            .setB(2)
            .setC(3)
            .setD(4)
            .setMyintlist(new ArrayList<>())
            .setBigint(10L)
            .setTriple(10.5)
            .build());
    TestUnion unionWithList = new TestUnion(TestUnion.STRUCT_LIST, randomList);
    String unionWithListJSON = serializer.toString(unionWithList, "UTF-8");
    TestUnion unionWithList2 = new TestUnion();
    deserializer.fromString(unionWithList2, unionWithListJSON);
    assertThat(unionWithList2, equalTo(unionWithList));
    assertThat(unionWithList2.getStruct_list(), equalTo(randomList));

    // Serialize struct with union then deserialize it. Should be the same.
    StructWithAUnion swau = StructWithAUnion.builder().setTest_union(union2).build();
    String swauJSON = serializer.toString(swau, "UTF-8");
    StructWithAUnion swau2 = new StructWithAUnion();
    deserializer.fromString(swau2, swauJSON);
    assertThat(swau2, equalTo(swau));
  }

  @Test(expected = TProtocolException.class)
  public void testEmptyUnionBinarySerialization() {
    TMemoryBuffer buf = new TMemoryBuffer(0);
    TProtocol binaryProto = new TBinaryProtocol(buf);
    TestUnion emptyUnion = new TestUnion();

    // Should throw a TProtocolException when writing an empty union
    emptyUnion.write(binaryProto);
  }

  @Test(expected = TProtocolException.class)
  public void testEmptyUnionCompactSerialization() {
    TMemoryBuffer buf = new TMemoryBuffer(0);
    TProtocolFactory compactFactory = new TCompactProtocol.Factory();
    TProtocol compactProto = compactFactory.getProtocol(buf);
    TestUnion emptyUnion = new TestUnion();

    // Should throw a TProtocolException when writing an empty union
    emptyUnion.write(compactProto);
  }

  @Test(expected = TTransportException.class)
  public void testEmptyUnionBinaryDeserialization() {
    TMemoryBuffer buf = new TMemoryBuffer(0);
    TProtocol binaryProto = new TBinaryProtocol(buf);
    TestUnion emptyUnion = new TestUnion();

    // Should throw a TTransportException when reading no bytes
    emptyUnion.read(binaryProto);
  }

  @Test(expected = TTransportException.class)
  public void testEmptyUnionCompactDeserialization() {
    TMemoryBuffer buf = new TMemoryBuffer(0);
    TProtocolFactory compactFactory = new TCompactProtocol.Factory();
    TProtocol compactProto = compactFactory.getProtocol(buf);
    TestUnion emptyUnion = new TestUnion();

    // Should throw a TTransportException when reading no bytes
    emptyUnion.read(compactProto);
  }

  @Test
  public void testAndroidUnion() {
    TMemoryBuffer buf = new TMemoryBuffer(0);
    TProtocolFactory factory = new TCompactProtocol.Factory();
    TProtocol proto = factory.getProtocol(buf);

    MySimpleUnion union = MySimpleUnion.caseOne(61753);
    union.write(proto);

    com.facebook.thrift.android.test.MySimpleUnion androidUnion =
        new com.facebook.thrift.android.test.MySimpleUnion();
    androidUnion.read(proto);

    assertThat(androidUnion.getCaseOne(), equalTo(union.getCaseOne()));
  }

  @Test
  public void testInvalidUnion() {
    try {
      MySimpleUnion invalidUnion = new MySimpleUnion(1, null);
      fail();
    } catch (IllegalArgumentException e) {
      assertThat(e.getMessage(), equalTo("TUnion value for field id '1' can't be null!"));
    }
  }

  @Test
  public void testToStringOnEmptyUnion() {
    MySimpleUnion union = new MySimpleUnion();
    assertThat(union.toString(), equalTo("<MySimpleUnion uninitialized>"));

    com.facebook.thrift.android.test.MySimpleUnion androidUnion =
        new com.facebook.thrift.android.test.MySimpleUnion();
    assertThat(union.toString(), equalTo("<MySimpleUnion uninitialized>"));
  }

  @Test
  public void testUnionConstructor() {
    MySimpleUnion union =
        new MySimpleUnion(MySimpleUnion.CASEFOUR, new MySimpleStruct(1L, "blabla"));
    com.facebook.thrift.android.test.MySimpleUnion androidUnion =
        new com.facebook.thrift.android.test.MySimpleUnion(
            MySimpleUnion.CASEFOUR,
            new com.facebook.thrift.android.test.MySimpleStruct(1L, "blabla"));

    MySimpleUnion union5 = new MySimpleUnion(MySimpleUnion.CASEFIVE, Arrays.asList("foo", "bar"));
    com.facebook.thrift.android.test.MySimpleUnion androidUnion5 =
        new com.facebook.thrift.android.test.MySimpleUnion(
            MySimpleUnion.CASEFIVE, Arrays.asList("foo", "bar"));
  }

  @Test
  public void union_shouldReturnTheRightName() {
    com.facebook.thrift.javaswift.test.MySimpleUnion union =
        com.facebook.thrift.javaswift.test.MySimpleUnion.fromCaseOne(1);
    assertThat(union.getThriftName(), equalTo("caseOne"));
  }
}
