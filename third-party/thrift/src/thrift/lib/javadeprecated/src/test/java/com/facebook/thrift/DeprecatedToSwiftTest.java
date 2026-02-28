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

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.Assert.assertThat;

import com.facebook.thrift.javaswift.test.ComplexNestedStruct;
import com.facebook.thrift.javaswift.test.DefaultValueStruct;
import com.facebook.thrift.javaswift.test.MyOptioalStruct;
import com.facebook.thrift.javaswift.test.MySimpleStruct;
import com.facebook.thrift.javaswift.test.MySimpleUnion;
import com.facebook.thrift.javaswift.test.SimpleCollectionStruct;
import com.facebook.thrift.javaswift.test.SimpleStructTypes;
import com.facebook.thrift.javaswift.test.SmallEnum;
import com.facebook.thrift.javaswift.test.TypeRemapped;
import com.facebook.thrift.swift.adapters.ApacheToFacebookProtocolAdapter;
import it.unimi.dsi.fastutil.ints.Int2LongArrayMap;
import it.unimi.dsi.fastutil.ints.Int2ObjectArrayMap;
import it.unimi.dsi.fastutil.longs.Long2ObjectArrayMap;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.junit.Test;

public class DeprecatedToSwiftTest {

  @Test
  public void testWriteEqualDeprecatedReadSimple() throws Exception {
    long idValue = 4444444444444444444L;
    String nameValue = "Hello Thrift Team";

    org.apache.thrift.transport.TMemoryBuffer buf =
        new org.apache.thrift.transport.TMemoryBuffer(0);
    org.apache.thrift.protocol.TProtocolFactory factory =
        new org.apache.thrift.protocol.TCompactProtocol.Factory();

    org.apache.thrift.protocol.TProtocol apacheProto = factory.getProtocol(buf);
    MySimpleStruct struct1 = new MySimpleStruct(idValue, nameValue);
    struct1.write0(apacheProto);
    com.facebook.thrift.java.test.MySimpleStruct structJavaDeprecated =
        new com.facebook.thrift.java.test.MySimpleStruct();

    ApacheToFacebookProtocolAdapter protocol = new ApacheToFacebookProtocolAdapter(apacheProto);
    structJavaDeprecated.read(protocol);
    assertThat(structJavaDeprecated.getId(), equalTo(idValue));
    assertThat(structJavaDeprecated.getName(), equalTo(nameValue));
  }

  @Test
  public void testWriteEqualDeprecatedReadSimpleDFValue() throws Exception {
    org.apache.thrift.transport.TMemoryBuffer buf =
        new org.apache.thrift.transport.TMemoryBuffer(0);
    org.apache.thrift.protocol.TProtocolFactory factory =
        new org.apache.thrift.protocol.TCompactProtocol.Factory();

    org.apache.thrift.protocol.TProtocol apacheProto = factory.getProtocol(buf);
    MySimpleStruct struct1 = new MySimpleStruct.Builder().build();
    struct1.write0(apacheProto);
    com.facebook.thrift.java.test.MySimpleStruct structJavaDeprecated =
        new com.facebook.thrift.java.test.MySimpleStruct();

    ApacheToFacebookProtocolAdapter protocol = new ApacheToFacebookProtocolAdapter(apacheProto);
    structJavaDeprecated.read(protocol);
    assertThat(structJavaDeprecated.getId(), equalTo(99L));
    assertThat(structJavaDeprecated.getName(), equalTo("Batman"));
  }

  @Test
  public void testWriteEqualDeprecatedReadSimpleUnion() throws Exception {
    long idValue = 4444444444444444444L;
    org.apache.thrift.transport.TMemoryBuffer buf =
        new org.apache.thrift.transport.TMemoryBuffer(0);
    org.apache.thrift.protocol.TProtocolFactory factory =
        new org.apache.thrift.protocol.TCompactProtocol.Factory();

    org.apache.thrift.protocol.TProtocol apacheProto = factory.getProtocol(buf);
    MySimpleUnion union = MySimpleUnion.fromCaseOne(idValue);
    union.write0(apacheProto);

    com.facebook.thrift.java.test.MySimpleUnion unionJavaDeprecated =
        new com.facebook.thrift.java.test.MySimpleUnion();
    ApacheToFacebookProtocolAdapter protocol = new ApacheToFacebookProtocolAdapter(apacheProto);
    unionJavaDeprecated.read(protocol);

    assertThat(unionJavaDeprecated.getCaseOne(), equalTo(idValue));
  }

  @Test
  public void testReadEqualWriteSimpleUnion() throws Exception {
    long idValue = 4444444444444444444L;
    String nameValue = "Hello Thrift Team";

    org.apache.thrift.transport.TMemoryBuffer buf =
        new org.apache.thrift.transport.TMemoryBuffer(0);
    org.apache.thrift.protocol.TProtocolFactory factory =
        new org.apache.thrift.protocol.TCompactProtocol.Factory();
    org.apache.thrift.protocol.TProtocol apacheProto = factory.getProtocol(buf);

    MySimpleStruct struct1 = new MySimpleStruct(idValue, nameValue);
    MySimpleUnion union = MySimpleUnion.fromCaseFour(struct1);
    union.write0(apacheProto);
    MySimpleUnion unionCreatedFromRead = MySimpleUnion.read0(apacheProto);
    assertThat(unionCreatedFromRead.getCaseFour(), equalTo(struct1));
  }

  @Test
  public void testWriteEqualDeprecatedReadStructs() throws Exception {
    String msg = "Hello Thrift Team";
    boolean b = true;
    byte y = 111;
    short i = 0;
    int j = Integer.MAX_VALUE;
    long k = 4444444444444444444L;
    double d = -5555555.5555;

    org.apache.thrift.transport.TMemoryBuffer buf =
        new org.apache.thrift.transport.TMemoryBuffer(0);
    org.apache.thrift.protocol.TProtocolFactory factory =
        new org.apache.thrift.protocol.TCompactProtocol.Factory();

    org.apache.thrift.protocol.TProtocol apacheProto = factory.getProtocol(buf);
    SimpleStructTypes struct1 = new SimpleStructTypes(msg, b, y, i, j, k, d);
    struct1.write0(apacheProto);
    com.facebook.thrift.java.test.SimpleStructTypes structJavaDeprecated =
        new com.facebook.thrift.java.test.SimpleStructTypes();

    ApacheToFacebookProtocolAdapter protocol = new ApacheToFacebookProtocolAdapter(apacheProto);
    structJavaDeprecated.read(protocol);
    assertThat(structJavaDeprecated.getMsg(), equalTo(msg));
    assertThat(structJavaDeprecated.isB(), equalTo(b));
    assertThat(structJavaDeprecated.getY(), equalTo(y));
    assertThat(structJavaDeprecated.getI(), equalTo(i));
    assertThat(structJavaDeprecated.getJ(), equalTo(j));
    assertThat(structJavaDeprecated.getK(), equalTo(k));
    assertThat(structJavaDeprecated.getD(), equalTo(d));
  }

  @Test
  public void testReadEqualWriteStructTypes() throws Exception {
    String msg = "Hello Thrift Team";
    boolean b = true;
    byte y = 111;
    short i = 0;
    int j = Integer.MAX_VALUE;
    long k = 4444444444444444444L;
    double d = -5555555.5555;

    org.apache.thrift.transport.TMemoryBuffer buf =
        new org.apache.thrift.transport.TMemoryBuffer(0);
    org.apache.thrift.protocol.TProtocolFactory factory =
        new org.apache.thrift.protocol.TCompactProtocol.Factory();

    org.apache.thrift.protocol.TProtocol apacheProto = factory.getProtocol(buf);
    SimpleStructTypes struct1 = new SimpleStructTypes(msg, b, y, i, j, k, d);
    struct1.write0(apacheProto);

    SimpleStructTypes structCreatedFromRead = SimpleStructTypes.read0(apacheProto);
    assertThat(structCreatedFromRead.getMsg(), equalTo(msg));
    assertThat(structCreatedFromRead.isB(), equalTo(b));
    assertThat(structCreatedFromRead.getY(), equalTo(y));
    assertThat(structCreatedFromRead.getI(), equalTo(i));
    assertThat(structCreatedFromRead.getJ(), equalTo(j));
    assertThat(structCreatedFromRead.getK(), equalTo(k));
    assertThat(structCreatedFromRead.getD(), equalTo(d));
  }

  @Test
  public void testDefaultValuesSimpleStructTypes() throws Exception {
    String defaultMsg = "Bye Thrift Team";
    boolean defaultB = false;
    byte defaultY = 97;
    short defaultI = 1;
    int defaultJ = -9999;
    long defaultK = 14444444444444L;
    double defaultD = 14;

    org.apache.thrift.transport.TMemoryBuffer buf =
        new org.apache.thrift.transport.TMemoryBuffer(0);
    org.apache.thrift.protocol.TProtocolFactory factory =
        new org.apache.thrift.protocol.TCompactProtocol.Factory();

    org.apache.thrift.protocol.TProtocol apacheProto = factory.getProtocol(buf);
    SimpleStructTypes struct1 = new SimpleStructTypes.Builder().build();
    struct1.write0(apacheProto);

    SimpleStructTypes structCreatedFromRead = SimpleStructTypes.read0(apacheProto);
    assertThat(structCreatedFromRead.getMsg(), equalTo(defaultMsg));
    assertThat(structCreatedFromRead.isB(), equalTo(defaultB));
    assertThat(structCreatedFromRead.getY(), equalTo(defaultY));
    assertThat(structCreatedFromRead.getI(), equalTo(defaultI));
    assertThat(structCreatedFromRead.getJ(), equalTo(defaultJ));
    assertThat(structCreatedFromRead.getK(), equalTo(defaultK));
    assertThat(structCreatedFromRead.getD(), equalTo(defaultD));
  }

  @Test
  public void testDefaultValues() throws Exception {
    long defaultMyLongDFset = 10L;
    long defaultMyLongDF = 0L; // Default value
    int defaultPortDFset = 3456; // Constant PortNum
    int defaultPortNum = 0;
    byte[] defaultMyBinaryDFset = "abc".getBytes(); // binary
    byte[] defaultMyBinary = null; // Default binary
    byte defaultByteDFSet = (byte) 17;
    byte defaultMyByte = (byte) 0;
    double defaultMyDoubleDFset = 99.7678;
    double defaultMyDoubleDFZero = 0.0;
    double defaultMyDouble = 0.0;
    Map<Integer, String> defaultMapIntStr = new HashMap<Integer, String>(2);
    defaultMapIntStr.put(15, "a_value");
    defaultMapIntStr.put(2, "b_value");
    MySimpleStruct simpleStruct = new MySimpleStruct.Builder().setId(40L).setName("John").build();
    List<SmallEnum> defaultMyList = new ArrayList<SmallEnum>(3);
    defaultMyList.add(SmallEnum.RED);
    defaultMyList.add(SmallEnum.BLUE);
    defaultMyList.add(SmallEnum.GREEN);
    HashSet<String> defaultMySet = new HashSet<String>(3);
    defaultMySet.add("house");
    defaultMySet.add("car");
    defaultMySet.add("dog");
    List<MySimpleStruct> defaultListStructDFset = new ArrayList<MySimpleStruct>(2);
    defaultListStructDFset.add(new MySimpleStruct.Builder().setId(40L).setName("IronMan").build());
    defaultListStructDFset.add(new MySimpleStruct.Builder().setId(999L).setName("Thanos").build());
    MySimpleUnion defaultMyUnion = MySimpleUnion.fromSmallEnum(SmallEnum.BLUE);
    List<MySimpleUnion> defaultListUnionDFset = new ArrayList<MySimpleUnion>(2);
    defaultListUnionDFset.add(MySimpleUnion.fromSmallEnum(SmallEnum.BLUE));
    defaultListUnionDFset.add(MySimpleUnion.fromCaseTwo(123L));
    Map<Integer, List<MySimpleStruct>> defaultMapNestlistStructDfSet =
        new HashMap<Integer, List<MySimpleStruct>>(3);
    defaultMapNestlistStructDfSet.put(1, defaultListStructDFset);
    List<MySimpleStruct> defaultListStructDFsetTwo = new ArrayList<MySimpleStruct>(2);
    defaultListStructDFsetTwo.add(
        new MySimpleStruct.Builder().setId(28L).setName("BatMan").build());
    defaultListStructDFsetTwo.add(new MySimpleStruct.Builder().setId(12L).setName("Robin").build());
    defaultMapNestlistStructDfSet.put(2, defaultListStructDFsetTwo);
    List<MySimpleStruct> defaultListStructDFsetThree = new ArrayList<MySimpleStruct>(2);
    defaultListStructDFsetThree.add(
        new MySimpleStruct.Builder().setId(12L).setName("RatMan").build());
    defaultListStructDFsetThree.add(
        new MySimpleStruct.Builder().setId(6L).setName("Catman").build());
    defaultMapNestlistStructDfSet.put(5, defaultListStructDFsetThree);
    Map<Long, Integer> defaultEmptyMap = new HashMap<Long, Integer>();
    Map<String, Map<Integer, SmallEnum>> defaultEnumMapDFset =
        new HashMap<String, Map<Integer, SmallEnum>>(3);
    Map<Integer, SmallEnum> innerMap = new HashMap<Integer, SmallEnum>(2);
    innerMap.put(16, SmallEnum.RED);
    innerMap.put(144, SmallEnum.RED);
    defaultEnumMapDFset.put("SANDY BRIDGE", innerMap);
    Map<Integer, SmallEnum> innerMapTwo = new HashMap<Integer, SmallEnum>(2);
    innerMapTwo.put(32, SmallEnum.GREEN);
    innerMapTwo.put(144, SmallEnum.BLUE);
    defaultEnumMapDFset.put("IVY BRIDGE", innerMapTwo);
    Map<Integer, SmallEnum> innerMapThree = new HashMap<Integer, SmallEnum>(3);
    innerMapThree.put(32, SmallEnum.RED);
    innerMapThree.put(128, SmallEnum.BLUE);
    innerMapThree.put(256, SmallEnum.GREEN);
    defaultEnumMapDFset.put("HASWELL", innerMapThree);
    Long2ObjectArrayMap<String> defaultMapJavaTypeDFset = new Long2ObjectArrayMap<String>();
    defaultMapJavaTypeDFset.put(15L, "a_value");
    defaultMapJavaTypeDFset.put(2L, "b_value");

    org.apache.thrift.transport.TMemoryBuffer buf =
        new org.apache.thrift.transport.TMemoryBuffer(0);
    org.apache.thrift.protocol.TProtocolFactory factory =
        new org.apache.thrift.protocol.TCompactProtocol.Factory();

    org.apache.thrift.protocol.TProtocol apacheProto = factory.getProtocol(buf);
    DefaultValueStruct struct1 = new DefaultValueStruct.Builder().build();
    struct1.write0(apacheProto);

    DefaultValueStruct structCreatedFromRead = DefaultValueStruct.read0(apacheProto);
    assertThat(structCreatedFromRead.getMyLongDFset(), equalTo(defaultMyLongDFset));
    assertThat(structCreatedFromRead.getMyLongDF(), equalTo(defaultMyLongDF));
    assertThat(structCreatedFromRead.getPortDFset(), equalTo(defaultPortDFset));
    assertThat(structCreatedFromRead.getPortNum(), equalTo(defaultPortNum));
    assertThat(structCreatedFromRead.getMyByteDFSet(), equalTo(defaultByteDFSet));
    assertThat(structCreatedFromRead.getMyByte(), equalTo(defaultMyByte));
    assertThat(structCreatedFromRead.getMyDoubleDFset(), equalTo(defaultMyDoubleDFset));
    assertThat(structCreatedFromRead.getMyDoubleDFZero(), equalTo(defaultMyDoubleDFZero));
    assertThat(structCreatedFromRead.getMyDouble(), equalTo(defaultMyDouble));
    assertThat(structCreatedFromRead.getMIntegerString(), equalTo(defaultMapIntStr));
    assertThat(structCreatedFromRead.getMyList(), equalTo(defaultMyList));
    assertThat(structCreatedFromRead.getMySet(), equalTo(defaultMySet));
    assertThat(structCreatedFromRead.getListStructDFset(), equalTo(defaultListStructDFset));
    assertThat(structCreatedFromRead.getMyUnion(), equalTo(defaultMyUnion));
    assertThat(structCreatedFromRead.getListUnionDFset(), equalTo(defaultListUnionDFset));
    assertThat(
        structCreatedFromRead.getMapNestlistStructDfSet(), equalTo(defaultMapNestlistStructDfSet));
    assertThat(structCreatedFromRead.getEmptyMap(), equalTo(defaultEmptyMap));
    assertThat(structCreatedFromRead.getEnumMapDFset(), equalTo(defaultEnumMapDFset));
    assertThat(structCreatedFromRead.getMapJavaTypeDFset(), equalTo(defaultMapJavaTypeDFset));
    assertThat(structCreatedFromRead.getMyBinaryDFset(), equalTo(defaultMyBinaryDFset));
    assertThat(structCreatedFromRead.getMyBinary(), equalTo(defaultMyBinary));
  }

  @Test
  public void testWriteEqualDeprecatedCollectionStructs() throws Exception {
    List<Double> lDouble = new ArrayList<Double>();
    lDouble.add(5555555.5555);
    lDouble.add(0.0);
    lDouble.add(-5555555.5555);
    List<Short> lShort = new ArrayList<Short>();
    lShort.add((short) 111);
    lShort.add((short) 0);
    lShort.add((short) -111);
    Map<Integer, String> mIntegerString = new HashMap<Integer, String>();
    mIntegerString.put(-1, "Hello");
    mIntegerString.put(0, "Thrift");
    mIntegerString.put(1, "Team");
    Map<String, String> mStringString = new HashMap<String, String>();
    mStringString.put("Team", "Hello");
    mStringString.put("Thrift", "Thrift");
    mStringString.put("Hello", "Team");
    Set<Long> sLong = new HashSet<Long>();
    sLong.add(4444444444444444444L);
    sLong.add(0L);
    sLong.add(-4444444444444444444L);

    org.apache.thrift.transport.TMemoryBuffer buf =
        new org.apache.thrift.transport.TMemoryBuffer(0);
    org.apache.thrift.protocol.TProtocolFactory factory =
        new org.apache.thrift.protocol.TCompactProtocol.Factory();

    org.apache.thrift.protocol.TProtocol apacheProto = factory.getProtocol(buf);
    SimpleCollectionStruct struct1 =
        new SimpleCollectionStruct(lDouble, lShort, mIntegerString, mStringString, sLong);
    struct1.write0(apacheProto);
    com.facebook.thrift.java.test.SimpleCollectionStruct structJavaDeprecated =
        new com.facebook.thrift.java.test.SimpleCollectionStruct();

    ApacheToFacebookProtocolAdapter protocol = new ApacheToFacebookProtocolAdapter(apacheProto);
    structJavaDeprecated.read(protocol);
    assertThat(structJavaDeprecated.getLDouble(), equalTo(lDouble));
    assertThat(structJavaDeprecated.getLShort(), equalTo(lShort));
    assertThat(structJavaDeprecated.getMIntegerString(), equalTo(mIntegerString));
    assertThat(structJavaDeprecated.getMStringString(), equalTo(mStringString));
    assertThat(structJavaDeprecated.getSLong(), equalTo(sLong));
  }

  @Test
  public void testSwiftWriteVsDeprecatedReadNestedStructs() throws Exception {

    Set<Set<Integer>> setOfSetOfInt = new HashSet<Set<Integer>>();
    Set<Integer> sIntOne = new HashSet<Integer>();
    sIntOne.add(-1);
    sIntOne.add(0);
    Set<Integer> sIntTwo = new HashSet<Integer>();
    sIntTwo.add(1);
    sIntTwo.add(2);
    setOfSetOfInt.add(sIntOne);
    setOfSetOfInt.add(sIntTwo);

    List<List<SmallEnum>> listOfListOfEnum = new ArrayList<List<SmallEnum>>();
    List<SmallEnum> lENUMOne = new ArrayList<SmallEnum>();
    lENUMOne.add(SmallEnum.RED);
    lENUMOne.add(SmallEnum.BLUE);
    List<SmallEnum> lEnumTwo = new ArrayList<SmallEnum>();
    lEnumTwo.add(SmallEnum.GREEN);
    listOfListOfEnum.add(lENUMOne);
    listOfListOfEnum.add(lEnumTwo);
    List<SmallEnum> lEnumThree = new ArrayList<SmallEnum>();
    lEnumThree.add(SmallEnum.GREEN);
    lEnumThree.add(SmallEnum.GREEN);
    List<List<SmallEnum>> listOfListOfEnumTwo = new ArrayList<List<SmallEnum>>();
    listOfListOfEnumTwo.add(lEnumThree);

    List<List<MySimpleStruct>> listOfListOfMyStruct = new ArrayList<List<MySimpleStruct>>();
    long idValueOne = 4444444444444444444L;
    String nameValueOne = "Hello Thrift Team";
    MySimpleStruct structOne = new MySimpleStruct(idValueOne, nameValueOne);
    long idValueTwo = 5L;
    String nameValueTwo = "Hello Batman!";
    MySimpleStruct structTwo = new MySimpleStruct(idValueTwo, nameValueTwo);
    List<MySimpleStruct> lStructOne = new ArrayList<MySimpleStruct>();
    lStructOne.add(structOne);
    List<MySimpleStruct> lStructTwo = new ArrayList<MySimpleStruct>();
    lStructTwo.add(structTwo);
    listOfListOfMyStruct.add(lStructOne);
    listOfListOfMyStruct.add(lStructTwo);

    Set<List<List<String>>> setOfListOfListOfString = new HashSet<List<List<String>>>();
    List<String> lstringOne = new ArrayList<String>();
    List<String> lstringTwo = new ArrayList<String>();
    lstringOne.add(nameValueOne);
    lstringTwo.add(nameValueTwo);
    List<List<String>> listOfListString = new ArrayList<List<String>>();
    listOfListString.add(lstringOne);
    listOfListString.add(lstringTwo);
    List<List<String>> listOfListStringEmpty = new ArrayList<List<String>>();
    setOfListOfListOfString.add(listOfListString);
    setOfListOfListOfString.add(listOfListStringEmpty);

    HashMap<Integer, List<List<SmallEnum>>> mapKeyIntValListOfListOfEnum =
        new HashMap<Integer, List<List<SmallEnum>>>();
    mapKeyIntValListOfListOfEnum.put(101, listOfListOfEnum);
    mapKeyIntValListOfListOfEnum.put(100, listOfListOfEnumTwo);

    Map<Map<Integer, String>, Map<String, String>> mapKeyMapValMap =
        new HashMap<Map<Integer, String>, Map<String, String>>();
    Map<Integer, String> mIntegerString = new HashMap<Integer, String>();
    mIntegerString.put(-1, "Hello");
    mIntegerString.put(0, "Thrift");
    mIntegerString.put(1, "Team");
    Map<String, String> mStringString = new HashMap<String, String>();
    mStringString.put("Team", "Hello");
    mStringString.put("Thrift", "Thrift");
    mStringString.put("Batman", "Thanos");
    mapKeyMapValMap.put(mIntegerString, mStringString);

    MySimpleUnion mySimpleUnion = MySimpleUnion.fromCaseOne(idValueOne);

    org.apache.thrift.transport.TMemoryBuffer buf =
        new org.apache.thrift.transport.TMemoryBuffer(0);
    org.apache.thrift.protocol.TProtocolFactory factory =
        new org.apache.thrift.protocol.TCompactProtocol.Factory();

    org.apache.thrift.protocol.TProtocol apacheProto = factory.getProtocol(buf);
    ComplexNestedStruct complexNestedStruct =
        new ComplexNestedStruct(
            setOfSetOfInt,
            listOfListOfEnum,
            listOfListOfMyStruct,
            setOfListOfListOfString,
            mapKeyIntValListOfListOfEnum,
            mapKeyMapValMap,
            mySimpleUnion);

    complexNestedStruct.write0(apacheProto);
    com.facebook.thrift.java.test.ComplexNestedStruct structJavaDeprecated =
        new com.facebook.thrift.java.test.ComplexNestedStruct();

    ApacheToFacebookProtocolAdapter protocol = new ApacheToFacebookProtocolAdapter(apacheProto);
    structJavaDeprecated.read(protocol);

    assertThat(structJavaDeprecated.getSetOfSetOfInt(), equalTo(setOfSetOfInt));
    assertThat(structJavaDeprecated.getSetOfListOfListOfString(), equalTo(setOfListOfListOfString));
    assertThat(structJavaDeprecated.getMapKeyMapValMap(), equalTo(mapKeyMapValMap));
    assertThat(structJavaDeprecated.getMyUnion().getCaseOne(), equalTo(idValueOne));
    // First list of listOfListOfMyStruct
    assertThat(
        structJavaDeprecated.getListOfListOfMyStruct().get(0).get(0).getId(), equalTo(idValueOne));
    assertThat(
        structJavaDeprecated.getListOfListOfMyStruct().get(0).get(0).getName(),
        equalTo(nameValueOne));
    // Second list of listOfListOfMyStruct
    assertThat(
        structJavaDeprecated.getListOfListOfMyStruct().get(1).get(0).getId(), equalTo(idValueTwo));
    assertThat(
        structJavaDeprecated.getListOfListOfMyStruct().get(1).get(0).getName(),
        equalTo(nameValueTwo));

    // red=1, blue=2, green=3
    assertThat(structJavaDeprecated.getListOfListOfEnum().get(0).get(0).getValue(), equalTo(1));
    assertThat(structJavaDeprecated.getListOfListOfEnum().get(0).get(1).getValue(), equalTo(2));
    assertThat(structJavaDeprecated.getListOfListOfEnum().get(1).get(0).getValue(), equalTo(3));
    assertThat(
        structJavaDeprecated.getMapKeyIntValListOfListOfEnum().get(101).get(0).get(0).getValue(),
        equalTo(1));
    assertThat(
        structJavaDeprecated.getMapKeyIntValListOfListOfEnum().get(101).get(0).get(1).getValue(),
        equalTo(2));
    assertThat(
        structJavaDeprecated.getMapKeyIntValListOfListOfEnum().get(101).get(1).get(0).getValue(),
        equalTo(3));
    assertThat(
        structJavaDeprecated.getMapKeyIntValListOfListOfEnum().get(100).get(0).get(0).getValue(),
        equalTo(3));
    assertThat(
        structJavaDeprecated.getMapKeyIntValListOfListOfEnum().get(100).get(0).get(1).getValue(),
        equalTo(3));
  }

  @Test
  public void testSwiftReadVsWriteNestedStructs() throws Exception {
    Set<Set<Integer>> setOfSetOfInt = new HashSet<Set<Integer>>();
    Set<Integer> sIntOne = new HashSet<Integer>();
    sIntOne.add(-1);
    sIntOne.add(0);
    Set<Integer> sIntTwo = new HashSet<Integer>();
    sIntTwo.add(1);
    sIntTwo.add(2);
    setOfSetOfInt.add(sIntOne);
    setOfSetOfInt.add(sIntTwo);

    List<List<SmallEnum>> listOfListOfEnum = new ArrayList<List<SmallEnum>>();
    List<SmallEnum> lENUMOne = new ArrayList<SmallEnum>();
    lENUMOne.add(SmallEnum.RED);
    lENUMOne.add(SmallEnum.BLUE);
    List<SmallEnum> lEnumTwo = new ArrayList<SmallEnum>();
    lEnumTwo.add(SmallEnum.GREEN);
    listOfListOfEnum.add(lENUMOne);
    listOfListOfEnum.add(lEnumTwo);
    List<SmallEnum> lEnumThree = new ArrayList<SmallEnum>();
    lEnumThree.add(SmallEnum.GREEN);
    lEnumThree.add(SmallEnum.GREEN);
    List<List<SmallEnum>> listOfListOfEnumTwo = new ArrayList<List<SmallEnum>>();
    listOfListOfEnumTwo.add(lEnumThree);

    List<List<MySimpleStruct>> listOfListOfMyStruct = new ArrayList<List<MySimpleStruct>>();
    long idValueOne = 4444444444444444444L;
    String nameValueOne = "Hello Thrift Team";
    MySimpleStruct structOne = new MySimpleStruct(idValueOne, nameValueOne);
    long idValueTwo = 5L;
    String nameValueTwo = "Hello Batman!";
    MySimpleStruct structTwo = new MySimpleStruct(idValueTwo, nameValueTwo);
    List<MySimpleStruct> lStructOne = new ArrayList<MySimpleStruct>();
    lStructOne.add(structOne);
    List<MySimpleStruct> lStructTwo = new ArrayList<MySimpleStruct>();
    lStructTwo.add(structTwo);
    listOfListOfMyStruct.add(lStructOne);
    listOfListOfMyStruct.add(lStructTwo);

    Set<List<List<String>>> setOfListOfListOfString = new HashSet<List<List<String>>>();
    List<String> lstringOne = new ArrayList<String>();
    List<String> lstringTwo = new ArrayList<String>();
    lstringOne.add(nameValueOne);
    lstringTwo.add(nameValueTwo);
    List<List<String>> listOfListString = new ArrayList<List<String>>();
    listOfListString.add(lstringOne);
    listOfListString.add(lstringTwo);
    List<List<String>> listOfListStringEmpty = new ArrayList<List<String>>();
    setOfListOfListOfString.add(listOfListString);
    setOfListOfListOfString.add(listOfListStringEmpty);

    HashMap<Integer, List<List<SmallEnum>>> mapKeyIntValListOfListOfEnum =
        new HashMap<Integer, List<List<SmallEnum>>>();
    mapKeyIntValListOfListOfEnum.put(101, listOfListOfEnum);
    mapKeyIntValListOfListOfEnum.put(100, listOfListOfEnumTwo);

    Map<Map<Integer, String>, Map<String, String>> mapKeyMapValMap =
        new HashMap<Map<Integer, String>, Map<String, String>>();
    Map<Integer, String> mIntegerString = new HashMap<Integer, String>();
    mIntegerString.put(-1, "Hello");
    mIntegerString.put(0, "Thrift");
    mIntegerString.put(1, "Team");
    Map<String, String> mStringString = new HashMap<String, String>();
    mStringString.put("Team", "Hello");
    mStringString.put("Thrift", "Thrift");
    mStringString.put("Batman", "Thanos");
    mapKeyMapValMap.put(mIntegerString, mStringString);

    MySimpleUnion mySimpleUnion = MySimpleUnion.fromCaseOne(idValueOne);

    org.apache.thrift.transport.TMemoryBuffer buf =
        new org.apache.thrift.transport.TMemoryBuffer(0);
    org.apache.thrift.protocol.TProtocolFactory factory =
        new org.apache.thrift.protocol.TCompactProtocol.Factory();

    org.apache.thrift.protocol.TProtocol apacheProto = factory.getProtocol(buf);
    ComplexNestedStruct complexNestedStruct =
        new ComplexNestedStruct(
            setOfSetOfInt,
            listOfListOfEnum,
            listOfListOfMyStruct,
            setOfListOfListOfString,
            mapKeyIntValListOfListOfEnum,
            mapKeyMapValMap,
            mySimpleUnion);

    complexNestedStruct.write0(apacheProto);
    ComplexNestedStruct structCreatedFromRead = ComplexNestedStruct.read0(apacheProto);

    assertThat(structCreatedFromRead.getSetOfSetOfInt(), equalTo(setOfSetOfInt));
    assertThat(
        structCreatedFromRead.getSetOfListOfListOfString(), equalTo(setOfListOfListOfString));
    assertThat(structCreatedFromRead.getMapKeyMapValMap(), equalTo(mapKeyMapValMap));
    assertThat(structCreatedFromRead.getMyUnion().getCaseOne(), equalTo(idValueOne));
    // First list of listOfListOfMyStruct
    assertThat(
        structCreatedFromRead.getListOfListOfMyStruct().get(0).get(0).getId(), equalTo(idValueOne));
    assertThat(
        structCreatedFromRead.getListOfListOfMyStruct().get(0).get(0).getName(),
        equalTo(nameValueOne));
    // Second list of listOfListOfMyStruct
    assertThat(
        structCreatedFromRead.getListOfListOfMyStruct().get(1).get(0).getId(), equalTo(idValueTwo));
    assertThat(
        structCreatedFromRead.getListOfListOfMyStruct().get(1).get(0).getName(),
        equalTo(nameValueTwo));

    // red=1, blue=2, green=3
    assertThat(structCreatedFromRead.getListOfListOfEnum().get(0).get(0).getValue(), equalTo(1));
    assertThat(structCreatedFromRead.getListOfListOfEnum().get(0).get(1).getValue(), equalTo(2));
    assertThat(structCreatedFromRead.getListOfListOfEnum().get(1).get(0).getValue(), equalTo(3));
    assertThat(
        structCreatedFromRead.getMapKeyIntValListOfListOfEnum().get(101).get(0).get(0).getValue(),
        equalTo(1));
    assertThat(
        structCreatedFromRead.getMapKeyIntValListOfListOfEnum().get(101).get(0).get(1).getValue(),
        equalTo(2));
    assertThat(
        structCreatedFromRead.getMapKeyIntValListOfListOfEnum().get(101).get(1).get(0).getValue(),
        equalTo(3));
    assertThat(
        structCreatedFromRead.getMapKeyIntValListOfListOfEnum().get(100).get(0).get(0).getValue(),
        equalTo(3));
    assertThat(
        structCreatedFromRead.getMapKeyIntValListOfListOfEnum().get(100).get(0).get(1).getValue(),
        equalTo(3));
  }

  @Test
  public void testReadEqualWriteCollectionStructs() throws Exception {
    List<Double> lDouble = new ArrayList<Double>();
    lDouble.add(5555555.5555);
    lDouble.add(0.0);
    lDouble.add(-5555555.5555);
    List<Short> lShort = new ArrayList<Short>();
    lShort.add((short) 111);
    lShort.add((short) 0);
    lShort.add((short) -111);
    Map<Integer, String> mIntegerString = new HashMap<Integer, String>();
    mIntegerString.put(-1, "Hello");
    mIntegerString.put(0, "Thrift");
    mIntegerString.put(1, "Team");
    Map<String, String> mStringString = new HashMap<String, String>();
    mStringString.put("Team", "Hello");
    mStringString.put("Thrift", "Thrift");
    mStringString.put("Hello", "Team");
    Set<Long> sLong = new HashSet<Long>();
    sLong.add(4444444444444444444L);
    sLong.add(0L);
    sLong.add(-4444444444444444444L);

    org.apache.thrift.transport.TMemoryBuffer buf =
        new org.apache.thrift.transport.TMemoryBuffer(0);
    org.apache.thrift.protocol.TProtocolFactory factory =
        new org.apache.thrift.protocol.TCompactProtocol.Factory();

    org.apache.thrift.protocol.TProtocol apacheProto = factory.getProtocol(buf);
    SimpleCollectionStruct exampleStruct =
        new SimpleCollectionStruct(lDouble, lShort, mIntegerString, mStringString, sLong);
    exampleStruct.write0(apacheProto);

    SimpleCollectionStruct structCreatedFromRead = SimpleCollectionStruct.read0(apacheProto);
    assertThat(structCreatedFromRead.getLDouble(), equalTo(lDouble));
    assertThat(structCreatedFromRead.getLShort(), equalTo(lShort));
    assertThat(structCreatedFromRead.getMIntegerString(), equalTo(mIntegerString));
    assertThat(structCreatedFromRead.getSLong(), equalTo(sLong));
  }

  @Test
  public void testReadEqualWriteOptionalFields() throws Exception {
    Map<Integer, String> mIntegerString = new HashMap<Integer, String>();
    mIntegerString.put(-1, "Hello");
    mIntegerString.put(0, "Thrift");
    mIntegerString.put(1, "Team");

    long idValue = 4444444444444444444L;
    String nameValue = "Hello Thrift Team";
    MySimpleStruct structOne = new MySimpleStruct(idValue, nameValue);

    org.apache.thrift.transport.TMemoryBuffer buf =
        new org.apache.thrift.transport.TMemoryBuffer(0);
    org.apache.thrift.protocol.TProtocolFactory factory =
        new org.apache.thrift.protocol.TCompactProtocol.Factory();

    org.apache.thrift.protocol.TProtocol apacheProto = factory.getProtocol(buf);
    MyOptioalStruct myOptioalStruct =
        new MyOptioalStruct.Builder()
            .setId(idValue)
            .setName("Steven")
            .setAgeShort((short) 15)
            .setAgeShortOptional((short) 15)
            .setAgeLong(44L)
            .setAgeLongOptional(44L)
            .setMySimpleStruct(structOne)
            .setMIntegerString(mIntegerString)
            .build();

    myOptioalStruct.write0(apacheProto);
    MyOptioalStruct structCreatedFromRead = MyOptioalStruct.read0(apacheProto);

    assertThat(structCreatedFromRead.getAgeLong(), equalTo(44L));
    assertThat(structCreatedFromRead.getAgeLongOptional(), equalTo(44L));
    assertThat(structCreatedFromRead.getAgeLongOptional().getClass().isPrimitive(), is(false));
    assertThat(structCreatedFromRead.getAgeShort(), equalTo((short) 15));
    assertThat(structCreatedFromRead.getAgeShortOptional(), equalTo((short) 15));
    assertThat(structCreatedFromRead.getAgeShortOptional().getClass().isPrimitive(), is(false));
    assertThat(structCreatedFromRead.getMIntegerStringOptional(), equalTo(null));
    assertThat(structCreatedFromRead.getSmallEnumOptional(), equalTo(null));
    assertThat(structCreatedFromRead.getMySmallEnum().getValue(), equalTo(0));
  }

  @Test
  public void testSwiftTypeAnnontationsFields() throws Exception {
    long idValue = 4444444444444444444L;
    String nameValue = "Hello Thrift Team";
    Long2ObjectArrayMap<String> longStrMap = new Long2ObjectArrayMap<String>();
    longStrMap.put(idValue, nameValue);
    Int2LongArrayMap intLongMap = new Int2LongArrayMap();
    intLongMap.put(1, 1L);
    intLongMap.put(100, 100L);
    Int2ObjectArrayMap<Int2LongArrayMap> ioMap = new Int2ObjectArrayMap<Int2LongArrayMap>();
    ioMap.put(123, intLongMap);
    List<Int2LongArrayMap> listOfFMap = new ArrayList<Int2LongArrayMap>();
    listOfFMap.add(intLongMap);

    int capacity = 4;
    ByteBuffer byteBuffer = ByteBuffer.allocate(capacity);
    byteBuffer.put((byte) 20);
    byteBuffer.put((byte) 30);
    byteBuffer.put((byte) 40);
    byteBuffer.put((byte) 50);

    TypeRemapped typeRemapped =
        new TypeRemapped.Builder()
            .setLsMap(longStrMap)
            .setIoMap(ioMap)
            .setMyListOfFMaps(listOfFMap)
            .setMyListOfFMaps(listOfFMap)
            .setByteBufferForBinary(byteBuffer)
            .build();

    org.apache.thrift.transport.TMemoryBuffer buf =
        new org.apache.thrift.transport.TMemoryBuffer(0);
    org.apache.thrift.protocol.TProtocolFactory factory =
        new org.apache.thrift.protocol.TCompactProtocol.Factory();
    org.apache.thrift.protocol.TProtocol apacheProto = factory.getProtocol(buf);
    typeRemapped.write0(apacheProto);
    TypeRemapped typeRemappedRead = TypeRemapped.read0(apacheProto);

    assertThat(typeRemappedRead.getLsMap(), equalTo(longStrMap));
    assertThat(typeRemappedRead.getLsMap().getClass(), equalTo(Long2ObjectArrayMap.class));
    assertThat(typeRemappedRead.getIoMap(), equalTo(ioMap));
    assertThat(typeRemappedRead.getIoMap().getClass(), equalTo(Int2ObjectArrayMap.class));
    assertThat(typeRemappedRead.getMyListOfFMaps(), equalTo(listOfFMap));
    assertThat(typeRemappedRead.getMyListOfFMaps().getClass(), equalTo(ArrayList.class));
    assertThat(typeRemappedRead.getByteBufferForBinary(), equalTo(byteBuffer));
    assertThat(
        ByteBuffer.class.isAssignableFrom(typeRemappedRead.getByteBufferForBinary().getClass()),
        equalTo(true));
  }
}
