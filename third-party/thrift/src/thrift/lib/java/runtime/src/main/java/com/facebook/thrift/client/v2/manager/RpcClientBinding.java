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
import java.util.Objects;
import java.util.concurrent.atomic.AtomicBoolean;
import reactor.core.Disposable;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Sinks;

/**
 * Per-typed-client binding between generated code and a manager.
 *
 * <p>This is the ownership boundary. The binding gates {@link #acquire()} after close and decides
 * whether {@link #dispose()} propagates to the underlying manager.
 */
public final class RpcClientBinding implements Disposable {
  private final RpcClientManager manager;
  private final ClientOwnership ownership;
  private final AtomicBoolean closed = new AtomicBoolean(false);
  private final Sinks.Empty<Void> onCloseSink = Sinks.empty();

  public RpcClientBinding(RpcClientManager manager, ClientOwnership ownership) {
    this.manager = Objects.requireNonNull(manager);
    this.ownership = Objects.requireNonNull(ownership);
  }

  /**
   * Returns a live transport from the underlying manager. Fails fast with {@link
   * IllegalStateException} if this binding has been disposed, regardless of whether the manager
   * itself is still alive (BORROWED bindings close the handle without touching the manager).
   */
  public Mono<RpcClient> acquire() {
    return Mono.defer(
        () -> {
          if (closed.get()) {
            return Mono.error(new IllegalStateException("Typed client is already closed"));
          }
          return manager.acquire();
        });
  }

  public Mono<Void> onClose() {
    return onCloseSink.asMono();
  }

  /**
   * Closes this binding. For OWNED bindings, also disposes the underlying manager (shutting down
   * transports). For BORROWED bindings, only marks this handle as closed — the shared manager and
   * its transports remain alive for other borrowers.
   */
  @Override
  public void dispose() {
    if (closed.compareAndSet(false, true)) {
      if (ownership == ClientOwnership.OWNED) {
        manager.dispose();
      }
      onCloseSink.tryEmitEmpty();
    }
  }

  @Override
  public boolean isDisposed() {
    return closed.get();
  }
}
