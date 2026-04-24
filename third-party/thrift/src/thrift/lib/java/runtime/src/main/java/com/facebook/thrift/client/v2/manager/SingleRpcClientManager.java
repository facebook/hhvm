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

package com.facebook.thrift.client.v2.manager;

import com.facebook.thrift.client.RpcClient;
import com.facebook.thrift.client.RpcClientFactory;
import java.net.SocketAddress;
import java.util.Objects;
import java.util.concurrent.atomic.AtomicReferenceFieldUpdater;
import reactor.core.publisher.Mono;

/**
 * Owns one concrete address and at most one live transport at a time.
 *
 * <p>On {@link #acquire()}, if a live client already exists it is reused. Otherwise a new client is
 * created through the transport factory and cached. When that transport later closes, the next
 * {@code acquire()} lazily creates a replacement.
 */
public final class SingleRpcClientManager extends AbstractRpcClientManager {
  private static final AtomicReferenceFieldUpdater<SingleRpcClientManager, RpcClient> RPC_CLIENT =
      AtomicReferenceFieldUpdater.newUpdater(
          SingleRpcClientManager.class, RpcClient.class, "rpcClient");

  private final RpcClientFactory transportFactory;
  private final SocketAddress socketAddress;
  private final Object lock = new Object();

  private volatile RpcClient rpcClient;
  private volatile Mono<RpcClient> connecting;

  public SingleRpcClientManager(RpcClientFactory transportFactory, SocketAddress socketAddress) {
    this.transportFactory = Objects.requireNonNull(transportFactory);
    this.socketAddress = Objects.requireNonNull(socketAddress);
  }

  /**
   * Returns the current live transport, or lazily creates one if none exists or the previous one
   * closed. The fast path (live client available) is a volatile read with no synchronization. The
   * slow path (connection creation) enters {@link #getOrCreateConnection()} which synchronizes to
   * prevent duplicate connections.
   */
  @Override
  public Mono<RpcClient> acquire() {
    if (isDisposed()) {
      return closedMono();
    }

    RpcClient current = rpcClient;
    if (current != null && !current.isDisposed()) {
      return Mono.just(current);
    }

    return getOrCreateConnection();
  }

  /**
   * Double-checked locking for connection creation. Only one thread creates the connection Mono;
   * concurrent callers share the same cached Mono until it resolves.
   */
  private Mono<RpcClient> getOrCreateConnection() {
    synchronized (lock) {
      if (isDisposed()) {
        return closedMono();
      }

      RpcClient current = rpcClient;
      if (current != null && !current.isDisposed()) {
        return Mono.just(current);
      }

      if (connecting == null) {
        connecting = createConnection();
      }

      return connecting;
    }
  }

  private Mono<RpcClient> createConnection() {
    return transportFactory
        .createRpcClient(socketAddress)
        .flatMap(this::handleConnection)
        .doOnSuccess(__ -> clearConnecting())
        .doOnError(__ -> clearConnecting())
        // Cache after publication/cleanup so concurrent waiters share one side-effect sequence.
        .cache();
  }

  /**
   * Publishes a newly connected transport under the same lock used by {@link #doDispose()}. If the
   * manager was disposed while the connection was being established, the new transport is
   * immediately disposed to avoid leaking it.
   *
   * <p>A concurrent shutdown can still close a just-connected transport that an already in-flight
   * {@code acquire()} eventually observes. This method guarantees that such a transport is not
   * cached or leaked after manager shutdown.
   *
   * <p>Close invalidation is subscribed after publication, so this code relies on {@link
   * RpcClient#onClose()} replaying terminal completion or error to late subscribers. A transport
   * that closes in that window still needs to invalidate the cached reference.
   */
  private Mono<RpcClient> handleConnection(RpcClient client) {
    synchronized (lock) {
      if (!isDisposed()) {
        rpcClient = client;
        client.onClose().doFinally(signal -> clearCurrentClient(client)).subscribe();
        return Mono.just(client);
      }
    }

    if (!client.isDisposed()) {
      client.dispose();
    }
    return closedMono();
  }

  private void clearConnecting() {
    synchronized (lock) {
      connecting = null;
    }
  }

  /**
   * Nulls out the cached transport only if it is the same instance that closed. This callback runs
   * outside the manager lock, so the identity check and clear must happen atomically to avoid a
   * late-firing {@code onClose()} callback clearing a newer replacement connection.
   */
  private void clearCurrentClient(RpcClient client) {
    RPC_CLIENT.compareAndSet(this, client, null);
  }

  @Override
  protected void doDispose() {
    RpcClient current;
    synchronized (lock) {
      connecting = null;
      current = rpcClient;
      rpcClient = null;
    }

    if (current != null && !current.isDisposed()) {
      current.dispose();
    }
  }
}
