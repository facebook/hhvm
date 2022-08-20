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

package com.facebook.thrift.protocol;

import com.facebook.thrift.TException;
import com.facebook.thrift.java.test.BoolStruct;
import com.facebook.thrift.java.test.EveryLayout;
import com.facebook.thrift.java.test.Nesting;
import com.facebook.thrift.java.test.SimpleStructTypes;
import com.facebook.thrift.transport.TIOStreamTransport;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSet;
import com.google.common.collect.Lists;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;
import org.junit.Assert;
import org.junit.Test;

public class TSimpleJSONProtocolTest {

  private BoolStruct deserializeBoolStruct(String json) throws Exception {
    ByteArrayInputStream bis = new ByteArrayInputStream(json.getBytes(StandardCharsets.UTF_8));
    TIOStreamTransport transport1 = new TIOStreamTransport(bis);
    TSimpleJSONProtocol protocol2 = new TSimpleJSONProtocol(transport1);
    BoolStruct struct = new BoolStruct();
    struct.read(protocol2);
    return struct;
  }

  @Test(expected = TException.class)
  public void testInvalidFalse() throws Exception {
    deserializeBoolStruct("{\"aBool\":felse}");
  }

  @Test(expected = TException.class)
  public void testInvalidLongFalse() throws Exception {
    deserializeBoolStruct("{\"aBool\":FalseASDF}");
  }

  @Test(expected = TException.class)
  public void testInvalidTrue() throws Exception {
    deserializeBoolStruct("{\"aBool\":Treu}");
  }

  @Test(expected = TException.class)
  public void testInvalidLongTrue() throws Exception {
    deserializeBoolStruct("{\"aBool\":TrueASDF}");
  }

  @Test
  public void testQuotedTrue() throws Exception {
    BoolStruct read = deserializeBoolStruct("{\"aBool\":\"true\"}");
    Assert.assertEquals(read.isABool(), true);
  }

  @Test
  public void testQuotedFalse() throws Exception {
    BoolStruct read = deserializeBoolStruct("{\"aBool\":\"false\"}");
    Assert.assertEquals(read.isABool(), false);
  }

  @Test
  public void testFalse() throws Exception {
    BoolStruct boolStruct = new BoolStruct.Builder().setABool(false).build();

    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    TIOStreamTransport transport = new TIOStreamTransport(bos);

    TSimpleJSONProtocol protocol1 = new TSimpleJSONProtocol(transport);
    boolStruct.write(protocol1);
    ByteArrayInputStream bis = new ByteArrayInputStream(bos.toByteArray());
    TIOStreamTransport transport1 = new TIOStreamTransport(bis);
    TSimpleJSONProtocol protocol2 = new TSimpleJSONProtocol(transport1);
    BoolStruct read = new BoolStruct();
    read.read(protocol2);
    Assert.assertEquals(boolStruct, read);
  }

  @Test
  public void testTrue() throws Exception {
    BoolStruct boolStruct = new BoolStruct.Builder().setABool(true).build();

    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    TIOStreamTransport transport = new TIOStreamTransport(bos);

    TSimpleJSONProtocol protocol1 = new TSimpleJSONProtocol(transport);
    boolStruct.write(protocol1);

    ByteArrayInputStream bis = new ByteArrayInputStream(bos.toByteArray());
    TIOStreamTransport transport1 = new TIOStreamTransport(bis);
    TSimpleJSONProtocol protocol2 = new TSimpleJSONProtocol(transport1);
    BoolStruct read = new BoolStruct();
    read.read(protocol2);
    Assert.assertEquals(boolStruct, read);
  }

  @Test
  public void testNested() throws Exception {
    EveryLayout everyLayout =
        new EveryLayout.Builder()
            .setABool(true)
            .setADouble(Double.MAX_VALUE)
            .setAFloat(Float.MAX_VALUE)
            .setAByte((byte) 1)
            .setAString("I am a string")
            .setAInt(Integer.MAX_VALUE)
            .setAShort(Short.MAX_VALUE)
            .setALong(Long.MAX_VALUE)
            .setAList(Lists.newArrayList("1", "2", "3"))
            .setAMap(ImmutableMap.of(1, "value", 2, "value"))
            .setASet(ImmutableSet.of("1", "2", "3"))
            .setAListOfLists(
                Lists.newArrayList(
                    Lists.newArrayList("1", "2", "3"),
                    Lists.newArrayList("1", "2", "3"),
                    Lists.newArrayList("1", "2", "3")))
            .setASetOfSets(
                ImmutableSet.of(
                    ImmutableSet.of("1", "2", "3"),
                    ImmutableSet.of("4", "5", "6"),
                    ImmutableSet.of("7", "8", "9")))
            .setAMapOfLists(
                ImmutableMap.of(
                    1, Lists.newArrayList(11, 12, 13), 2, Lists.newArrayList(21, 22, 23)))
            .setListOfSetsOfMap(
                Lists.newArrayList(
                    ImmutableSet.of(
                        ImmutableMap.of(true, "TRUE", false, "FALSE"),
                        ImmutableMap.of(true, "VRAI", false, "FAUX")),
                    ImmutableSet.of(ImmutableMap.of(true, "TRUE0", false, "FALSE0"))))
            .setBlob("XXXX".getBytes(StandardCharsets.UTF_8))
            .build();

    Nesting nesting =
        new Nesting.Builder()
            .setStructMap(ImmutableMap.of("1", everyLayout, "2", everyLayout))
            .setBoolMap(ImmutableMap.of("TRUE", true, "FALSE", false))
            .setEvery(everyLayout)
            .build();

    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    TIOStreamTransport transport = new TIOStreamTransport(bos);

    TSimpleJSONProtocol protocol1 = new TSimpleJSONProtocol(transport);
    nesting.write(protocol1);

    ByteArrayInputStream bis = new ByteArrayInputStream(bos.toByteArray());
    TIOStreamTransport transport1 = new TIOStreamTransport(bis);
    TSimpleJSONProtocol protocol2 = new TSimpleJSONProtocol(transport1);
    Nesting read = new Nesting();
    read.read(protocol2);
    Assert.assertEquals(nesting, read);
  }

  @Test
  public void testEveryLayout() throws Exception {
    EveryLayout everyLayout =
        new EveryLayout.Builder()
            .setABool(true)
            .setADouble(Double.MAX_VALUE)
            .setAFloat(Float.MAX_VALUE)
            .setAByte((byte) 1)
            .setAString("I am a string")
            .setAInt(Integer.MAX_VALUE)
            .setAShort(Short.MAX_VALUE)
            .setALong(Long.MAX_VALUE)
            .setAList(Lists.newArrayList("1", "2", "3"))
            .setAMap(ImmutableMap.of(1, "value", 2, "value"))
            .setASet(ImmutableSet.of("1", "2", "3"))
            .setAListOfLists(
                Lists.newArrayList(
                    Lists.newArrayList("1", "2", "3"),
                    Lists.newArrayList("1", "2", "3"),
                    Lists.newArrayList("1", "2", "3")))
            .setASetOfSets(
                ImmutableSet.of(
                    ImmutableSet.of("1", "2", "3"),
                    ImmutableSet.of("4", "5", "6"),
                    ImmutableSet.of("7", "8", "9")))
            .setAMapOfLists(ImmutableMap.of())
            .setBlob("XXXX".getBytes(StandardCharsets.UTF_8))
            .build();

    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    TIOStreamTransport transport = new TIOStreamTransport(bos);

    TSimpleJSONProtocol protocol1 = new TSimpleJSONProtocol(transport);
    everyLayout.write(protocol1);

    ByteArrayInputStream bis = new ByteArrayInputStream(bos.toByteArray());
    TIOStreamTransport transport1 = new TIOStreamTransport(bis);
    TSimpleJSONProtocol protocol2 = new TSimpleJSONProtocol(transport1);
    EveryLayout read = new EveryLayout();
    read.read(protocol2);
    Assert.assertEquals(everyLayout, read);
  }

  @Test(expected = UnsupportedOperationException.class)
  public void testAndroidEveryLayout() throws Exception {
    com.facebook.thrift.android.test.EveryLayout everyLayout =
        new com.facebook.thrift.android.test.EveryLayout.Builder()
            .setABool(true)
            .setADouble(Double.MAX_VALUE)
            .setAFloat(Float.MAX_VALUE)
            .setAByte((byte) 1)
            .setAString("I am a string")
            .setAInt(Integer.MAX_VALUE)
            .setAShort(Short.MAX_VALUE)
            .setALong(Long.MAX_VALUE)
            .setAList(Lists.newArrayList("1", "2", "3"))
            .setAMap(ImmutableMap.of(1, "value", 2, "value"))
            .setASet(ImmutableSet.of("1", "2", "3"))
            .setAListOfLists(
                Lists.newArrayList(
                    Lists.newArrayList("1", "2", "3"),
                    Lists.newArrayList("1", "2", "3"),
                    Lists.newArrayList("1", "2", "3")))
            .setASetOfSets(
                ImmutableSet.of(
                    ImmutableSet.of("1", "2", "3"),
                    ImmutableSet.of("4", "5", "6"),
                    ImmutableSet.of("7", "8", "9")))
            .setAMapOfLists(ImmutableMap.of())
            .setBlob("XXXX".getBytes(StandardCharsets.UTF_8))
            .build();

    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    TIOStreamTransport transport = new TIOStreamTransport(bos);

    TSimpleJSONProtocol protocol1 = new TSimpleJSONProtocol(transport);
    everyLayout.write(protocol1);

    ByteArrayInputStream bis = new ByteArrayInputStream(bos.toByteArray());
    TIOStreamTransport transport1 = new TIOStreamTransport(bis);
    TSimpleJSONProtocol protocol2 = new TSimpleJSONProtocol(transport1);

    // Android codegen doesn't generate metadata for code size reasons
    com.facebook.thrift.android.test.EveryLayout.deserialize(protocol2);
  }

  @Test
  public void testEveryLayoutWithBase64() throws Exception {
    EveryLayout everyLayout =
        new EveryLayout.Builder()
            .setABool(true)
            .setADouble(Double.MAX_VALUE)
            .setAFloat(Float.MAX_VALUE)
            .setAByte((byte) 1)
            .setAString("I am a string")
            .setAInt(Integer.MAX_VALUE)
            .setAShort(Short.MAX_VALUE)
            .setALong(Long.MAX_VALUE)
            .setAList(Lists.newArrayList("1", "2", "3"))
            .setAMap(ImmutableMap.of(1, "value", 2, "value"))
            .setASet(ImmutableSet.of("1", "2", "3"))
            .setAListOfLists(
                Lists.newArrayList(
                    Lists.newArrayList("1", "2", "3"),
                    Lists.newArrayList("1", "2", "3"),
                    Lists.newArrayList("1", "2", "3")))
            .setASetOfSets(
                ImmutableSet.of(
                    ImmutableSet.of("1", "2", "3"),
                    ImmutableSet.of("4", "5", "6"),
                    ImmutableSet.of("7", "8", "9")))
            .setAMapOfLists(ImmutableMap.of())
            .setBlob("XXXX".getBytes(StandardCharsets.UTF_8))
            .build();

    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    TIOStreamTransport transport = new TIOStreamTransport(bos);

    TSimpleJSONProtocol protocol1 = new TSimpleJSONProtocol(transport, true);
    everyLayout.write(protocol1);

    ByteArrayInputStream bis = new ByteArrayInputStream(bos.toByteArray());
    TIOStreamTransport transport1 = new TIOStreamTransport(bis);
    TSimpleJSONProtocol protocol2 = new TSimpleJSONProtocol(transport1, true);
    EveryLayout read0 = new EveryLayout();
    read0.read(protocol2);
    Assert.assertEquals(everyLayout, read0);
  }

  @Test
  public void testWithEmptyCollections() throws Exception {
    EveryLayout everyLayout =
        new EveryLayout.Builder()
            .setABool(true)
            .setADouble(Double.MAX_VALUE)
            .setAFloat(Float.MAX_VALUE)
            .setAByte((byte) 1)
            .setAString("I am a string")
            .setAInt(Integer.MAX_VALUE)
            .setAShort(Short.MAX_VALUE)
            .setALong(Long.MAX_VALUE)
            .setAList(Lists.newArrayList())
            .setAMap(ImmutableMap.of())
            .setASet(ImmutableSet.of())
            .setAListOfLists(Lists.newArrayList())
            .setASetOfSets(ImmutableSet.of())
            .setAMapOfLists(ImmutableMap.of())
            .setBlob("XXXX".getBytes(StandardCharsets.UTF_8))
            .build();

    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    TIOStreamTransport transport = new TIOStreamTransport(bos);

    TSimpleJSONProtocol protocol1 = new TSimpleJSONProtocol(transport);
    everyLayout.write(protocol1);

    ByteArrayInputStream bis = new ByteArrayInputStream(bos.toByteArray());
    TIOStreamTransport transport1 = new TIOStreamTransport(bis);
    TSimpleJSONProtocol protocol2 = new TSimpleJSONProtocol(transport1);
    EveryLayout read = new EveryLayout();
    read.read(protocol2);
    Assert.assertEquals(everyLayout, read);
  }

  private static final String prettyPrinted =
      "{\n"
          + "  \"aBool\": true,\n"
          + "  \"aInt\": 2147483647,\n"
          + "  \"aLong\": 9223372036854775807,\n"
          + "  \"aString\": \"I am a string\",\n"
          + "  \"aDouble\": 1.7976931348623157e+308,\n"
          + "  \"aFloat\": 3.4028235e+38,\n"
          + "  \"aShort\": 32767,\n"
          + "  \"aByte\": 1,\n"
          + "  \"aList\": [],\n"
          + "  \"aSet\": [],\n"
          + "  \"aMap\": {},\n"
          + "  \"aListOfLists\": [],\n"
          + "  \"aSetOfSets\": [],\n"
          + "  \"aMapOfLists\": {},\n"
          + "  \"blob\": \"XXXX\"\n"
          + "}";

  @Test
  public void testWithPrettyPrintedJson() throws Exception {
    EveryLayout everyLayout =
        new EveryLayout.Builder()
            .setABool(true)
            .setADouble(Double.MAX_VALUE)
            .setAFloat(Float.MAX_VALUE)
            .setAByte((byte) 1)
            .setAString("I am a string")
            .setAInt(Integer.MAX_VALUE)
            .setAShort(Short.MAX_VALUE)
            .setALong(Long.MAX_VALUE)
            .setAList(Lists.newArrayList())
            .setAMap(ImmutableMap.of())
            .setASet(ImmutableSet.of())
            .setAListOfLists(Lists.newArrayList())
            .setASetOfSets(ImmutableSet.of())
            .setAMapOfLists(ImmutableMap.of())
            .setBlob("XXXX".getBytes(StandardCharsets.UTF_8))
            .build();

    ByteArrayInputStream bis =
        new ByteArrayInputStream(prettyPrinted.getBytes(StandardCharsets.UTF_8));
    TIOStreamTransport transport1 = new TIOStreamTransport(bis);
    TSimpleJSONProtocol protocol2 = new TSimpleJSONProtocol(transport1);
    EveryLayout read = new EveryLayout();
    read.read(protocol2);
    Assert.assertEquals(everyLayout, read);
  }

  private static final String prettyPrintedWithCollections =
      "{\n"
          + "  \"aBool\": true,\n"
          + "  \"aInt\": 2147483647,\n"
          + "  \"aLong\": 9223372036854775807,\n"
          + "  \"aString\": \"I am a string\",\n"
          + "  \"aDouble\": 1.7976931348623157e+308,\n"
          + "  \"aFloat\": 3.4028235e+38,\n"
          + "  \"aShort\": 32767,\n"
          + "  \"aByte\": 1,\n"
          + "  \"aList\": [\n"
          + "    \"1\",\n"
          + "    \"2\",\n"
          + "    \"3\"\n"
          + "  ],\n"
          + "  \"aSet\": [\n"
          + "    \"1\",\n"
          + "    \"2\",\n"
          + "    \"3\"\n"
          + "  ],\n"
          + "  \"aMap\": {\n"
          + "    \"1\": \"value\",\n"
          + "    \"2\": \"value\"\n"
          + "  },\n"
          + "  \"aListOfLists\": [\n"
          + "    [\n"
          + "      \"1\",\n"
          + "      \"2\",\n"
          + "      \"3\"\n"
          + "    ],\n"
          + "    [\n"
          + "      \"1\",\n"
          + "      \"2\",\n"
          + "      \"3\"\n"
          + "    ],\n"
          + "    [\n"
          + "      \"1\",\n"
          + "      \"2\",\n"
          + "      \"3\"\n"
          + "    ]\n"
          + "  ],\n"
          + "  \"aSetOfSets\": [\n"
          + "    [\n"
          + "      \"1\",\n"
          + "      \"2\",\n"
          + "      \"3\"\n"
          + "    ],\n"
          + "    [\n"
          + "      \"4\",\n"
          + "      \"5\",\n"
          + "      \"6\"\n"
          + "    ],\n"
          + "    [\n"
          + "      \"7\",\n"
          + "      \"8\",\n"
          + "      \"9\"\n"
          + "    ]\n"
          + "  ],\n"
          + "  \"aMapOfLists\": {},\n"
          + "  \"blob\": \"XXXX\"\n"
          + "}";

  @Test
  public void testPrettyPrintWithCollections() throws Exception {
    EveryLayout everyLayout =
        new EveryLayout.Builder()
            .setABool(true)
            .setADouble(Double.MAX_VALUE)
            .setAFloat(Float.MAX_VALUE)
            .setAByte((byte) 1)
            .setAString("I am a string")
            .setAInt(Integer.MAX_VALUE)
            .setAShort(Short.MAX_VALUE)
            .setALong(Long.MAX_VALUE)
            .setAList(Lists.newArrayList("1", "2", "3"))
            .setAMap(ImmutableMap.of(1, "value", 2, "value"))
            .setASet(ImmutableSet.of("1", "2", "3"))
            .setAListOfLists(
                Lists.newArrayList(
                    Lists.newArrayList("1", "2", "3"),
                    Lists.newArrayList("1", "2", "3"),
                    Lists.newArrayList("1", "2", "3")))
            .setASetOfSets(
                ImmutableSet.of(
                    ImmutableSet.of("1", "2", "3"),
                    ImmutableSet.of("4", "5", "6"),
                    ImmutableSet.of("7", "8", "9")))
            .setAMapOfLists(ImmutableMap.of())
            .setBlob("XXXX".getBytes(StandardCharsets.UTF_8))
            .build();

    ByteArrayInputStream bis =
        new ByteArrayInputStream(prettyPrintedWithCollections.getBytes(StandardCharsets.UTF_8));
    TIOStreamTransport transport1 = new TIOStreamTransport(bis);
    TSimpleJSONProtocol protocol2 = new TSimpleJSONProtocol(transport1);
    EveryLayout read = new EveryLayout();
    read.read(protocol2);
    Assert.assertEquals(everyLayout, read);
  }

  @Test
  public void testMissingBool() throws Exception {
    BoolStruct read = deserializeBoolStruct("{}");
    Assert.assertEquals(read.isABool(), false);
  }

  @Test
  public void testMissingEveryLayout() throws Exception {
    EveryLayout everyLayout = new EveryLayout.Builder().build();

    ByteArrayInputStream bis = new ByteArrayInputStream("{}".getBytes(StandardCharsets.UTF_8));
    TIOStreamTransport transport = new TIOStreamTransport(bis);
    TSimpleJSONProtocol protocol = new TSimpleJSONProtocol(transport);
    EveryLayout read = new EveryLayout();
    read.read(protocol);
    Assert.assertEquals(everyLayout, read);
  }

  @Test
  public void testMissingSimpleStructTypes() throws Exception {
    SimpleStructTypes simpleStructTypes = new SimpleStructTypes.Builder().build();

    ByteArrayInputStream bis = new ByteArrayInputStream("{}".getBytes(StandardCharsets.UTF_8));
    TIOStreamTransport transport = new TIOStreamTransport(bis);
    TSimpleJSONProtocol protocol = new TSimpleJSONProtocol(transport);
    SimpleStructTypes read = new SimpleStructTypes();
    read.read(protocol);
    Assert.assertEquals(-9999, read.getJ());
    Assert.assertEquals(97, read.getY());
  }

  @Test
  public void testExtraField() throws Exception {
    testUnknown("{\"unknown_field\":1337}");
    testUnknown("{\"unknown_field\":1337, \"other_field\": 3.1415}");
    testUnknown("{\"unknown_field\": 1337}");
    testUnknown("{\"unknown_field\" : 1337}");
    testUnknown("{\"unknown_field\" : 1337}, \"other_field\" : 3.14.15");
    testUnknown("{\"unknown_field\" :1337}");
    testUnknown("{\"unknown_field\":\"1337\"}");
    testUnknown("{\"unknown_field\": \"1337\"}");
    testUnknown("{\"unknown_field\" :[ \"1337\"]}");
    testUnknown("{\"unknown_field\" : [ \"1337\"]}");
    testUnknown("{\"unknown_nested\":{\"key\":1337}}");
    testUnknown("{\"unknown_nested\":{\"key\":1337}}, \"other_field\": 3.1415}");
    testUnknown("{\"unknown_nested\": {\"key\": 1337}}");
    testUnknown("{\"unknown_nested\" : { \"key\" : 1337 } , \"other_field\" : 3.1415 } ");
    testUnknown("{\"unknown_nested\": [{\"key\": 1337}]}");
  }

  private void testUnknown(String json) {
    EveryLayout everyLayout = new EveryLayout.Builder().build();

    ByteArrayInputStream bis = new ByteArrayInputStream(json.getBytes(StandardCharsets.UTF_8));
    TIOStreamTransport transport = new TIOStreamTransport(bis);
    TSimpleJSONProtocol protocol = new TSimpleJSONProtocol(transport);
    EveryLayout read = new EveryLayout();
    read.read(protocol);
    Assert.assertEquals(everyLayout, read);
  }
}
