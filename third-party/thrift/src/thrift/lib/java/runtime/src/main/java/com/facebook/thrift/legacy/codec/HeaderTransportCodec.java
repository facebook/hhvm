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

package com.facebook.thrift.legacy.codec;

import static com.google.common.base.Verify.verify;
import static java.nio.charset.StandardCharsets.UTF_8;

import com.facebook.thrift.metadata.ClientInfo;
import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.util.CompressionUtil;
import com.google.common.annotations.VisibleForTesting;
import com.google.common.collect.ImmutableMap;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.ByteBufUtil;
import io.netty.channel.ChannelDuplexHandler;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelPromise;
import io.netty.util.ReferenceCountUtil;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.atomic.AtomicLong;
import reactor.core.Exceptions;

public final class HeaderTransportCodec extends ChannelDuplexHandler {
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

  private static final int ZLIB_TRANSFORM = 0x01;

  private static final long LOG_INTERVAL = 30_000_000_000L; // 30 sec
  private static final AtomicLong logTime = new AtomicLong(0);

  private final long jitter = ThreadLocalRandom.current().nextLong(LOG_INTERVAL / 10);
  private final long INTERVAL = LOG_INTERVAL + jitter; // between 30-33 sec

  private long time = System.nanoTime() + jitter;
  private final boolean isServer;

  public HeaderTransportCodec(boolean isServer) {
    this.isServer = isServer;
  }

  @Override
  public void channelRead(ChannelHandlerContext context, Object message) {
    if (message instanceof ByteBuf) {
      ByteBuf request = (ByteBuf) message;
      ThriftFrame frame = null;
      try {
        if (request.isReadable()) {
          frame = decodeFrame(context.alloc(), request);
        }
      } catch (Throwable t) {
        ReferenceCountUtil.safeRelease(request);
        context.fireExceptionCaught(t);
        return;
      }
      if (frame != null) {
        context.fireChannelRead(frame);
        return;
      }
    }

    context.fireChannelRead(message);
  }

  @Override
  public void write(ChannelHandlerContext context, Object message, ChannelPromise promise) {
    if (message instanceof ThriftFrame) {
      boolean addHeader = !isServer & addHeader();
      ByteBuf encodedFrame = encodeFrame(context.alloc(), (ThriftFrame) message, addHeader);
      context.writeAndFlush(encodedFrame, promise);
    } else {
      context.writeAndFlush(message, promise);
    }
  }

  private boolean addHeader() {
    long now = System.nanoTime();
    if (now - time > INTERVAL) {
      long curr = logTime.get();
      time = now;
      return logTime.compareAndSet(curr, curr + INTERVAL);
    }

    return false;
  }

  /**
   * Encodes the HeaderFrame into a ByteBuf transferring the reference ownership.
   *
   * @param frame frame to be encoded; reference count ownership is transferred to this method
   * @return the encoded frame data; caller is responsible for releasing this buffer
   */
  private static ByteBuf encodeFrame(ByteBufAllocator alloc, ThriftFrame frame, boolean addHeader) {
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

      if (addHeader) {
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

      return alloc
          .compositeBuffer()
          .addComponents(true, frameHeader, encodedInfo, encodedHeaders, frame.getMessage());
    } catch (Throwable t) {
      if (encodedInfo != null && encodedInfo.refCnt() > 0) {
        encodedInfo.release();
      }

      if (encodedHeaders != null && encodedHeaders.refCnt() > 0) {
        encodedHeaders.release();
      }

      if (frameHeader != null && frameHeader.refCnt() > 0) {
        frameHeader.release();
      }

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
   * Decodes the ByteBuf into a HeaderFrame transferring the reference ownership.
   *
   * @param buffer buffer to be decoded; reference count ownership is transferred to this method
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
    int headerSize = buffer.readShort() << 2;
    ByteBuf messageHeader = buffer.readSlice(headerSize);

    // read protocol id
    int protocolId = readVarInt32(messageHeader);
    TProtocolType protocolType = TProtocolType.fromProtocolId(protocolId);

    // read transforms
    int numberOfTransforms = readVarInt32(messageHeader);
    boolean inflate = false;
    if (numberOfTransforms > 0) {
      // only support zlib transformation
      int transform = readVarInt32(messageHeader);
      inflate = ZLIB_TRANSFORM == transform;

      if (!inflate) {
        throw new IllegalArgumentException("Unsupported transform -> " + transform);
      }
    }

    // headers
    Map<String, String> normalHeaders = decodeHeaders(NORMAL_HEADERS, messageHeader);
    Map<String, String> persistentHeaders = decodeHeaders(PERSISTENT_HEADERS, messageHeader);

    // NOTE: currently, if normal headers and persistent headers contains the same key.
    // persistent header will override normal header. This is consistent with Netty 3 implementation
    Map<String, String> allHeaders = new HashMap<>();
    allHeaders.putAll(normalHeaders);
    allHeaders.putAll(persistentHeaders);

    if (inflate) {
      buffer = CompressionUtil.inflate(alloc, buffer);
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

  private static Map<String, String> decodeHeaders(int expectedHeadersType, ByteBuf messageHeader) {
    if (messageHeader.readableBytes() == 0) {
      return Collections.emptyMap();
    }

    byte headersType = messageHeader.readByte();
    if (headersType != expectedHeadersType) {
      return Collections.emptyMap();
    }

    ImmutableMap.Builder<String, String> headers = ImmutableMap.builder();
    int headerCount = readVarInt32(messageHeader);
    for (int i = 0; i < headerCount; i++) {
      String key = readString(messageHeader);
      String value = readString(messageHeader);
      headers.put(key, value);
    }
    return headers.build();
  }

  private static String readString(ByteBuf buffer) {
    int length = readVarInt32(buffer);
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
}
