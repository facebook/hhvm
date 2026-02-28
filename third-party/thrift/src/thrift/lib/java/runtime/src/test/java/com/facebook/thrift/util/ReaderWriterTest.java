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

import static com.facebook.thrift.util.SerializationProtocol.TCompact;
import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSet;
import io.netty.buffer.Unpooled;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;
import org.junit.Before;
import org.junit.Test;

public class ReaderWriterTest {
  private Random random;

  @Before
  public void setup() {
    random = new Random();
  }

  @Test
  public void testByteArray() {
    final byte[] test = new byte[10];
    random.nextBytes(test);

    byte[] serialized = SerializerUtil.toByteArrayWriter(Writers.binaryWriter(test), TCompact);
    byte[] deserialized =
        SerializerUtil.fromByteArray(Readers.binaryReader(), serialized, TCompact);

    assertArrayEquals(test, deserialized);
  }

  @Test
  public void testByteBuffer() {
    final byte[] test = new byte[10];
    random.nextBytes(test);

    byte[] serialized =
        SerializerUtil.toByteArrayWriter(Writers.binaryWriter(ByteBuffer.wrap(test)), TCompact);
    byte[] deserialized =
        SerializerUtil.fromByteArray(Readers.binaryReader(), serialized, TCompact);

    assertArrayEquals(test, deserialized);
  }

  @Test
  public void testByteBuf() {
    final byte[] test = new byte[10];
    random.nextBytes(test);

    byte[] serialized =
        SerializerUtil.toByteArrayWriter(
            Writers.binaryWriter(Unpooled.wrappedBuffer(test)), TCompact);
    byte[] deserialized =
        SerializerUtil.fromByteArray(Readers.binaryReader(), serialized, TCompact);

    assertArrayEquals(test, deserialized);
  }

  @Test
  public void testBoolean() {
    final boolean test = random.nextBoolean();

    byte[] serialized = SerializerUtil.toByteArrayWriter(Writers.booleanWriter(test), TCompact);
    boolean deserialized =
        SerializerUtil.fromByteArray(Readers.booleanReader(), serialized, TCompact);

    assertEquals(test, deserialized);
  }

  @Test
  public void tesString() {
    StringBuilder builder = new StringBuilder();
    for (int i = 0; i < 10; i++) {
      // random alpha chars
      builder.append((char) random.nextInt(26) + 'a');
    }
    String test = builder.toString();

    byte[] serialized = SerializerUtil.toByteArrayWriter(Writers.stringWriter(test), TCompact);
    String deserialized =
        SerializerUtil.fromByteArray(Readers.stringReader(), serialized, TCompact);

    assertEquals(test, deserialized);
  }

  @Test
  public void testByte() {
    byte test = (byte) random.nextInt();

    byte[] serialized = SerializerUtil.toByteArrayWriter(Writers.byteWriter(test), TCompact);
    byte deserialized = SerializerUtil.fromByteArray(Readers.byteReader(), serialized, TCompact);

    assertEquals(test, deserialized);
  }

  @Test
  public void testI16() {
    short test = (short) random.nextInt();

    byte[] serialized = SerializerUtil.toByteArrayWriter(Writers.i16Writer(test), TCompact);
    short deserialized = SerializerUtil.fromByteArray(Readers.i16Reader(), serialized, TCompact);

    assertEquals(test, deserialized);
  }

  @Test
  public void testI32() {
    int test = random.nextInt();

    byte[] serialized = SerializerUtil.toByteArrayWriter(Writers.i32Writer(test), TCompact);
    int deserialized = SerializerUtil.fromByteArray(Readers.i32Reader(), serialized, TCompact);

    assertEquals(test, deserialized);
  }

  @Test
  public void testI64() {
    long test = random.nextLong();

    byte[] serialized = SerializerUtil.toByteArrayWriter(Writers.i64Writer(test), TCompact);
    long deserialized = SerializerUtil.fromByteArray(Readers.i64Reader(), serialized, TCompact);

    assertEquals(test, deserialized);
  }

  @Test
  public void testFloat() {
    float test = random.nextFloat();

    byte[] serialized = SerializerUtil.toByteArrayWriter(Writers.floatWriter(test), TCompact);
    float deserialized = SerializerUtil.fromByteArray(Readers.floatReader(), serialized, TCompact);

    assertEquals(test, deserialized, 0);
  }

  @Test
  public void testDouble() {
    double test = random.nextDouble();

    byte[] serialized = SerializerUtil.toByteArrayWriter(Writers.doubleWriter(test), TCompact);
    double deserialized =
        SerializerUtil.fromByteArray(Readers.doubleReader(), serialized, TCompact);

    assertEquals(test, deserialized, 0);
  }

  @Test
  public void testList() {
    List<Integer> test = Arrays.asList(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);

    byte[] serialized =
        SerializerUtil.toByteArrayWriter(Writers.listWriter(test, Integer.class), TCompact);
    List<Integer> deserialized =
        SerializerUtil.fromByteArray(Readers.listReader(Readers.i32Reader()), serialized, TCompact);

    assertEquals(test, deserialized);
  }

  @Test
  public void testMap() {
    Map<String, Integer> test = ImmutableMap.of("one", 1, "two", 2, "three", 3);

    byte[] serialized =
        SerializerUtil.toByteArrayWriter(
            Writers.mapWriter(test, String.class, Integer.class), TCompact);
    Map<String, Integer> deserialized =
        SerializerUtil.fromByteArray(
            Readers.mapReader(Readers.stringReader(), Readers.i32Reader()), serialized, TCompact);

    assertEquals(test, deserialized);
  }

  @Test
  public void testSet() {
    Set<Integer> test = ImmutableSet.of(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);

    byte[] serialized =
        SerializerUtil.toByteArrayWriter(Writers.setWriter(test, Integer.class), TCompact);
    Set<Integer> deserialized =
        SerializerUtil.fromByteArray(Readers.setReader(Readers.i32Reader()), serialized, TCompact);

    assertEquals(test, deserialized);
  }
}
