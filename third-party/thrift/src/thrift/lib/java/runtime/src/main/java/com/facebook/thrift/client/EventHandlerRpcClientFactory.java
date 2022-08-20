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

package com.facebook.thrift.client;

import com.facebook.swift.service.ThriftClientEventHandler;
import java.net.SocketAddress;
import java.util.List;
import reactor.core.publisher.Mono;

public class EventHandlerRpcClientFactory implements RpcClientFactory {
  private final RpcClientFactory delegate;
  private final List<? extends ThriftClientEventHandler> eventHandlers;

  public EventHandlerRpcClientFactory(
      RpcClientFactory delegate, List<? extends ThriftClientEventHandler> eventHandlers) {
    this.delegate = delegate;
    this.eventHandlers = eventHandlers;
  }

  @Override
  public Mono<RpcClient> createRpcClient(SocketAddress socketAddress) {
    return delegate
        .createRpcClient(socketAddress)
        .map(rpcClient -> new EventHandlerRpcClient(rpcClient, eventHandlers));
  }
}
