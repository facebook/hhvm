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

package com.facebook.thrift.util;

import com.facebook.thrift.test.EveryLayout;
import com.facebook.thrift.test.universalname.TestRequest;
import com.facebook.thrift.util.resources.ChunkedInputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.concurrent.ThreadLocalRandom;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

public class SerializerUtilTest {
  private EveryLayout everyLayout;

  @Before
  public void setup() {
    byte[] bytes = new byte[128];
    ThreadLocalRandom.current().nextBytes(bytes);
    everyLayout =
        new EveryLayout.Builder()
            .setABool(true)
            .setAInt(Integer.MAX_VALUE)
            .setALong(Long.MAX_VALUE)
            .setAString("a string")
            .setADouble(Double.MAX_VALUE)
            .setAFloat(Float.MAX_VALUE)
            .setAShort(Short.MIN_VALUE)
            .setAByte(Byte.MAX_VALUE)
            .setABinary(bytes)
            .setAList(Arrays.asList("1", "2", "3"))
            .build();
  }

  @Test
  public void testToAndFromByteBuffer() {
    ByteBuffer byteBuffer = SerializerUtil.toByteBuffer(everyLayout, SerializationProtocol.TBinary);

    EveryLayout everyLayout =
        SerializerUtil.fromByteBuffer(
            EveryLayout.asReader(), byteBuffer, SerializationProtocol.TBinary);

    Assert.assertEquals(this.everyLayout, everyLayout);
  }

  @Test
  public void testToAndFromByteArray() {
    byte[] bytes = SerializerUtil.toByteArray(everyLayout, SerializationProtocol.TBinary);

    EveryLayout everyLayout =
        SerializerUtil.fromByteArray(EveryLayout.asReader(), bytes, SerializationProtocol.TBinary);

    Assert.assertEquals(this.everyLayout, everyLayout);
  }

  @Test
  public void testToAndFromByteArrayWithOffset() {
    byte[] bytes = SerializerUtil.toByteArray(everyLayout, SerializationProtocol.TBinary);

    byte[] other = new byte[bytes.length * 2];
    System.arraycopy(bytes, 0, other, 1, bytes.length);

    EveryLayout everyLayout =
        SerializerUtil.fromByteArray(
            EveryLayout.asReader(), other, 1, bytes.length, SerializationProtocol.TBinary);

    Assert.assertEquals(this.everyLayout, everyLayout);
  }

  @Test
  public void testToAndFromStreams() {
    InputStream in = SerializerUtil.toInputStream(everyLayout, SerializationProtocol.TBinary);
    EveryLayout everyLayout =
        SerializerUtil.fromInputStream(EveryLayout.asReader(), in, SerializationProtocol.TBinary);
    Assert.assertEquals(this.everyLayout, everyLayout);
  }

  /**
   * Test reading a chunked input stream where reading will only return up to 3 bytes max, this
   * simulates effects we saw in the sanppy input stream reading scenario where it only returns up
   * to ~32kb at a time. This test ensures that our read api's read and fill the array buffers with
   * the expect byte content.
   */
  @Test
  public void testToChunkedInputStream() {
    InputStream in = SerializerUtil.toInputStream(everyLayout, SerializationProtocol.TBinary);
    EveryLayout everyLayout =
        SerializerUtil.fromInputStream(
            EveryLayout.asReader(), new ChunkedInputStream(in), SerializationProtocol.TBinary);
    Assert.assertEquals(this.everyLayout, everyLayout);
  }

  @Test(expected = IndexOutOfBoundsException.class)
  public void testBadDataWithoutStopTagThrowsIndexOutOfBoundsException() {
    ByteBuffer in =
        SerializerUtil.toByteBuffer(
            new TestRequest.Builder().setAString("abcd").build(), SerializationProtocol.TCompact);

    // Remove Trailing byte tha would normally indicate STOP
    byte[] slice = new byte[in.capacity() - 1];
    in.get(slice, 0, in.capacity() - 1);

    // Use ByteArrayInputStream to simulate use of input stream
    TestRequest everyLayout =
        SerializerUtil.fromInputStream(
            TestRequest.asReader(),
            new ByteArrayInputStream(slice),
            SerializationProtocol.TCompact);
  }

  @Test
  public void testToOutputStream() {
    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    SerializerUtil.toOutStream(everyLayout, bos, SerializationProtocol.TBinary);
    EveryLayout everyLayout =
        SerializerUtil.fromByteArray(
            EveryLayout.asReader(), bos.toByteArray(), SerializationProtocol.TBinary);
    Assert.assertEquals(this.everyLayout, everyLayout);
  }

  @Test
  public void testFromJsonStringBase64() {
    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    SerializerUtil.toOutStream(everyLayout, bos, SerializationProtocol.TSimpleJSONBase64);
    String json = new String(bos.toByteArray(), StandardCharsets.UTF_8);
    EveryLayout everyLayout = SerializerUtil.fromJsonStringBase64(EveryLayout.asReader(), json);
    Assert.assertEquals(this.everyLayout, everyLayout);
  }

  @Test
  public void testFromsTJsonString() {
    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    SerializerUtil.toOutStream(everyLayout, bos, SerializationProtocol.TJSON);
    String json = new String(bos.toByteArray(), StandardCharsets.UTF_8);
    EveryLayout everyLayout = SerializerUtil.fromTJsonString(EveryLayout.asReader(), json);
    Assert.assertEquals(this.everyLayout, everyLayout);
  }

  @Test
  public void testToAndFromBinaryBase64() {
    String base64 = SerializerUtil.toBase64(everyLayout, false, SerializationProtocol.TBinary);
    EveryLayout everyLayout =
        SerializerUtil.fromBase64(
            EveryLayout.asReader(), base64, false, SerializationProtocol.TBinary);
    Assert.assertEquals(this.everyLayout, everyLayout);
  }

  @Test
  public void testToAndFromCompactBase64() {
    String base64 = SerializerUtil.toBase64(everyLayout, false, SerializationProtocol.TCompact);
    EveryLayout everyLayout =
        SerializerUtil.fromBase64(
            EveryLayout.asReader(), base64, false, SerializationProtocol.TCompact);
    Assert.assertEquals(this.everyLayout, everyLayout);
  }

  @Test
  public void testToAndFromBinaryBase64UrlSafe() {
    String base64 = SerializerUtil.toBase64(everyLayout, true, SerializationProtocol.TBinary);
    EveryLayout everyLayout =
        SerializerUtil.fromBase64(
            EveryLayout.asReader(), base64, true, SerializationProtocol.TBinary);
    Assert.assertEquals(this.everyLayout, everyLayout);
  }

  @Test
  public void testToAndFromCompactBase64UrlSafe() {
    String base64 = SerializerUtil.toBase64(everyLayout, true, SerializationProtocol.TCompact);
    EveryLayout everyLayout =
        SerializerUtil.fromBase64(
            EveryLayout.asReader(), base64, true, SerializationProtocol.TCompact);
    Assert.assertEquals(this.everyLayout, everyLayout);
  }
}
