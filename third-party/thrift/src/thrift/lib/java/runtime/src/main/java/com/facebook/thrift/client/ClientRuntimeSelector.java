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
import com.facebook.thrift.util.resources.RpcResources;
import reactor.core.publisher.Mono;

/**
 * Centralizes legacy-vs-v2 client-source selection.
 *
 * <p>Every path that creates an {@link RpcClientSource} for a generated typed client goes through
 * this class. It inspects the active {@link ClientRuntimeMode} (from per-config or global JVM flag)
 * and returns either a {@link LegacyRpcClientSource} (no-op dispose, preserves historical behavior)
 * or a {@link BindingRpcClientSource} (v2 manager-backed, with explicit close semantics).
 *
 * <p>SR callers that need explicit OWNED/BORROWED semantics use {@link #createOwnedSource} and
 * {@link #createBorrowedSource} directly, bypassing mode resolution.
 */
public final class ClientRuntimeSelector {
  private ClientRuntimeSelector() {}

  public static ClientRuntimeMode resolve(ThriftClientConfig thriftClientConfig) {
    if (thriftClientConfig != null && thriftClientConfig.getClientRuntimeMode() != null) {
      return thriftClientConfig.getClientRuntimeMode();
    }
    return resolveGlobal();
  }

  public static ClientRuntimeMode resolveGlobal() {
    return RpcResources.getClientRuntimeMode();
  }

  /**
   * Creates a source for a raw {@code Mono<RpcClient>}, using the global runtime mode.
   *
   * <p>In v2 mode this remains a compatibility bridge: the typed client gets explicit close
   * semantics, but the underlying Mono keeps owning its own lifecycle. Prefer {@link
   * ClientBuilder#build(RpcClientFactory, java.net.SocketAddress)} for full manager-backed
   * ownership.
   */
  public static RpcClientSource createSource(Mono<? extends RpcClient> rpcClientMono) {
    return createSource(rpcClientMono, resolveGlobal());
  }

  /** Creates a source for a raw Mono with an explicit runtime mode override. */
  public static RpcClientSource createSource(
      Mono<? extends RpcClient> rpcClientMono, ClientRuntimeMode runtimeMode) {
    if (runtimeMode == ClientRuntimeMode.V2) {
      return new BindingRpcClientSource(
          new RpcClientBinding(
              new MonoBackedRpcClientManager(rpcClientMono), ClientOwnership.OWNED));
    }
    return new LegacyRpcClientSource(rpcClientMono);
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
