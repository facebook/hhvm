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
import com.facebook.thrift.util.ReactorHooks;
import java.net.SocketAddress;
import reactor.core.publisher.Mono;

public final class HeaderAwareRpcClientFactory implements RpcClientFactory {
  static {
    ReactorHooks.init();
  }

  private final RSocketRpcClientFactory delegate;

  public HeaderAwareRpcClientFactory(ThriftClientConfig config) {
    this.delegate = new RSocketRpcClientFactory(config);
  }

  @Override
  public Mono<RpcClient> createRpcClient(SocketAddress socketAddress) {

    try {
      return delegate.createRpcClient(socketAddress).map(HeaderAwareRSocketRpcClient::new);
    } catch (Throwable t) {
      return Mono.error(t);
    }
  }

  public void close() {
    delegate.close();
  }
}
