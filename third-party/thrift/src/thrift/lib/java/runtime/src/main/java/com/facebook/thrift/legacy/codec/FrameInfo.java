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

import static com.google.common.base.MoreObjects.toStringHelper;
import static java.util.Objects.requireNonNull;

import com.facebook.thrift.protocol.TProtocolType;
import java.util.Objects;

public class FrameInfo {
  private final String methodName;
  private final byte messageType;
  private final int sequenceId;
  private final LegacyTransportType transport;
  private final TProtocolType protocol;
  private final boolean supportOutOfOrderResponse;

  public FrameInfo(
      String methodName,
      byte messageType,
      int sequenceId,
      LegacyTransportType transport,
      TProtocolType protocol,
      boolean supportOutOfOrderResponse) {
    this.methodName = requireNonNull(methodName, "methodName is null");
    this.messageType = messageType;
    this.sequenceId = sequenceId;
    this.transport = requireNonNull(transport, "transport is null");
    this.protocol = requireNonNull(protocol, "protocol is null");
    this.supportOutOfOrderResponse = supportOutOfOrderResponse;
  }

  public String getMethodName() {
    return methodName;
  }

  public byte getMessageType() {
    return messageType;
  }

  public int getSequenceId() {
    return sequenceId;
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
  public boolean equals(Object o) {
    if (this == o) {
      return true;
    }
    if (o == null || getClass() != o.getClass()) {
      return false;
    }
    FrameInfo that = (FrameInfo) o;
    return messageType == that.messageType
        && sequenceId == that.sequenceId
        && supportOutOfOrderResponse == that.supportOutOfOrderResponse
        && Objects.equals(methodName, that.methodName)
        && transport == that.transport
        && protocol == that.protocol;
  }

  @Override
  public int hashCode() {
    return Objects.hash(
        methodName, messageType, sequenceId, transport, protocol, supportOutOfOrderResponse);
  }

  @Override
  public String toString() {
    return toStringHelper(this)
        .add("methodName", methodName)
        .add("messageType", messageType)
        .add("sequenceId", sequenceId)
        .add("transport", transport)
        .add("protocol", protocol)
        .add("supportOutOfOrderResponse", supportOutOfOrderResponse)
        .toString();
  }
}
