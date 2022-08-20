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

package com.facebook.thrift.rsocket.client;

import com.facebook.thrift.client.RpcClient;
import com.facebook.thrift.client.RpcClientFactory;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.metadata.ThriftTransportType;
import com.facebook.thrift.rsocket.transport.reactor.client.ReactorClientTransport;
import com.facebook.thrift.util.ReactorHooks;
import com.facebook.thrift.util.RpcClientUtils;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.channel.EventLoopGroup;
import io.rsocket.core.RSocketConnector;
import io.rsocket.frame.decoder.PayloadDecoder;
import java.net.SocketAddress;
import reactor.core.publisher.Mono;

public class RSocketRpcClientFactory implements RpcClientFactory {
  static {
    ReactorHooks.init();
  }

  private final ThriftClientConfig config;
  private final EventLoopGroup group;
  private final RSocketConnector connector;

  public RSocketRpcClientFactory(ThriftClientConfig config) {
    this(config, RpcClientUtils.createRSocketConnector());
  }

  public RSocketRpcClientFactory(ThriftClientConfig config, RSocketConnector connector) {
    this.config = config;
    this.group = RpcResources.getEventLoopGroup();
    this.connector = connector;
  }

  @Override
  public Mono<RpcClient> createRpcClient(SocketAddress socketAddress) {

    try {
      final ReactorClientTransport transport =
          new ReactorClientTransport(socketAddress, this.config, ThriftTransportType.RSOCKET);

      return connector
          .payloadDecoder(PayloadDecoder.ZERO_COPY)
          .connect(transport)
          .map(RSocketRpcClient::new);
    } catch (Throwable t) {
      return Mono.error(t);
    }
  }

  public void close() {
    group.shutdownGracefully();
  }
}
