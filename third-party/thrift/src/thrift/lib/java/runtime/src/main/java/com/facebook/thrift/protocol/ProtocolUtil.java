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

import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.payload.Writer;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.Unpooled;
import io.netty.util.ReferenceCountUtil;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import org.apache.thrift.ProtocolId;
import org.apache.thrift.protocol.TProtocol;

/**
 * Util class for serializing and de-serializing Thrift Structs working directly with Netty's
 * ByteBuf API.
 *
 * <p>This class is less user friendly then {@link com.facebook.thrift.util.SerializerUtil} because
 * you need to work with ByteBuf directly.
 */
public final class ProtocolUtil {
  private ProtocolUtil() {}

  public static void write(Writer writer, ByteBufTProtocol protocol) {
    writer.write(protocol);
  }

  public static void write(Writer writer, ByteBuf dest, TProtocolType type) {
    write(writer, type.apply(dest));
  }

  public static void write(Writer writer, OutputStream os, ByteBuf dest, TProtocolType type)
      throws IOException {
    write(writer, dest, type);
    dest.readBytes(os, dest.readableBytes());
  }

  public static void write(
      Writer writer, OutputStream os, ByteBufAllocator alloc, TProtocolType type)
      throws IOException {
    ByteBuf dest = alloc.buffer();
    try {
      write(writer, os, dest, type);
    } finally {
      ReferenceCountUtil.safeRelease(dest);
    }
  }

  public static <T> T read(Reader<T> reader, ByteBufTProtocol protocol) {
    return reader.read(protocol);
  }

  public static <T> T read(Reader<T> reader, ByteBuf src, TProtocolType type) {
    return read(reader, type.apply(src));
  }

  public static <T> T read(
      Reader<T> reader, InputStream is, int length, ByteBuf src, TProtocolType type)
      throws IOException {
    src.writeBytes(is, length);
    return read(reader, src, type);
  }

  public static <T> T read(
      Reader<T> reader, InputStream is, int length, ByteBufAllocator alloc, TProtocolType type)
      throws IOException {
    ByteBuf src = alloc.buffer();
    try {
      return read(reader, is, length, src, type);
    } finally {
      ReferenceCountUtil.safeRelease(src);
    }
  }

  public static TProtocol toTBinaryProtocol(byte[] src) {
    return toTProtocol(Unpooled.wrappedBuffer(src), ProtocolId.BINARY);
  }

  public static TProtocol toTCompactProtocol(byte[] src) {
    return toTProtocol(Unpooled.wrappedBuffer(src), ProtocolId.COMPACT);
  }

  public static TProtocol toTBinaryProtocol(ByteBuffer src) {
    return toTProtocol(Unpooled.wrappedBuffer(src), ProtocolId.BINARY);
  }

  public static TProtocol toTCompactProtocol(ByteBuffer src) {
    return toTProtocol(Unpooled.wrappedBuffer(src), ProtocolId.COMPACT);
  }

  public static TProtocol toTBinaryProtocol(ByteBuf src) {
    return toTProtocol(src, ProtocolId.BINARY);
  }

  public static TProtocol toTCompactProtocol(ByteBuf src) {
    return toTProtocol(src, ProtocolId.COMPACT);
  }

  public static TProtocol toTProtocol(ByteBuf src, ProtocolId type) {
    return TProtocolType.fromProtocolId(type).apply(src);
  }
}
