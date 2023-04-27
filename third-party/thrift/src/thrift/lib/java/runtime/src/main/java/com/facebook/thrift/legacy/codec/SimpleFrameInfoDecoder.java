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

import static java.util.Objects.requireNonNull;

import com.facebook.thrift.protocol.TProtocolType;
import io.netty.buffer.ByteBuf;
import java.util.Optional;
import org.apache.thrift.protocol.TMessage;
import org.apache.thrift.protocol.TProtocol;

public class SimpleFrameInfoDecoder implements FrameInfoDecoder {
  private final LegacyTransportType legacyTransportType;
  private final TProtocolType protocolType;
  private final boolean assumeClientsSupportOutOfOrderResponses;

  public SimpleFrameInfoDecoder(
      LegacyTransportType legacyTransportType,
      TProtocolType protocolType,
      boolean assumeClientsSupportOutOfOrderResponses) {
    this.legacyTransportType = requireNonNull(legacyTransportType, "transportType is null");
    this.protocolType = requireNonNull(protocolType, "protocolType is null");
    this.assumeClientsSupportOutOfOrderResponses = assumeClientsSupportOutOfOrderResponses;
  }

  @Override
  public Optional<FrameInfo> tryDecodeFrameInfo(ByteBuf buffer) {
    try {
      TProtocol protocol = protocolType.apply(buffer.markReaderIndex());
      TMessage message;
      try {
        message = protocol.readMessageBegin();
      } catch (RuntimeException e) {
        return Optional.empty();
      }
      return Optional.of(
          new FrameInfo(
              message.name,
              message.type,
              message.seqid,
              legacyTransportType,
              protocolType,
              assumeClientsSupportOutOfOrderResponses));
    } finally {
      buffer.resetReaderIndex();
    }
  }
}
