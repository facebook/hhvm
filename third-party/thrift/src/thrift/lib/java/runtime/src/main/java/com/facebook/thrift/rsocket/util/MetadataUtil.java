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

package com.facebook.thrift.rsocket.util;

import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.protocol.TProtocolType;
import io.rsocket.Payload;
import org.apache.thrift.RequestRpcMetadata;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.StreamPayloadMetadata;
import org.apache.thrift.TException;

/** Helper class used to encode and decode RSocket metadata. */
public final class MetadataUtil {
  private MetadataUtil() {}

  public static ResponseRpcMetadata decodePayloadMetadata(Payload payload) throws TException {
    if (!payload.hasMetadata()) {
      return ResponseRpcMetadata.defaultInstance();
    }

    final ByteBufTProtocol protocol = TProtocolType.TCompact.apply(payload.sliceMetadata());
    return ResponseRpcMetadata.read0(protocol);
  }

  public static StreamPayloadMetadata decodeStreamingPayloadMetadata(Payload payload)
      throws TException {
    if (!payload.hasMetadata()) {
      return StreamPayloadMetadata.defaultInstance();
    }

    final ByteBufTProtocol protocol = TProtocolType.TCompact.apply(payload.sliceMetadata());

    return StreamPayloadMetadata.read0(protocol);
  }

  public static RequestRpcMetadata decodeRequestRpcMetadata(Payload payload) throws TException {
    if (!payload.hasMetadata()) {
      return RequestRpcMetadata.defaultInstance();
    }

    final ByteBufTProtocol protocol = TProtocolType.TCompact.apply(payload.sliceMetadata());
    return RequestRpcMetadata.read0(protocol);
  }
}
