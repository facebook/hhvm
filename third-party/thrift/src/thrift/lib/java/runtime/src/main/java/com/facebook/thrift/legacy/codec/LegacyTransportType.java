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

import com.facebook.thrift.legacy.client.ThriftClientHandler;
import com.facebook.thrift.metadata.ClientInfo;
import com.facebook.thrift.protocol.TProtocolType;
import io.airlift.units.DataSize;
import io.netty.channel.ChannelPipeline;
import io.netty.handler.codec.LengthFieldBasedFrameDecoder;
import io.netty.handler.codec.LengthFieldPrepender;
import io.netty.handler.flush.FlushConsolidationHandler;
import io.netty.handler.logging.LogLevel;
import io.netty.handler.logging.LoggingHandler;
import java.util.Optional;

public enum LegacyTransportType {
  FRAMED {
    @Override
    public void addFrameHandlers(
        ChannelPipeline pipeline,
        Optional<TProtocolType> protocol,
        DataSize maxFrameSize,
        boolean assumeClientsSupportOutOfOrderResponses) {
      TProtocolType protocolType =
          protocol.orElseThrow(
              () -> new IllegalArgumentException("FRAMED transport requires a protocol"));

      pipeline.addLast(
          new LoggingHandler(LogLevel.TRACE),
          new FlushConsolidationHandler(256, true),
          new LengthFieldBasedFrameDecoder(Integer.MAX_VALUE, 0, Integer.BYTES, 0, Integer.BYTES),
          new LengthFieldPrepender(Integer.BYTES),
          new SimpleFrameCodec(this, protocolType, assumeClientsSupportOutOfOrderResponses),
          new ThriftClientHandler(LegacyTransportType.FRAMED));
    }
  },
  HEADER {
    @Override
    public void addFrameHandlers(
        ChannelPipeline pipeline,
        Optional<TProtocolType> protocol,
        DataSize maxFrameSize,
        boolean assumeClientsSupportOutOfOrderResponses) {

      pipeline.addLast(
          new LoggingHandler(LogLevel.TRACE),
          new FlushConsolidationHandler(256, true),
          new LengthFieldBasedFrameDecoder(Integer.MAX_VALUE, 0, Integer.BYTES, 0, Integer.BYTES),
          new LengthFieldPrepender(Integer.BYTES),
          new HeaderTransportCodec(false),
          new ThriftClientHandler(LegacyTransportType.HEADER));

      ClientInfo.addTransport(ClientInfo.Transport.HEADER);
    }
  };

  public abstract void addFrameHandlers(
      ChannelPipeline pipeline,
      Optional<TProtocolType> protocol,
      DataSize maxFrameSize,
      boolean assumeClientsSupportOutOfOrderResponses);
}
