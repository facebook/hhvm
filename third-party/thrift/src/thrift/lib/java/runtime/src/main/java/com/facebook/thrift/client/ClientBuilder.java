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

import com.facebook.thrift.client.v2.manager.ClientOwnership;
import com.facebook.thrift.client.v2.manager.RpcClientBinding;
import com.facebook.thrift.client.v2.manager.SingleRpcClientManager;
import com.google.common.base.Preconditions;
import java.net.SocketAddress;
import java.util.Map;
import org.apache.thrift.ProtocolId;
import reactor.core.publisher.Mono;

/**
 * Abstract builder for typed Thrift service clients.
 *
 * <p>Generated service interfaces expose a static {@code clientBuilder()} method that returns a
 * concrete subclass. Callers configure protocol and headers, then choose a {@code build()} overload
 * to create the client.
 */
public abstract class ClientBuilder<T> {
  protected ProtocolId protocolId = ProtocolId.COMPACT;
  protected Mono<Map<String, String>> headersMono = Mono.empty();
  protected Mono<Map<String, String>> persistentHeadersMono = Mono.empty();

  public ClientBuilder<T> setProtocolId(ProtocolId protocolId) {
    this.protocolId = Preconditions.checkNotNull(protocolId);
    return this;
  }

  public ClientBuilder<T> setHeaders(Map<String, String> headers) {
    this.headersMono = Mono.just(Preconditions.checkNotNull(headers));
    return this;
  }

  public ClientBuilder<T> setPersistentHeaders(Map<String, String> persistentHeaders) {
    this.persistentHeadersMono = Mono.just(Preconditions.checkNotNull(persistentHeaders));
    return this;
  }

  public ClientBuilder<T> setHeadersMono(Mono<Map<String, String>> headersMono) {
    this.headersMono = headersMono;
    return this;
  }

  public ClientBuilder<T> setPersistentHeadersMono(
      Mono<Map<String, String>> persistentHeadersMono) {
    this.persistentHeadersMono = persistentHeadersMono;
    return this;
  }

  /** Builds a typed client from a binding factory and address. */
  public T build(RpcClientFactory factory, SocketAddress address) {
    return build(factory.createRpcClientBinding(address));
  }

  /**
   * Builds a typed client from a raw transport factory and address. The typed client owns a
   * lazily-connected single transport via {@link SingleRpcClientManager}. Use this overload when
   * you've constructed a transport factory directly (e.g., {@code new LegacyRpcClientFactory(...)
   * }) rather than going through {@link RpcClientFactory#builder()}.
   */
  public T build(RpcClientTransportFactory transportFactory, SocketAddress address) {
    return build(
        new RpcClientBinding(
            new SingleRpcClientManager(transportFactory, address), ClientOwnership.OWNED));
  }

  /**
   * Builds a typed client from a pre-constructed {@link RpcClientBinding}. This is the abstract
   * method that generated service builders implement.
   */
  public abstract T build(RpcClientBinding rpcClientBinding);
}
