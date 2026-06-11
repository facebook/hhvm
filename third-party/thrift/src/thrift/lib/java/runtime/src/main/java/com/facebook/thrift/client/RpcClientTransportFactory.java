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

import java.net.SocketAddress;
import reactor.core.publisher.Mono;

/**
 * Lower-level contract for creating a raw {@link RpcClient} connection for a given address.
 *
 * <p>Implementations are the transport-layer factories that produce a connecting {@code
 * Mono<RpcClient>} (e.g., {@code LegacyRpcClientFactory}, {@code RSocketRpcClientFactory}, plus the
 * per-connection decorators that wrap them). Managers (e.g., {@code SingleRpcClientManager}) call
 * into this contract when they need to establish a new connection.
 *
 * <p>This is distinct from {@link RpcClientFactory}, which is the higher-level binding factory used
 * by typed-client construction.
 */
@FunctionalInterface
public interface RpcClientTransportFactory {
  Mono<RpcClient> createRpcClient(SocketAddress socketAddress);
}
