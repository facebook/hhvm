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
import static org.hamcrest.core.Is.is;
import static org.junit.Assert.assertThat;

import com.facebook.thrift.utils.ByteBufferUtils;
import java.io.InputStream;
import java.nio.ByteBuffer;
import org.junit.Test;

public class ByteBufferUtilsTest {
  private ByteBuffer generateBuffer(boolean useDirectBuffer, byte[] bytes) {
    ByteBuffer buf;
    if (useDirectBuffer) {
      buf = ByteBuffer.allocateDirect(bytes.length);
      buf.put(bytes);
      buf.flip();
    } else {
      buf = ByteBuffer.wrap(bytes);
    }
    return buf;
  }

  private ByteBuffer generateBuffer(boolean useDirectBuffer, String data) {
    return generateBuffer(useDirectBuffer, data.getBytes());
  }

  @Test
  public void testToHexString_returnCorrectValue_givenUnreadBuffer() throws Exception {
    ByteBuffer buf = generateBuffer(false, "toto1234");
    String hex = ByteBufferUtils.toHexString(buf);
    assertThat(hex, equalTo("74 6f 74 6f 31 32 33 34"));

    byte[] bytes = new byte[] {(byte) 0x80, (byte) 0xff, (byte) 0x00, (byte) 0x0f, (byte) 0x7f};
    buf = generateBuffer(false, bytes);
    hex = ByteBufferUtils.toHexString(buf);
    assertThat(hex, equalTo("80 ff 00 0f 7f"));
  }

  @Test
  public void testToHexString_returnCorrectValue_givenUnreadDirectBuffer() throws Exception {
    ByteBuffer buf = generateBuffer(true, "toto1234");
    String hex = ByteBufferUtils.toHexString(buf);
    assertThat(hex, equalTo("74 6f 74 6f 31 32 33 34"));

    byte[] bytes = new byte[] {(byte) 0x80, (byte) 0xff, (byte) 0x00, (byte) 0x0f, (byte) 0x7f};
    buf = generateBuffer(true, bytes);
    hex = ByteBufferUtils.toHexString(buf);
    assertThat(hex, equalTo("80 ff 00 0f 7f"));
  }

  @Test
  public void testToHexString_returnCorrectValue_givenPartialBuffer() throws Exception {
    ByteBuffer buf = generateBuffer(false, "123toto1234");
    buf.position(3);
    buf.limit(buf.capacity() - 4);
    String hex = ByteBufferUtils.toHexString(buf);
    assertThat(hex, equalTo("74 6f 74 6f"));

    byte[] bytes =
        new byte[] {
          (byte) 0x00, // prefix
          (byte) 0x00, // prefix
          (byte) 0x00, // prefix
          (byte) 0x80,
          (byte) 0xff,
          (byte) 0x00,
          (byte) 0x0f,
          (byte) 0x7f,
          (byte) 0x00, // suffix
          (byte) 0x00, // suffix
          (byte) 0x00, // suffix
          (byte) 0x00 // suffix
        };
    buf = generateBuffer(false, bytes);
    buf.position(3);
    buf.limit(buf.capacity() - 4);
    hex = ByteBufferUtils.toHexString(buf);
    assertThat(hex, equalTo("80 ff 00 0f 7f"));
  }

  @Test
  public void testToHexString_returnCorrectValue_givenPartialDirectBuffer() throws Exception {
    ByteBuffer buf = generateBuffer(true, "123toto1234");
    buf.position(3);
    buf.limit(buf.capacity() - 4);
    String hex = ByteBufferUtils.toHexString(buf);
    assertThat(hex, equalTo("74 6f 74 6f"));

    byte[] bytes =
        new byte[] {
          (byte) 0x00, // prefix
          (byte) 0x00, // prefix
          (byte) 0x00, // prefix
          (byte) 0x80,
          (byte) 0xff,
          (byte) 0x00,
          (byte) 0x0f,
          (byte) 0x7f,
          (byte) 0x00, // suffix
          (byte) 0x00, // suffix
          (byte) 0x00, // suffix
          (byte) 0x00 // suffix
        };
    buf = generateBuffer(true, bytes);
    buf.position(3);
    buf.limit(buf.capacity() - 4);
    hex = ByteBufferUtils.toHexString(buf);
    assertThat(hex, equalTo("80 ff 00 0f 7f"));
  }

  @Test
  public void testToBytes_returnCorrectValue_givenUnreadBuffer() throws Exception {
    String input = "QWERTY";
    ByteBuffer buf = generateBuffer(false, input);
    byte[] expected = input.getBytes();
    byte[] bytes = ByteBufferUtils.toBytes(buf);
    assertThat(bytes, equalTo(expected));
  }

  @Test
  public void testToBytes_returnCorrectValue_givenUnreadDirectBuffer() throws Exception {
    String input = "QWERTY";
    ByteBuffer buf = generateBuffer(true, input);
    byte[] expected = input.getBytes();
    byte[] bytes = ByteBufferUtils.toBytes(buf);
    assertThat(bytes, equalTo(expected));
  }

  @Test
  public void testToBytes_returnCorrectValue_givenPartialBuffer() throws Exception {
    String input = "123QWERTY12345";
    ByteBuffer buf = generateBuffer(false, input);
    buf.position(3);
    buf.limit(buf.capacity() - 5);
    byte[] expected = "QWERTY".getBytes();
    byte[] bytes = ByteBufferUtils.toBytes(buf);
    assertThat(bytes, equalTo(expected));
  }

  @Test
  public void testToBytes_returnCorrectValue_givenPartialDirectBuffer() throws Exception {
    String input = "123QWERTY12345";
    ByteBuffer buf = generateBuffer(true, input);
    buf.position(3);
    buf.limit(buf.capacity() - 5);
    byte[] expected = "QWERTY".getBytes();
    byte[] bytes = ByteBufferUtils.toBytes(buf);
    assertThat(bytes, equalTo(expected));
  }

  @Test
  public void testDeepCopy() throws Exception {
    String original = "XXXXX";
    byte[] data = original.getBytes();
    ByteBuffer buf = ByteBuffer.wrap(data);
    ByteBuffer copy = ByteBufferUtils.deepCopy(buf);
    assertThat(copy, is(buf));

    data[0] = '_'; // data is modified
    ByteBuffer buf2 = ByteBuffer.wrap(original.getBytes());
    assertThat(copy, equalTo(buf2));
  }

  @Test
  public void testInputStream() throws Exception {
    byte[] data = "12345".getBytes();
    ByteBuffer buf = ByteBuffer.wrap(data);
    InputStream in = ByteBufferUtils.toInputStream(buf);

    byte[] bytes = new byte[data.length];
    in.read(bytes);
    assertThat(bytes, equalTo(data));
  }
}
