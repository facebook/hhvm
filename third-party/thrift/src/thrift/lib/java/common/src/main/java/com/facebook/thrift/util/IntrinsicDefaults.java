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

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import java.nio.ByteBuffer;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * This class contains the methods which return the intrinsic default for a thrift field, and
 * methods to check if a value is an intrinsic default.
 */
public final class IntrinsicDefaults {
  private static final IntrinsicDefaults INSTANCE = new IntrinsicDefaults();

  private IntrinsicDefaults() {}

  public static IntrinsicDefaults getInstance() {
    return INSTANCE;
  }

  public static boolean defaultBoolean() {
    return false;
  }

  public static boolean isDefault(boolean b) {
    return b == defaultBoolean();
  }

  public static boolean isDefault(Boolean b) {
    return b != null && isDefault(b.booleanValue());
  }

  public static byte defaultByte() {
    return 0;
  }

  public static int defaultEnum() {
    return 0;
  }

  public static boolean isDefault(byte b) {
    return b == defaultByte();
  }

  public static boolean isDefault(Byte b) {
    return b != null && isDefault(b.byteValue());
  }

  public static short defaultShort() {
    return 0;
  }

  public static boolean isDefault(short s) {
    return s == defaultShort();
  }

  public static boolean isDefault(Short s) {
    return s != null && isDefault(s.shortValue());
  }

  public static int defaultInt() {
    return 0;
  }

  public static long defaultLong() {
    return 0;
  }

  public static boolean isDefault(int i) {
    return i == defaultInt();
  }

  public static boolean isDefault(Integer i) {
    return i != null && isDefault(i.intValue());
  }

  public static boolean isDefault(long l) {
    return l == defaultLong();
  }

  public static boolean isDefault(Long l) {
    return l != null && isDefault(l.longValue());
  }

  public static float defaultFloat() {
    return 0F;
  }

  public static boolean isDefault(float f) {
    return f == defaultFloat();
  }

  public static boolean isDefault(Float f) {
    return f != null && isDefault(f.floatValue());
  }

  public static double defaultDouble() {
    return 0D;
  }

  public static boolean isDefault(double d) {
    return d == defaultDouble();
  }

  public static boolean isDefault(Double d) {
    return d != null && isDefault(d.doubleValue());
  }

  public static String defaultString() {
    return "";
  }

  public static boolean isDefault(String s) {
    return s != null && s.equals(defaultString());
  }

  public static <T> Set<T> defaultSet() {
    return Collections.emptySet();
  }

  public static boolean isDefault(Set<?> s) {
    return s != null && s.isEmpty();
  }

  public static <T> List<T> defaultList() {
    return Collections.emptyList();
  }

  public static boolean isDefault(List<?> l) {
    return l != null && l.isEmpty();
  }

  public static <K, V> Map<K, V> defaultMap() {
    return Collections.emptyMap();
  }

  public static boolean isDefault(Map<?, ?> m) {
    return m != null && m.isEmpty();
  }

  private static final byte[] EMPTY_BYTE_ARRAY = new byte[0];

  private static final ByteBuffer EMPTY_BYTE_BUFFER = ByteBuffer.allocate(0).asReadOnlyBuffer();

  public static byte[] defaultByteArray() {
    return EMPTY_BYTE_ARRAY;
  }

  public static ByteBuffer defaultByteBuffer() {
    return EMPTY_BYTE_BUFFER;
  }

  public static ByteBuf defaultByteBuf() {
    return Unpooled.EMPTY_BUFFER;
  }

  public static boolean isDefault(byte[] bytes) {
    return defaultByteArray() == bytes || (bytes != null && bytes.length == 0);
  }

  public static boolean isDefault(ByteBuffer byteBuffer) {
    return defaultByteBuffer() == byteBuffer || (byteBuffer != null && byteBuffer.remaining() == 0);
  }

  public static boolean isDefault(ByteBuf byteBuf) {
    return defaultByteBuf() == byteBuf || (byteBuf != null && !byteBuf.isReadable());
  }
}
