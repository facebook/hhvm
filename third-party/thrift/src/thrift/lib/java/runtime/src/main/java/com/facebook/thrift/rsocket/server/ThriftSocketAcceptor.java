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

import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.util.resources.RpcResources;
import io.rsocket.ConnectionSetupPayload;
import io.rsocket.RSocket;
import io.rsocket.SocketAcceptor;
import reactor.core.publisher.Mono;

public class ThriftSocketAcceptor implements SocketAcceptor {
  private final RpcServerHandler serverHandler;

  public ThriftSocketAcceptor(RpcServerHandler serverHandler) {
    this.serverHandler = serverHandler;
  }

  @Override
  public Mono<RSocket> accept(ConnectionSetupPayload setup, RSocket sendingSocket) {
    ThriftServerRSocket rSocket =
        new ThriftServerRSocket(serverHandler, RpcResources.getByteBufAllocator());

    return Mono.just(rSocket);
  }
}
