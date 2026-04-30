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

package com.facebook.thrift.transport.unified;

import static com.google.common.base.Verify.verify;
import static java.nio.charset.StandardCharsets.UTF_8;

import com.facebook.thrift.compression.CompressionManager;
import com.facebook.thrift.legacy.codec.LegacyTransportType;
import com.facebook.thrift.legacy.codec.ThriftFrame;
import com.facebook.thrift.metadata.ClientInfo;
import com.facebook.thrift.protocol.TProtocolType;
import com.google.common.annotations.VisibleForTesting;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.ByteBufUtil;
import io.netty.util.ReferenceCountUtil;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import org.apache.thrift.TApplicationException;
import org.apache.thrift.protocol.TMessage;
import org.apache.thrift.protocol.TMessageType;
import org.apache.thrift.protocol.TProtocol;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.Exceptions;
import reactor.netty.Connection;

public class ReactiveHeaderCodec {
  private static final Logger log = LoggerFactory.getLogger(ReactiveHeaderCodec.class);

  private static final int HEADER_MAGIC = 0x0FFF;
  private static final int ENCODED_INFO_SIZE = 2 * Byte.BYTES;
  private static final int FRAME_HEADER_SIZE =
      Short.BYTES
          + // magic
          Short.BYTES
          + // flags
          Integer.BYTES
          + // sequenceId
          Short.BYTES; // header size

  private static final int FLAGS_NONE = 0;
  private static final int FLAG_SUPPORT_OUT_OF_ORDER = 1;

  private static final int NORMAL_HEADERS = 0x01;
  private static final int PERSISTENT_HEADERS = 0x02;

  /**
   * Encodes the HeaderFrame into a ByteBuf transferring the reference ownership.
   *
   * @param frame frame to be encoded; reference count ownership is transferred to this method
   * @return the encoded frame data; caller is responsible for releasing this buffer
   */
  static ByteBuf encodeFrame(
      ByteBufAllocator alloc, ThriftFrame frame, boolean addClientMetadataHeader) {
    ByteBuf encodedInfo = null;
    ByteBuf encodedHeaders = null;
    ByteBuf frameHeader = null;

    try {
      encodedInfo = alloc.buffer(ENCODED_INFO_SIZE);

      // protocol id
      encodedInfo.writeByte(frame.getProtocol().getProtocolId().getValue());

      // number of "transforms" -- no transforms are supported
      encodedInfo.writeByte(0);

      encodedHeaders = alloc.buffer();

      // This will always be false for the server, see original HeaderTransportCodec for explanation
      if (addClientMetadataHeader) {
        frame.getHeaders().put(ClientInfo.CLIENT_METADATA_HEADER, ClientInfo.getClientMetadata());
      }

      encodeHeaders(NORMAL_HEADERS, frame.getHeaders(), encodedHeaders);
      encodeHeaders(PERSISTENT_HEADERS, frame.getPersistentHeaders(), encodedHeaders);

      final int headerSize = encodedHeaders.readableBytes() + encodedInfo.readableBytes();
      final int paddingSize = getPaddingSize(headerSize);

      // write padding
      encodedHeaders.writeZero(paddingSize);

      final int paddedHeaderSize = headerSize + paddingSize;

      frameHeader = alloc.buffer(FRAME_HEADER_SIZE);

      // header magic
      frameHeader.writeShort(HEADER_MAGIC);

      // flags
      frameHeader.writeShort(
          frame.isSupportOutOfOrderResponse() ? FLAG_SUPPORT_OUT_OF_ORDER : FLAGS_NONE);

      // seq id
      frameHeader.writeInt(frame.getSequenceId());

      // header size
      frameHeader.writeShort(paddedHeaderSize >> 2);

      ByteBuf result =
          alloc
              .compositeBuffer()
              .addComponents(true, frameHeader, encodedInfo, encodedHeaders, frame.getMessage());

      // Success - ownership transferred to composite buffer, null out to prevent release in finally
      frameHeader = null;
      encodedInfo = null;
      encodedHeaders = null;

      return result;
    } catch (Throwable t) {
      // Only release components that weren't successfully added to composite
      ReferenceCountUtil.safeRelease(encodedInfo);
      ReferenceCountUtil.safeRelease(encodedHeaders);
      ReferenceCountUtil.safeRelease(frameHeader);
      if (frame.refCnt() > 0) {
        frame.release();
      }
      throw Exceptions.propagate(t);
    }
  }

  private static int getPaddingSize(int headerSize) {
    return 4 - (headerSize & 0b11); // headerSize & 0b11 is equivalent to headerSize % 4
  }

  private static void encodeHeaders(
      int headerType, Map<String, String> headers, ByteBuf headersBuffer) {

    if (!headers.isEmpty()) {
      writeVarInt32(headersBuffer, headerType);
      writeVarInt32(headersBuffer, headers.size());
      for (Map.Entry<String, String> entry : headers.entrySet()) {
        writeString(headersBuffer, entry.getKey());
        writeString(headersBuffer, entry.getValue());
      }
    }
  }

  private static void writeString(ByteBuf out, String value) {
    final int length = ByteBufUtil.utf8Bytes(value);
    writeVarInt32(out, length);
    ByteBufUtil.reserveAndWriteUtf8(out, value, length);
  }

  private static void writeVarInt32(ByteBuf out, int n) {
    while (true) {
      if ((n & ~0x7F) == 0) {
        out.writeByte(n);
        return;
      }

      out.writeByte(n | 0x80);
      n >>>= 7;
    }
  }

  /**
   * Decodes the ByteBuf into a HeaderFrame. The buffer is consumed and either: - Retained in the
   * returned ThriftFrame (if no compression), or - Released and replaced with an inflated buffer
   * (if compressed) In both cases, the caller must release the returned ThriftFrame to free memory.
   *
   * @param buffer buffer to be decoded; reference is consumed by this method
   * @return the decoded frame; caller is responsible for releasing this object
   */
  @VisibleForTesting
  static ThriftFrame decodeFrame(ByteBufAllocator alloc, ByteBuf buffer) {
    short magic = buffer.readShort();
    verify(magic == HEADER_MAGIC, "Invalid header magic");

    // read flags
    short flags = buffer.readShort();
    boolean outOfOrderResponse;
    switch (flags) {
      case FLAGS_NONE:
        outOfOrderResponse = false;
        break;
      case FLAG_SUPPORT_OUT_OF_ORDER:
        outOfOrderResponse = true;
        break;
      default:
        throw new IllegalArgumentException("Unsupported header flags: " + flags);
    }

    // read seq id
    int sequenceId = buffer.readInt();

    // read header size
    int headerSize = buffer.readUnsignedShort() << 2;
    ByteBuf messageHeader = buffer.readSlice(headerSize);

    // read protocol id
    int protocolId = readVarInt32(messageHeader);
    TProtocolType protocolType = TProtocolType.fromProtocolId(protocolId);

    // read transforms
    int numberOfTransforms = readVarInt32(messageHeader);
    int[] transforms = new int[numberOfTransforms];
    for (int i = 0; i < numberOfTransforms; i++) {
      transforms[i] = readVarInt32(messageHeader);
    }

    // Persistent headers override normal headers on key collision (consistent with Netty 3).
    Map<String, String> allHeaders = new HashMap<>();
    Map<String, String> persistentHeaders = new HashMap<>();
    while (messageHeader.readableBytes() > 0) {
      int infoId = readVarInt32(messageHeader);
      if (infoId == NORMAL_HEADERS) {
        decodeHeaders(messageHeader, allHeaders);
      } else if (infoId == PERSISTENT_HEADERS) {
        decodeHeaders(messageHeader, persistentHeaders);
      } else {
        // padding (0) or unknown info id
        break;
      }
    }
    allHeaders.putAll(persistentHeaders);

    // Apply decompression in reverse order, matching C++ THeader::untransform()
    for (int i = transforms.length - 1; i >= 0; i--) {
      buffer = CompressionManager.decompressFromTransform(transforms[i], alloc, buffer);
    }

    ThriftFrame frame =
        new ThriftFrame(
            sequenceId,
            buffer,
            allHeaders,
            persistentHeaders,
            LegacyTransportType.HEADER,
            protocolType,
            outOfOrderResponse);

    return frame;
  }

  private static void decodeHeaders(ByteBuf messageHeader, Map<String, String> headers) {
    int headerCount = readVarInt32(messageHeader);
    for (int i = 0; i < headerCount; i++) {
      String key = readString(messageHeader);
      String value = null;
      try {
        value = readString(messageHeader);
      } catch (Exception e) {
        log.error("Failed to read header {}", key, e);
        throw e;
      }
      headers.put(key, value);
    }
  }

  private static String readString(ByteBuf buffer) {
    int length = readVarInt32(buffer);
    if (length < 0 || length > buffer.readableBytes()) {
      throw new IllegalArgumentException(
          String.format(
              "Header string length %d exceeds available bytes %d",
              length, buffer.readableBytes()));
    }
    return buffer.readCharSequence(length, UTF_8).toString();
  }

  private static int readVarInt32(ByteBuf buffer) {
    int result = 0;
    int shift = 0;

    while (true) {
      byte b = buffer.readByte();
      result |= (b & 0x7f) << shift;
      if ((b & 0x80) != 0x80) {
        break;
      }
      shift += 7;
    }

    return result;
  }

  /**
   * Encodes a TApplicationException into a ThriftFrame.
   *
   * <p>This is a utility method for creating exception response frames.
   *
   * @param conn The Reactor Netty connection
   * @param methodName The method name for the exception
   * @param transport The transport type
   * @param protocol The protocol type
   * @param sequenceId The sequence ID
   * @param supportOutOfOrderResponse Whether out-of-order responses are supported
   * @param errorCode The TApplicationException error code
   * @param errorMessage The error message
   * @param responseHeaders The response headers
   * @return A ThriftFrame containing the encoded exception
   */
  static ThriftFrame encodeApplicationException(
      Connection conn,
      String methodName,
      LegacyTransportType transport,
      TProtocolType protocol,
      int sequenceId,
      boolean supportOutOfOrderResponse,
      int errorCode,
      String errorMessage,
      Map<String, String> responseHeaders) {

    ByteBuf buf = conn.channel().alloc().buffer();
    try {
      TApplicationException applicationException =
          new TApplicationException(errorCode, errorMessage);

      TProtocol out = protocol.apply(buf);

      out.writeMessageBegin(new TMessage(methodName, TMessageType.EXCEPTION, sequenceId));
      applicationException.write(out);
      out.writeMessageEnd();

      return new ThriftFrame(
          sequenceId,
          buf,
          responseHeaders,
          Collections.emptyMap(),
          transport,
          protocol,
          supportOutOfOrderResponse);
    } catch (Throwable t) {
      ReferenceCountUtil.safeRelease(buf);
      throw t;
    }
  }
}
