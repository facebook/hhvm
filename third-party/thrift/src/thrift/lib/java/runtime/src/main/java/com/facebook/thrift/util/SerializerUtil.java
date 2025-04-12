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

import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.payload.ThriftSerializable;
import com.facebook.thrift.payload.Writer;
import com.facebook.thrift.protocol.ByteBufBase64TSimpleJSONProtocol;
import com.facebook.thrift.protocol.ByteBufDefaultTSimpleJSONProtocol;
import com.facebook.thrift.protocol.ByteBufTJSONProtocol;
import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.protocol.ProtocolUtil;
import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufInputStream;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import io.netty.handler.codec.base64.Base64;
import io.netty.handler.codec.base64.Base64Dialect;
import io.netty.util.ReferenceCountUtil;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import reactor.core.Exceptions;

/**
 * User friendly class for serializing and de-serializing Thrift Structs. If you want to work with
 * ByteBuf or the underlying protocol directly use {@link com.facebook.thrift.protocol.ProtocolUtil}
 */
public final class SerializerUtil {

  private SerializerUtil() {}

  /**
   * De-serializing a type from a JSON String using the TSimpleJSONBase64 protocol. Binary data
   * fields will be read using Base64 encoding. This is the preferred method.
   *
   * @param <T>
   */
  public static <T> T fromJsonStringBase64(Reader<T> reader, String t) {
    byte[] bytes = t.getBytes(StandardCharsets.UTF_8);
    SerializationProtocol protocol = SerializationProtocol.TSimpleJSONBase64;
    return fromByteArray(reader, bytes, protocol);
  }

  /**
   * De-serializing a type from a JSON String using the TJSON protocol. Binary data fields will be
   * read using Base64 encoding. This is the preferred method.
   *
   * @param <T>
   */
  public static <T> T fromTJsonString(Reader<T> reader, String t) {
    byte[] bytes = t.getBytes(StandardCharsets.UTF_8);
    SerializationProtocol protocol = SerializationProtocol.TJSON;
    return fromByteArray(reader, bytes, protocol);
  }

  /**
   * De-serializing a type T from a ByteBuffer. Thrift Java Structs have a static method read0 on
   * them. This method is used as follows: <code>
   * ByteBuffer src = ...
   * Foo f = SerializerUtil.fromByteBuffer(Foo.asReader(), src, TProtocolType.TBinary);
   * </code>
   *
   * @param reader the interface that will read the TProtocol - can be a lambda from Struct
   * @param src ByteBuf that contains the serialized struct
   * @param protocol the protocol of the serialized struct
   * @param <T> The type to return
   * @return the de-serialized object
   */
  public static <T> T fromByteBuffer(
      Reader<T> reader, ByteBuffer src, SerializationProtocol protocol) {
    ByteBufTProtocol byteBufTProtocol = toByteBufProtocol(protocol, Unpooled.wrappedBuffer(src));
    return reader.read(byteBufTProtocol);
  }

  /**
   * De-serializing a type T from a byte[]. Thrift Java Structs have a static method read0 on them.
   * This method is used as follows: <code>
   * byte[] src = ...
   * Foo f = SerializerUtil.fromByteBuffer(Foo.asReader(), src, 0, 10, TProtocolType.TBinary);
   * </code>
   *
   * @param reader the interface that will read the TProtocol - can be a lambda from Struct
   * @param src byte array that contains the serialized struct
   * @param offset the offset to start reading bytes from
   * @param length the length of bytes in the array that contains the struct
   * @param protocol the protocol of the serialized struct
   * @param <T> The type to return
   * @return the de-serialized object
   */
  public static <T> T fromByteArray(
      Reader<T> reader, byte[] src, int offset, int length, SerializationProtocol protocol) {
    ByteBufTProtocol byteBufTProtocol =
        toByteBufProtocol(protocol, Unpooled.wrappedBuffer(src, offset, length));
    return reader.read(byteBufTProtocol);
  }

  /**
   * De-serializing a type T from an {@link InputStream}. Thrift Java Structs have a static method
   * read0 on them. This method is used as follows: <code>
   * InputStream is = ...
   * Foo f = SerializerUtil.fromInputStream(Foo.asReader(), is, TProtocolType.TBinary);
   * </code>
   *
   * @param reader
   * @param is
   * @param protocol
   * @param <T>
   * @return
   */
  public static <T> T fromInputStream(
      Reader<T> reader, InputStream is, SerializationProtocol protocol) {
    InputStreamByteBuf byteBuf = new InputStreamByteBuf(is);
    ByteBufTProtocol byteBufTProtocol = toByteBufProtocol(protocol, byteBuf);
    return reader.read(byteBufTProtocol);
  }

  /**
   * De-serializing a type T from a byte[]. Thrift Java Structs have a static method read0 on them.
   * This method is used as follows: <code>
   * byte[] src = ...
   * Foo f = SerializerUtil.fromByteBuffer(Foo.asReader(), src, TProtocolType.TBinary);
   * </code>
   *
   * @param reader the interface that will read the TProtocol - can be a lambda from Struct
   * @param src byte array that contains the serialized struct
   * @param protocol the protocol of the serialized struct
   * @param <T> The type to return
   * @return the de-serialized object
   */
  public static <T> T fromByteArray(Reader<T> reader, byte[] src, SerializationProtocol protocol) {
    ByteBufTProtocol byteBufTProtocol = toByteBufProtocol(protocol, Unpooled.wrappedBuffer(src));
    return reader.read(byteBufTProtocol);
  }

  /**
   * Serializes a Thrift struct a direct {@link ByteBuffer} using a Writer. Thrift Java Structs
   * instance have a write0 method on them. This method is used as follows: <code>
   * Foo instance = ...
   * ByteBuffer b = SerializerUtil.toByteBuffer(instance, TProtocolType.TBinary);
   * </code>
   *
   * @param writer the interface that will write to TProtocol - can be a lambda from a Struct
   * @param protocol the protocol to serialize the struct with
   * @return a ByteBuffer with the serialized data
   */
  public static ByteBuffer toByteBuffer(Writer writer, SerializationProtocol protocol) {
    return doToByteBuffer(writer, protocol);
  }

  public static ByteBuffer toByteBuffer(ThriftSerializable writer, SerializationProtocol protocol) {
    return doToByteBuffer(writer::write0, protocol);
  }

  private static ByteBuffer doToByteBuffer(Writer writer, SerializationProtocol protocol) {
    ByteBuf dst = Unpooled.buffer();
    writer.write(toByteBufProtocol(protocol, dst));
    return dst.nioBuffer();
  }

  /**
   * Serializes a Thrift struct a direct {@link ByteBuffer} using a Writer. Thrift Java Structs
   * instance have a write0 method on them. This method is used as follows: <code>
   * Foo instance = ...
   * ByteBuffer b = SerializerUtil.toByteBuffer(instance, TProtocolType.TBinary);
   * </code>
   *
   * @param writer the interface that will write to TProtocol - can be a lambda from a Struct
   * @param protocol the protocol to serialize the struct with
   * @return a byte[] with the serialized data
   */
  public static byte[] toByteArrayWriter(Writer writer, SerializationProtocol protocol) {
    return doToByteArray(writer, protocol);
  }

  public static byte[] toByteArray(ThriftSerializable writer, SerializationProtocol protocol) {
    return doToByteArray(writer::write0, protocol);
  }

  private static byte[] doToByteArray(Writer writer, SerializationProtocol protocol) {
    ByteBuf dst = RpcResources.getByteBufAllocator().buffer();
    try {
      ByteBufTProtocol apply = toByteBufProtocol(protocol, dst);
      writer.write(apply);
      return ByteBufUtil.getBytes(dst);
    } finally {
      ReferenceCountUtil.safeRelease(dst);
    }
  }

  /**
   * Serializes a Thrift struct an {@link InputStream} using a {@link Writer}. Thrift Java Structs
   * instances have a write0 method on them. This is method is used as follows: <code>
   * Foo instance = ...
   * OutputStream os = SerializeUtil.toInputStream(instance, TProtocolType.TBinary);
   * </code>
   *
   * @param writer the interface that will write to TProtocol - can be a lambda from a Struct
   * @param protocol protocol struct is serialized with
   * @return an {@link OutputStream} with the serialized data
   */
  public static ByteBufInputStream toInputStream(Writer writer, SerializationProtocol protocol) {
    return doToInputStream(writer, protocol);
  }

  public static ByteBufInputStream toInputStream(
      ThriftSerializable writer, SerializationProtocol protocol) {
    return doToInputStream(writer::write0, protocol);
  }

  private static ByteBufInputStream doToInputStream(Writer writer, SerializationProtocol protocol) {
    ByteBuf dst = RpcResources.getUnpooledByteBufAllocator().buffer();
    writer.write(toByteBufProtocol(protocol, dst));
    return new ByteBufInputStream(dst);
  }

  public static void toOutStream(Writer writer, OutputStream os, SerializationProtocol protocol) {
    doToOutStream(writer, os, protocol);
  }

  public static void toOutStream(
      ThriftSerializable writer, OutputStream os, SerializationProtocol protocol) {
    doToOutStream(writer::write0, os, protocol);
  }

  private static void doToOutStream(
      Writer writer, OutputStream os, SerializationProtocol protocol) {
    try {
      byte[] bytes = doToByteArray(writer, protocol);
      os.write(bytes);
    } catch (Exception e) {
      throw Exceptions.propagate(e);
    }
  }

  /**
   * Creates a deep copy of a Thrift Struct. It's used as follows: <code>
   * Foo source = ...;
   * Foo copy = SerializerUtil.deepCopy(source, Foo.asReader());
   * </code>
   *
   * @param source The struct you want to copy
   * @param reader A {@link Reader} than can deserialize the struct you want to copy
   * @return the copied struct
   */
  public static <R extends ThriftSerializable> R deepCopy(R source, Reader<R> reader) {
    ByteBuf temp = null;
    try {
      temp = RpcResources.getByteBufAllocator().buffer();
      ByteBufTProtocol protocol = TProtocolType.TBinary.apply(temp);
      ProtocolUtil.write(source::write0, protocol);
      return ProtocolUtil.read(reader, protocol);
    } finally {
      if (temp != null) {
        ReferenceCountUtil.safeRelease(temp);
      }
    }
  }

  public static ByteBufTProtocol toByteBufProtocol(
      SerializationProtocol protocol, ByteBuf byteBuf) {
    switch (protocol) {
      case TJSON:
        ByteBufTJSONProtocol byteBufTJSONProtocol = new ByteBufTJSONProtocol();
        byteBufTJSONProtocol.wrap(byteBuf);
        return byteBufTJSONProtocol;
      case TSimpleJSON:
        ByteBufDefaultTSimpleJSONProtocol byteBufDefaultTSimpleJSONProtocol =
            new ByteBufDefaultTSimpleJSONProtocol();
        byteBufDefaultTSimpleJSONProtocol.wrap(byteBuf);
        return byteBufDefaultTSimpleJSONProtocol;
      case TSimpleJSONBase64:
        ByteBufBase64TSimpleJSONProtocol byteBufBase64TSimpleJSONProtocol =
            new ByteBufBase64TSimpleJSONProtocol();
        byteBufBase64TSimpleJSONProtocol.wrap(byteBuf);
        return byteBufBase64TSimpleJSONProtocol;
      case TBinary:
        return TProtocolType.TBinary.apply(byteBuf);
      case TCompact:
        return TProtocolType.TCompact.apply(byteBuf);
      default:
        throw new IllegalArgumentException("unsupported protocol: " + protocol);
    }
  }

  public static byte[] toBase64(
      ThriftSerializable writer, SerializationProtocol protocol, boolean urlSafe) {
    ByteBuf base64encoded = null;
    try {
      base64encoded = toBase64Impl(writer, urlSafe, protocol);
      return ByteBufUtil.getBytes(base64encoded);
    } finally {
      ReferenceCountUtil.safeRelease(base64encoded);
    }
  }

  public static String toBase64(
      ThriftSerializable writer, boolean urlSafe, SerializationProtocol protocol) {
    ByteBuf base64encoded = null;
    try {
      base64encoded = toBase64Impl(writer, urlSafe, protocol);
      return base64encoded.toString(StandardCharsets.UTF_8);
    } finally {
      ReferenceCountUtil.safeRelease(base64encoded);
    }
  }

  private static ByteBuf toBase64Impl(
      ThriftSerializable writer, boolean urlSafe, SerializationProtocol protocol) {
    ByteBuf thriftEncoded = RpcResources.getByteBufAllocator().buffer();
    try {
      ByteBufTProtocol tProtocol = toByteBufProtocol(protocol, thriftEncoded);
      writer.write0(tProtocol);

      Base64Dialect dialect = urlSafe ? Base64Dialect.URL_SAFE : Base64Dialect.STANDARD;
      ByteBuf base64encoded = Base64.encode(thriftEncoded, false, dialect);

      return base64encoded;
    } finally {
      ReferenceCountUtil.safeRelease(thriftEncoded);
    }
  }

  public static <T> T fromBase64(
      Reader<T> reader, byte[] base64, boolean urlSafe, SerializationProtocol protocol) {
    ByteBuf utf8Bytes = Unpooled.wrappedBuffer(base64);

    return fromBase64Impl(reader, utf8Bytes, urlSafe, protocol);
  }

  public static <T> T fromBase64(
      Reader<T> reader, String base64String, boolean urlSafe, SerializationProtocol protocol) {
    ByteBuf utf8Bytes = Unpooled.copiedBuffer(base64String, StandardCharsets.UTF_8);

    return fromBase64Impl(reader, utf8Bytes, urlSafe, protocol);
  }

  private static <T> T fromBase64Impl(
      Reader<T> reader, ByteBuf buf, boolean urlSafe, SerializationProtocol protocol) {
    ByteBuf base64decoded = null;
    try {
      Base64Dialect dialect = urlSafe ? Base64Dialect.URL_SAFE : Base64Dialect.STANDARD;
      base64decoded = Base64.decode(buf, dialect);

      ByteBufTProtocol byteBufTProtocol = toByteBufProtocol(protocol, base64decoded);
      return reader.read(byteBufTProtocol);
    } finally {
      ReferenceCountUtil.safeRelease(buf);
      ReferenceCountUtil.safeRelease(base64decoded);
    }
  }
}
