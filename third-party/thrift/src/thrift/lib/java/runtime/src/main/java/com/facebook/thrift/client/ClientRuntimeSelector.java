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
import com.facebook.thrift.client.v2.manager.RpcClientManager;
import com.facebook.thrift.client.v2.transport.BindingRpcClientSource;

/**
 * Helpers for constructing manager-backed {@link RpcClientSource} instances with explicit
 * OWNED/BORROWED ownership semantics.
 */
public final class ClientRuntimeSelector {
  private ClientRuntimeSelector() {}

  /**
   * Creates an OWNED source from a manager. Closing the typed client will dispose the manager and
   * its underlying transports. Use for clients that have a dedicated connection (e.g., SR sticky
   * clients, standalone non-SR clients).
   */
  public static RpcClientSource createOwnedSource(RpcClientManager manager) {
    return new BindingRpcClientSource(new RpcClientBinding(manager, ClientOwnership.OWNED));
  }

  /**
   * Creates a BORROWED source from a manager. Closing the typed client only closes the handle; the
   * shared manager and its transports remain alive. Use for clients that share a process-lifetime
   * connection (e.g., SR non-sticky sidecar clients, pooled direct-tier clients).
   */
  public static RpcClientSource createBorrowedSource(RpcClientManager manager) {
    return new BindingRpcClientSource(new RpcClientBinding(manager, ClientOwnership.BORROWED));
  }
}
