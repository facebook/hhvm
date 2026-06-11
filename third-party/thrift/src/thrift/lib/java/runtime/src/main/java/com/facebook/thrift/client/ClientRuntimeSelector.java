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
import com.facebook.thrift.client.v2.manager.MonoBackedRpcClientManager;
import com.facebook.thrift.client.v2.manager.RpcClientBinding;
import com.facebook.thrift.client.v2.manager.RpcClientManager;
import com.facebook.thrift.client.v2.transport.BindingRpcClientSource;
import reactor.core.publisher.Mono;

/**
 * Helpers for constructing v2 manager-backed {@link RpcClientSource} instances.
 *
 * <p>Raw-Mono callers use {@link #createSource(Mono)} as a compatibility bridge: the typed client
 * gets explicit close semantics, but the underlying Mono keeps owning its own lifecycle. Prefer
 * {@link ClientBuilder#build(RpcClientFactory, java.net.SocketAddress)} for full manager-backed
 * ownership.
 *
 * <p>SR callers that need explicit OWNED/BORROWED semantics use {@link #createOwnedSource} and
 * {@link #createBorrowedSource} directly.
 */
public final class ClientRuntimeSelector {
  private ClientRuntimeSelector() {}

  /** Creates a v2 manager-backed source over a raw {@code Mono<RpcClient>}. */
  public static RpcClientSource createSource(Mono<? extends RpcClient> rpcClientMono) {
    return new BindingRpcClientSource(
        new RpcClientBinding(new MonoBackedRpcClientManager(rpcClientMono), ClientOwnership.OWNED));
  }

  /**
   * Creates an OWNED source from a v2 manager. Closing the typed client will dispose the manager
   * and its underlying transports. Use for clients that have a dedicated connection (e.g., SR
   * sticky clients, standalone non-SR clients).
   */
  public static RpcClientSource createOwnedSource(RpcClientManager manager) {
    return new BindingRpcClientSource(new RpcClientBinding(manager, ClientOwnership.OWNED));
  }

  /**
   * Creates a BORROWED source from a v2 manager. Closing the typed client only closes the handle;
   * the shared manager and its transports remain alive. Use for clients that share a
   * process-lifetime connection (e.g., SR non-sticky sidecar clients, pooled direct-tier clients).
   */
  public static RpcClientSource createBorrowedSource(RpcClientManager manager) {
    return new BindingRpcClientSource(new RpcClientBinding(manager, ClientOwnership.BORROWED));
  }
}
