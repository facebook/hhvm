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

package com.facebook.thrift.legacy.server;

import static java.util.Objects.requireNonNull;

import com.facebook.thrift.legacy.codec.HeaderTransportCodec;
import com.facebook.thrift.legacy.codec.LegacyTransportType;
import com.facebook.thrift.legacy.codec.SimpleFrameCodec;
import com.facebook.thrift.protocol.TProtocolType;
import com.google.common.primitives.Ints;
import io.airlift.units.DataSize;
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelPipeline;
import io.netty.handler.codec.ByteToMessageDecoder;
import io.netty.handler.codec.LengthFieldBasedFrameDecoder;
import io.netty.handler.codec.LengthFieldPrepender;
import java.util.List;
import java.util.Optional;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ThriftProtocolDetection extends ByteToMessageDecoder {
  private static final Logger logger = LoggerFactory.getLogger(ThriftProtocolDetection.class);

  private static final int BINARY_PROTOCOL_VERSION_MASK = 0xFFFF_0000;
  private static final int BINARY_PROTOCOL_VERSION_1 = 0x8001_0000;

  private static final int COMPACT_PROTOCOL_VERSION_MASK = 0xFF1F_0000;
  private static final int COMPACT_PROTOCOL_VERSION_2 = 0x8202_0000;

  private static final int HEADER_MAGIC = 0x0FFF_0000;

  private static final int HTTP_POST_MAGIC =
      Ints.fromBytes((byte) 'P', (byte) 'O', (byte) 'S', (byte) 'T');

  private final ThriftServerHandler thriftServerHandler;
  private final DataSize maxFrameSize;
  private final boolean assumeClientsSupportOutOfOrderResponses;

  public ThriftProtocolDetection(
      ThriftServerHandler thriftServerHandler,
      DataSize maxFrameSize,
      boolean assumeClientsSupportOutOfOrderResponses) {
    this.maxFrameSize = requireNonNull(maxFrameSize, "maxFrameSize is null");
    this.thriftServerHandler = requireNonNull(thriftServerHandler, "thriftServerHandler is null");
    this.assumeClientsSupportOutOfOrderResponses = assumeClientsSupportOutOfOrderResponses;
  }

  @Override
  protected void decode(ChannelHandlerContext context, ByteBuf in, List<Object> out)
      throws Exception {
    if (in.readableBytes() < 8) {
      return;
    }

    int magic = in.markReaderIndex().skipBytes(Integer.BYTES).readInt();
    in.resetReaderIndex();

    // HTTP not supported
    if (magic == HTTP_POST_MAGIC) {
      logger.error("http not supported, closing connection");
      in.clear();
      context.close();
      return;
    }

    if ((magic & HEADER_MAGIC) == HEADER_MAGIC) {
      logger.debug("detected header protocol");
      switchToHeader(context);
      return;
    }

    Optional<TProtocolType> protocol = detectProtocol(magic);
    if (!protocol.isPresent()) {
      logger.error("unable to detect protocol, closing connection");
      in.clear();
      context.close();
      return;
    }

    switchToFramed(context, protocol.get());
  }

  private static Optional<TProtocolType> detectProtocol(int magic) {
    if ((magic & BINARY_PROTOCOL_VERSION_MASK) == BINARY_PROTOCOL_VERSION_1) {
      logger.debug("detected frame transport with binary protocol");
      return Optional.of(TProtocolType.TBinary);
    } else if ((magic & COMPACT_PROTOCOL_VERSION_MASK) == COMPACT_PROTOCOL_VERSION_2) {
      logger.debug("detected framed transport with compact protocol");
      return Optional.of(TProtocolType.TCompact);
    } else {
      return Optional.empty();
    }
  }

  private void switchToFramed(ChannelHandlerContext context, TProtocolType protocolType) {
    ChannelPipeline pipeline = context.pipeline();

    pipeline.addLast(
        new LengthFieldBasedFrameDecoder(Integer.MAX_VALUE, 0, Integer.BYTES, 0, Integer.BYTES),
        new LengthFieldPrepender(Integer.BYTES),
        new SimpleFrameCodec(
            LegacyTransportType.FRAMED, protocolType, assumeClientsSupportOutOfOrderResponses),
        thriftServerHandler);

    // remove(this) must be last because it triggers downstream processing of the current message
    pipeline.remove(this);
  }

  private void switchToHeader(ChannelHandlerContext context) {
    ChannelPipeline pipeline = context.pipeline();

    pipeline.addLast(
        new ThriftHeaderFrameLengthBasedDecoder(),
        new LengthFieldPrepender(Integer.BYTES),
        new HeaderTransportCodec(true),
        thriftServerHandler);

    // remove(this) must be last because it triggers downstream processing of the current message
    pipeline.remove(this);
  }
}
