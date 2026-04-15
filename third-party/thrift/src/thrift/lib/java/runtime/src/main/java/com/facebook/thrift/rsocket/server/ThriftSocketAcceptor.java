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

package com.facebook.thrift.rsocket.server;

import com.facebook.thrift.payload.Constants;
import com.facebook.thrift.protocol.ProtocolUtil;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.rsocket.ConnectionSetupPayload;
import io.rsocket.Payload;
import io.rsocket.RSocket;
import io.rsocket.SocketAcceptor;
import io.rsocket.exceptions.UnsupportedSetupException;
import io.rsocket.util.ByteBufPayload;
import org.apache.thrift.ServerPushMetadata;
import org.apache.thrift.SetupResponse;
import reactor.core.publisher.Mono;

public class ThriftSocketAcceptor implements SocketAcceptor {

  private final RpcServerHandler serverHandler;

  public ThriftSocketAcceptor(RpcServerHandler serverHandler) {
    this.serverHandler = serverHandler;
  }

  @Override
  public Mono<RSocket> accept(ConnectionSetupPayload setup, RSocket sendingSocket) {
    String metadataMimeType = setup.metadataMimeType();
    if (!metadataMimeType.equals(Constants.ROCKET_METADATA_COMPACT_MIME_TYPE)) {
      throw new UnsupportedSetupException("Unsupported Metadata Mime Type: " + metadataMimeType);
    }

    ThriftServerRSocket rSocket =
        new ThriftServerRSocket(serverHandler, RpcResources.getByteBufAllocator());

    return sendSetupResponse(sendingSocket).thenReturn(rSocket);
  }

  /**
   * Sends a ServerPushMetadata with SetupResponse to the client, advertising compression support.
   * Matches C++ ThriftRocketServerHandler which sends zstdSupported=true during connection setup.
   */
  private static Mono<Void> sendSetupResponse(RSocket sendingSocket) {
    SetupResponse setupResponse = new SetupResponse.Builder().setZstdSupported(true).build();
    ServerPushMetadata serverPushMetadata = ServerPushMetadata.fromSetupResponse(setupResponse);

    ByteBuf metadata = RpcResources.getByteBufAllocator().buffer();
    ProtocolUtil.writeCompact(serverPushMetadata::write0, metadata);

    Payload payload = ByteBufPayload.create(Unpooled.EMPTY_BUFFER, metadata);
    return sendingSocket.metadataPush(payload);
  }
}
