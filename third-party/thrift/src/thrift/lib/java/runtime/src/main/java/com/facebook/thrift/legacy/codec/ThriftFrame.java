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
import static javax.annotation.meta.When.UNKNOWN;

import com.facebook.thrift.protocol.TProtocolType;
import io.netty.buffer.ByteBuf;
import io.netty.util.ReferenceCounted;
import java.util.Map;
import javax.annotation.CheckReturnValue;

public class ThriftFrame implements ReferenceCounted {
  private final int sequenceId;
  private final ByteBuf message;
  private final Map<String, String> headers;
  private final Map<String, String> persistentHeaders;
  private final LegacyTransportType transport;
  private final TProtocolType protocol;
  private final boolean supportOutOfOrderResponse;

  public ThriftFrame(
      int sequenceId,
      ByteBuf message,
      Map<String, String> headers,
      Map<String, String> persistentHeaders,
      LegacyTransportType transport,
      TProtocolType protocol,
      boolean supportOutOfOrderResponse) {
    this.sequenceId = sequenceId;
    this.message = requireNonNull(message, "message is null");
    this.headers = requireNonNull(headers, "headers is null");
    this.persistentHeaders = requireNonNull(persistentHeaders, "headers is null");
    this.transport = requireNonNull(transport, "transport is null");
    this.protocol = requireNonNull(protocol, "protocol is null");
    this.supportOutOfOrderResponse = supportOutOfOrderResponse;
  }

  public int getSequenceId() {
    return sequenceId;
  }

  public ByteBuf getMessage() {
    return message;
  }

  public Map<String, String> getHeaders() {
    return headers;
  }

  public Map<String, String> getPersistentHeaders() {
    return persistentHeaders;
  }

  public LegacyTransportType getTransport() {
    return transport;
  }

  public TProtocolType getProtocol() {
    return protocol;
  }

  public boolean isSupportOutOfOrderResponse() {
    return supportOutOfOrderResponse;
  }

  @Override
  public int refCnt() {
    return message.refCnt();
  }

  @Override
  public ThriftFrame retain() {
    message.retain();
    return this;
  }

  @Override
  public ThriftFrame retain(int increment) {
    message.retain(increment);
    return this;
  }

  @Override
  public ThriftFrame touch() {
    message.touch();
    return this;
  }

  @Override
  public ThriftFrame touch(Object hint) {
    message.touch(hint);
    return this;
  }

  @CheckReturnValue(when = UNKNOWN)
  @Override
  public boolean release() {
    return message.release();
  }

  @Override
  public boolean release(int decrement) {
    return message.release(decrement);
  }
}
