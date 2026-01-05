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

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.server.ServerTransportFactory;
import com.facebook.thrift.util.SPINiftyMetrics;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import reactor.core.publisher.Mono;

public class LegacyServerTransportFactory implements ServerTransportFactory<LegacyServerTransport> {
  private final ThriftServerConfig config;

  public LegacyServerTransportFactory(ThriftServerConfig config) {
    this.config = config;
  }

  @Override
  public Mono<? extends LegacyServerTransport> createServerTransport(
      SocketAddress bindAddress, RpcServerHandler rpcServerHandler, SPINiftyMetrics serverMetrics) {
    return LegacyServerTransport.createNewInstance(
        bindAddress, rpcServerHandler, config, serverMetrics);
  }

  public Mono<? extends LegacyServerTransport> createServerTransport(
      RpcServerHandler rpcServerHandler) {
    return createServerTransport(
        new InetSocketAddress("localhost", config.getPort()),
        rpcServerHandler,
        new SPINiftyMetrics());
  }
}
