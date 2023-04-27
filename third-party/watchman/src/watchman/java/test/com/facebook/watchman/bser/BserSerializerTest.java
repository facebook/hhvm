/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman.bser;

import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.not;
import static org.hamcrest.Matchers.sameInstance;
import static org.junit.Assert.assertThat;

import com.google.common.base.Strings;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.common.io.BaseEncoding;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.CharacterCodingException;

import com.facebook.watchman.bser.BserSerializer;

import org.junit.rules.ExpectedException;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;

public class BserSerializerTest {
  @Rule
  public ExpectedException thrown = ExpectedException.none();

  private ByteBuffer buffer;

  private static final String EXPECTED_EMPTY_ARRAY;
  private static final String EXPECTED_INT8_ARRAY;
  private static final String EXPECTED_STRING;
  private static final String EXPECTED_EMPTY_MAP;
  private static final String EXPECTED_INT8_MAP;
  private static final String EXPECTED_INT8;
  private static final String EXPECTED_INT16;
  private static final String EXPECTED_INT32;
  private static final String EXPECTED_INT64;
  private static final String EXPECTED_MIN_INT8;
  private static final String EXPECTED_MIN_INT16;
  private static final String EXPECTED_MIN_INT32;
  private static final String EXPECTED_MIN_INT64;
  private static final String EXPECTED_REAL;
  private static final String EXPECTED_TRUE;
  private static final String EXPECTED_FALSE;
  private static final String EXPECTED_NULL;

  // BSER encoding depends on the host byte order, and to keep the
  // encoder simpler, we always encode the buffer length as a
  // (endian-specific) 32-bit value at the start of the encoded byte
  // sequence.
  //
  // So, we initialize these expected byte sequences statically depending
  // on the host byte order.
  static {
    if (ByteOrder.nativeOrder() == ByteOrder.BIG_ENDIAN) {
      EXPECTED_EMPTY_ARRAY = "00010500000003000300";
      EXPECTED_INT8_ARRAY = "000105000000090003030323034203F0";
      EXPECTED_STRING = "0001050000000E02030B68656C6C6F20776F726C64";
      EXPECTED_EMPTY_MAP = "00010500000003010300";
      EXPECTED_INT8_MAP = "0001050000001B010303020303666F6F0323020303626172034202030362617A03F0";
      EXPECTED_INT8 = "000105000000020342";
      EXPECTED_INT16 = "000105000000030411FF";
      EXPECTED_INT32 = "0001050000005051122EEFF";
      EXPECTED_INT64 = "000105000000090611223344CCDDEEFF";
      EXPECTED_MIN_INT8 = "000105000000020380";
      EXPECTED_MIN_INT16 = "00010500000003048000";
      EXPECTED_MIN_INT32 = "00010500000050580000000";
      EXPECTED_MIN_INT64 = "00010500000009068000000000000000";
      EXPECTED_REAL = "00010500000009073FBF9ADD3739635F";
      EXPECTED_TRUE = "0001050000000108";
      EXPECTED_FALSE = "0001050000000109";
      EXPECTED_NULL = "000105000000010A";
    } else {
      EXPECTED_EMPTY_ARRAY = "00010503000000000300";
      EXPECTED_INT8_ARRAY = "000105090000000003030323034203F0";
      EXPECTED_STRING = "0001050E00000002030B68656C6C6F20776F726C64";
      EXPECTED_EMPTY_MAP = "00010503000000010300";
      EXPECTED_INT8_MAP = "0001051B000000010303020303666F6F0323020303626172034202030362617A03F0";
      EXPECTED_INT8 = "000105020000000342";
      EXPECTED_INT16 = "0001050300000004FF11";
      EXPECTED_INT32 = "0001050500000005FFEE2211";
      EXPECTED_INT64 = "0001050900000006FFEEDDCC44332211";
      EXPECTED_MIN_INT8 = "000105020000000380";
      EXPECTED_MIN_INT16 = "00010503000000040080";
      EXPECTED_MIN_INT32 = "000105050000000500000080";
      EXPECTED_MIN_INT64 = "00010509000000060000000000000080";
      EXPECTED_REAL = "00010509000000075F633937DD9ABF3F";
      EXPECTED_TRUE = "0001050100000008";
      EXPECTED_FALSE = "0001050100000009";
      EXPECTED_NULL = "000105010000000A";
    }
  }

  @Before
  public void setUp() {
    buffer = ByteBuffer.allocate(512).order(ByteOrder.nativeOrder());
  }

  private ByteBuffer assertEncodingMatches(Object object, String base16) throws IOException {
    BserSerializer serializer = new BserSerializer();
    ByteBuffer result = serializer.serializeToBuffer(object, buffer);
    result.flip();
    byte[] base16Array = BaseEncoding.base16().decode(base16);
    assertThat(
        String.format(
            "Encoded buffer mismatch (%s != %s)",
            base16,
            BaseEncoding.base16().encode(result.array(), result.position(), result.limit())),
        result,
        equalTo(ByteBuffer.wrap(base16Array)));
    return result;
  }

  @Test
  public void serializeEmptyArray() throws IOException {
    assertEncodingMatches(ImmutableList.of(), EXPECTED_EMPTY_ARRAY);
  }

  @Test
  public void serializeArrayOfInt8() throws IOException {
    assertEncodingMatches(
        ImmutableList.of((byte) 0x23, (byte) 0x42, (byte) 0xF0),
        EXPECTED_INT8_ARRAY);
  }

  @Test
  public void serializeString() throws IOException {
    assertEncodingMatches("hello world", EXPECTED_STRING);
  }

  @Test
  public void serializeEmptyMap() throws IOException {
    assertEncodingMatches(ImmutableMap.of(), EXPECTED_EMPTY_MAP);
  }

  @Test
  public void serializeInt8Map() throws IOException {
    assertEncodingMatches(
        ImmutableMap.of(
            "foo", (byte) 0x23,
            "bar", (byte) 0x42,
            "baz", (byte) 0xF0),
        EXPECTED_INT8_MAP);
  }

  @Test
  public void serializeInt8() throws IOException {
    assertEncodingMatches((byte) 0x42, EXPECTED_INT8);
  }

  @Test
  public void serializeInt16() throws IOException {
    assertEncodingMatches(0x11FF, EXPECTED_INT16);
  }

  @Test
  public void serializeInt32() throws IOException {
    assertEncodingMatches(0x1122EEFF, EXPECTED_INT32);
  }

  @Test
  public void serializeInt64() throws IOException {
    assertEncodingMatches(0x11223344CCDDEEFFL, EXPECTED_INT64);
  }

  @Test
  public void serializeMinInt8() throws IOException {
    assertEncodingMatches((byte) -0x80, EXPECTED_MIN_INT8);
  }

  @Test
  public void serializeMinInt16() throws IOException {
    assertEncodingMatches(-0x8000, EXPECTED_MIN_INT16);
  }

  @Test
  public void serializeMinInt32() throws IOException {
    assertEncodingMatches(-0x80000000, EXPECTED_MIN_INT32);
  }

  @Test
  public void serializeMinInt64() throws IOException {
    assertEncodingMatches(-0x8000000000000000L, EXPECTED_MIN_INT64);
  }

  @Test
  public void serializeReal() throws IOException {
    assertEncodingMatches(0.123456789, EXPECTED_REAL);
  }

  @Test
  public void serializeTrue() throws IOException {
    assertEncodingMatches(true, EXPECTED_TRUE);
  }

  @Test
  public void serializeFalse() throws IOException {
    assertEncodingMatches(false, EXPECTED_FALSE);
  }

  @Test
  public void serializeNull() throws IOException {
    assertEncodingMatches(null, EXPECTED_NULL);
  }

  @Test
  public void throwIfStringNotUTF8() throws IOException {
    thrown.expect(CharacterCodingException.class);
    BserSerializer serializer = new BserSerializer();
    // UTF-8 cannot legally represent half of a surrogate pair, so this should throw.
    serializer.serializeToBuffer("\uDC00", buffer);
  }

  @Test
  public void throwIfMapKeyNotString() throws IOException {
    thrown.expect(IOException.class);
    thrown.expectMessage("Unrecognized map key type class java.lang.Integer, expected string");
    BserSerializer serializer = new BserSerializer();
    serializer.serializeToBuffer(ImmutableMap.of(0, 1), buffer);
  }

  @Test
  public void smallObjectReusesInputBuffer() throws IOException {
    BserSerializer serializer = new BserSerializer();
    ByteBuffer result = serializer.serializeToBuffer(true, buffer);
    assertThat(result, is(sameInstance(buffer)));
  }

  @Test
  public void largeObjectAllocatesNewBuffer() throws IOException {
    BserSerializer serializer = new BserSerializer();
    ByteBuffer result = serializer.serializeToBuffer(Strings.repeat("X", 10000), buffer);
    assertThat(result, is(not(sameInstance(buffer))));
  }

  @Test
  public void serializeTrueToStream() throws IOException {
    try (ByteArrayOutputStream os = new ByteArrayOutputStream()) {
      BserSerializer serializer = new BserSerializer();
      serializer.serializeToStream(true, os);
      assertThat(os.toByteArray(), equalTo(BaseEncoding.base16().decode(EXPECTED_TRUE)));
    }
  }
}
