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

import io.netty.buffer.ByteBuf;
import java.util.function.Function;
import org.apache.thrift.ProtocolId;

/** Enum that can gets a {@link TProtocolType} that can found by a {@link ProtocolId} */
public enum TProtocolType implements Function<ByteBuf, ByteBufTProtocol> {
  TBinary(0) {
    @Override
    public ByteBufTProtocol apply(final ByteBuf byteBuf) {
      final ByteBufTBinaryProtocol protocol = new ByteBufTBinaryProtocol();
      protocol.wrap(byteBuf);
      return protocol;
    }
  },
  TCompact(2) {
    @Override
    public ByteBufTProtocol apply(final ByteBuf byteBuf) {
      final ByteBufTProtocol protocol = new ByteBufTCompactProtocol();
      protocol.wrap(byteBuf);
      return protocol;
    }
  };

  private final int protocolId;

  TProtocolType(final int protocolId) {
    this.protocolId = protocolId;
  }

  public static TProtocolType fromProtocolId(final ProtocolId protocolId) {
    if (protocolId == ProtocolId.BINARY) {
      return TBinary;
    } else if (protocolId == ProtocolId.COMPACT) {
      return TCompact;
    } else {
      throw new IllegalArgumentException("unknown protocol id " + protocolId);
    }
  }

  public static TProtocolType fromProtocolId(final byte id) {
    final ProtocolId protocolId = ProtocolId.fromInteger(id);
    return fromProtocolId(protocolId);
  }

  public static TProtocolType fromProtocolId(final int id) {
    return fromProtocolId((byte) id);
  }

  public ProtocolId getProtocolId() {
    return ProtocolId.fromInteger(protocolId);
  }
}
