/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman.bser;

import static org.hamcrest.Matchers.closeTo;
import static org.hamcrest.Matchers.contains;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.nullValue;
import static org.hamcrest.Matchers.sameInstance;
import static org.junit.Assert.assertThat;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.common.io.BaseEncoding;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.IOException;

import java.nio.ByteOrder;
import java.nio.charset.CharacterCodingException;

import java.util.AbstractMap.SimpleImmutableEntry;
import java.util.List;
import java.util.Map;

import com.facebook.watchman.bser.BserDeserializer;

import org.hamcrest.Matchers;

import org.junit.rules.ExpectedException;
import org.junit.Rule;
import org.junit.Test;

@SuppressWarnings("unchecked")
public class BserDeserializerTest {
  @Rule
  public ExpectedException thrown = ExpectedException.none();

  private static final String SHORT_11FF;
  private static final String INT_1122EEFF;
  private static final String LONG_0000000080000000;
  private static final String LONG_11223344CCDDEEFF;
  private static final String REAL_0DOT123456789;
  private static final Map.Entry<String, Object> FOO_MAP_ENTRY =
      new SimpleImmutableEntry<String, Object>("foo", (byte) 0x23);
  private static final Map.Entry<String, Object> BAR_MAP_ENTRY =
      new SimpleImmutableEntry<String, Object>("bar", (byte) 0x42);
  private static final Map.Entry<String, Object> BAZ_MAP_ENTRY =
      new SimpleImmutableEntry<String, Object>("baz", (byte) 0xF0);

  static {
    if (ByteOrder.nativeOrder() == ByteOrder.BIG_ENDIAN) {
      SHORT_11FF = "11FF";
      INT_1122EEFF = "1122EEFF";
      LONG_0000000080000000 = "0000000080000000";
      LONG_11223344CCDDEEFF = "11223344CCDDEEFF";
      REAL_0DOT123456789 = "3FBF9ADD3739635F";
    } else {
      SHORT_11FF = "FF11";
      INT_1122EEFF = "FFEE2211";
      LONG_0000000080000000 = "0000008000000000";
      LONG_11223344CCDDEEFF = "FFEEDDCC44332211";
      REAL_0DOT123456789 = "5F633937DD9ABF3F";
    }
  }

  private static InputStream getByteStream(String base16) {
    return new ByteArrayInputStream(BaseEncoding.base16().decode(base16));
  }

  @Test
  public void deserializeEmptyArray() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    List<Object> deserialized = (List<Object>) deserializer.deserializeBserValue(
        getByteStream("00010303000300"));
    List<Object> expected = ImmutableList.of();
    assertThat(deserialized, equalTo(expected));
  }

  @Test
  public void deserializeEmptyArrayTwiceReturnsSameArray() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    List<Object> deserialized = (List<Object>) deserializer.deserializeBserValue(
        getByteStream("00010303000300"));
    List<Object> deserialized2 = (List<Object>) deserializer.deserializeBserValue(
        getByteStream("00010303000300"));
    assertThat(deserialized, is(sameInstance(deserialized2)));
  }

  @Test
  public void deserializeArrayOfInt8() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    List<Object> deserialized = (List<Object>) deserializer.deserializeBserValue(
        getByteStream("000103090003030323034203F0"));
    List<Object> expected = ImmutableList.<Object>of((byte) 0x23, (byte) 0x42, (byte) 0xF0);
    assertThat(deserialized, equalTo(expected));
  }

  @Test
  public void deserializeString() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    String deserialized = (String) deserializer.deserializeBserValue(
        getByteStream("0001030E02030B68656C6C6F20776F726C64"));
    String expected = "hello world";
    assertThat(deserialized, equalTo(expected));
  }

  @Test
  public void sameStringDeserializedTwiceReturnsSameInstance() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    String deserialized = (String) deserializer.deserializeBserValue(
        getByteStream("0001030E02030B68656C6C6F20776F726C64"));
    String deserialized2 = (String) deserializer.deserializeBserValue(
        getByteStream("0001030E02030B68656C6C6F20776F726C64"));
    assertThat(deserialized, is(sameInstance(deserialized2)));
  }

  @Test
  public void deserializeEmptyMap() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    Map<String, Object> deserialized = (Map<String, Object>) deserializer.deserializeBserValue(
        getByteStream("00010303010300"));
    Map<String, Object> expected = ImmutableMap.of();
    assertThat(deserialized, equalTo(expected));
  }

  @Test
  public void deserializeEmptyMapTwiceReturnsSameMap() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    Map<String, Object> deserialized = (Map<String, Object>) deserializer.deserializeBserValue(
        getByteStream("00010303010300"));
    Map<String, Object> deserialized2 = (Map<String, Object>) deserializer.deserializeBserValue(
        getByteStream("00010303010300"));
    assertThat(deserialized, is(sameInstance(deserialized2)));
  }

  @Test
  public void deserializeUnsortedMapOfStringToInt8() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    Map<String, Object> deserialized = (Map<String, Object>) deserializer.deserializeBserValue(
        getByteStream("0001031B010303020303666F6F0323020303626172034202030362617A03F0"));
    // Make sure the result contains these entries in the order they appeared in the input.
    assertThat(
        deserialized.entrySet(),
        contains(FOO_MAP_ENTRY, BAR_MAP_ENTRY, BAZ_MAP_ENTRY));
  }

  @Test
  public void deserializeSortedMapOfStringToInt8() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.SORTED);
    Map<String, Object> deserialized = (Map<String, Object>) deserializer.deserializeBserValue(
        getByteStream("0001031B010303020303666F6F0323020303626172034202030362617A03F0"));
    // Make sure the result contains these entries in sorted order.
    assertThat(
        deserialized.entrySet(),
        contains(BAR_MAP_ENTRY, BAZ_MAP_ENTRY, FOO_MAP_ENTRY));
  }

  @Test
  public void deserializeTemplate() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    List<Map<String, Object>> deserialized = (List<Map<String, Object>>)
        deserializer.deserializeBserValue(
            getByteStream(
                "000103280B0003020203046E616D6502030361676503030203046672656403140203" +
                "0470657465031E0C0319"));

    // We have to cast to byte because otherwise Java coerces the ages to Integer
    // objects which are sadly not equal to the BSER-deserialized Byte objects with the
    // same value.
    assertThat(
        deserialized,
        Matchers.<Map<String, Object>>contains(
            Matchers.<Map<String, Object>>allOf(
                Matchers.<String, Object>hasEntry("name", "fred"),
                Matchers.<String, Object>hasEntry("age", (byte) 20)),
            Matchers.<Map<String, Object>>allOf(
                Matchers.<String, Object>hasEntry("name", "pete"),
                Matchers.<String, Object>hasEntry("age", (byte) 30)),
            Matchers.<Map<String, Object>>allOf(
                Matchers.<String, Object>hasEntry("age", (byte) 25))));
  }

  @Test
  public void deserializeInt8() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    Byte deserialized = (Byte) deserializer.deserializeBserValue(getByteStream("000103020342"));
    assertThat(deserialized, equalTo((byte) 0x42));
  }

  @Test
  public void sameInt8DeserializedTwiceReturnsSameInstance() throws IOException {
    // Java actually interns small integer values for us. How nice!
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    Byte deserialized = (Byte) deserializer.deserializeBserValue(getByteStream("000103020342"));
    Byte deserialized2 = (Byte) deserializer.deserializeBserValue(getByteStream("000103020342"));
    assertThat(deserialized, is(sameInstance(deserialized2)));
  }

  @Test
  public void deserializeInt16() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    Short deserialized = (Short) deserializer.deserializeBserValue(
        getByteStream("0001030304" + SHORT_11FF));
    assertThat(deserialized, equalTo((short) 0x11FF));
  }

  @Test
  public void deserializeInt32() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    Integer deserialized = (Integer) deserializer.deserializeBserValue(
        getByteStream("0001030505" + INT_1122EEFF));
    assertThat(deserialized, equalTo(0x1122EEFF));
  }

  @Test
  public void deserializeInt64() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    Long deserialized = (Long) deserializer.deserializeBserValue(
        getByteStream("0001030906" + LONG_11223344CCDDEEFF));
    assertThat(deserialized, equalTo(0x11223344CCDDEEFFL));
  }

  @Test
  public void deserializeReal() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    Double deserialized = (Double) deserializer.deserializeBserValue(
        getByteStream("0001030907" + REAL_0DOT123456789));
    assertThat(deserialized, closeTo(0.123456789, 1e-6));
  }

  @Test
  public void deserializeTrue() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    Boolean deserialized = (Boolean) deserializer.deserializeBserValue(getByteStream("0001030108"));
    assertThat(deserialized, is(true));
  }

  @Test
  public void deserializeFalse() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    Boolean deserialized = (Boolean) deserializer.deserializeBserValue(getByteStream("0001030109"));
    assertThat(deserialized, is(false));
  }

  @Test
  public void deserializeNull() throws IOException {
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    Object deserialized = deserializer.deserializeBserValue(getByteStream("000103010A"));
    assertThat(deserialized, is(nullValue()));
  }

  @Test
  public void throwIfSniffLengthTooShort() throws IOException {
    thrown.expect(IOException.class);
    thrown.expectMessage("Invalid BSER header (expected 3 bytes, got 0 bytes)");
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    deserializer.deserializeBserValue(getByteStream(""));
  }

  @Test
  public void throwIfInvalidHeader() throws IOException {
    thrown.expect(IOException.class);
    thrown.expectMessage("Invalid BSER header");
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    deserializer.deserializeBserValue(getByteStream("000F03"));
  }

  @Test
  public void throwIfInvalidHeaderLengthType() throws IOException {
    thrown.expect(IOException.class);
    thrown.expectMessage("Unrecognized BSER header length type 7");
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    deserializer.deserializeBserValue(getByteStream("000107" + REAL_0DOT123456789));
  }

  @Test
  public void throwIfHeaderLengthIsNegative() throws IOException {
    thrown.expect(IOException.class);
    thrown.expectMessage("BSER length out of range (-128 < 0)");
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    deserializer.deserializeBserValue(getByteStream("00010380"));
  }

  @Test
  public void throwIfBodyLengthIsOverMaxInt() throws IOException {
    thrown.expect(IOException.class);
    thrown.expectMessage("BSER length out of range (2147483648 > 2147483647)");
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    deserializer.deserializeBserValue(getByteStream("000106" + LONG_0000000080000000));
  }

  @Test
  public void throwIfHeaderLengthTooShort() throws IOException {
    thrown.expect(IOException.class);
    thrown.expectMessage("Invalid BSER header length (expected 1 bytes, got 0 bytes)");
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    deserializer.deserializeBserValue(getByteStream("000103"));
  }

  @Test
  public void throwIfRemainingLengthTooShort() throws IOException {
    thrown.expect(IOException.class);
    thrown.expectMessage("Invalid BSER header (expected 1 bytes, got 0 bytes)");
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    deserializer.deserializeBserValue(getByteStream("00010301"));
  }

  @Test
  public void throwIfStringNotUTF8() throws IOException {
    thrown.expect(CharacterCodingException.class);
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    deserializer.deserializeBserValue(getByteStream("00010306020303ABCDEF"));
  }

  @Test
  public void throwIfArrayLengthTooShort() throws IOException {
    thrown.expect(BserDeserializer.BserEofException.class);
    thrown.expectMessage("Prematurely reached end of BSER buffer");
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    deserializer.deserializeBserValue(getByteStream("000103050003020323"));
  }

  @Test
  public void throwIfMapLengthTooShort() throws IOException {
    thrown.expect(BserDeserializer.BserEofException.class);
    thrown.expectMessage("Prematurely reached end of BSER buffer");
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    deserializer.deserializeBserValue(getByteStream("0001030B010303020303666F6F0323"));
  }

  @Test
  public void throwIfMapKeyNotString() throws IOException {
    thrown.expect(IOException.class);
    thrown.expectMessage("Unrecognized BSER object key type 3, expected string");
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    deserializer.deserializeBserValue(getByteStream("0001030701030103030323"));
  }
}
