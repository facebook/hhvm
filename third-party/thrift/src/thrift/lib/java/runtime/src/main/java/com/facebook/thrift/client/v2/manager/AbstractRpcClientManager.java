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
import java.util.concurrent.atomic.AtomicBoolean;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Sinks;

/**
 * Shared close-state and {@code onClose()} plumbing for manager implementations.
 *
 * <p>Subclasses provide only their resource-specific disposal logic in {@link #doDispose()}.
 */
abstract class AbstractRpcClientManager implements RpcClientManager {
  private final AtomicBoolean closed = new AtomicBoolean(false);
  private final Sinks.Empty<Void> onCloseSink = Sinks.empty();

  /**
   * Atomically transitions to closed, invokes subclass cleanup, and completes the onClose signal.
   * The CAS ensures dispose is idempotent. The finally block guarantees onClose completes even if
   * doDispose throws.
   */
  @Override
  public final void dispose() {
    if (closed.compareAndSet(false, true)) {
      try {
        doDispose();
      } finally {
        onCloseSink.tryEmitEmpty();
      }
    }
  }

  @Override
  public final boolean isDisposed() {
    return closed.get();
  }

  @Override
  public final Mono<Void> onClose() {
    return onCloseSink.asMono();
  }

  protected final Mono<RpcClient> closedMono() {
    return Mono.error(new IllegalStateException("Rpc client manager is already closed"));
  }

  /** Subclass hook for resource-specific cleanup. Called exactly once under the closed CAS. */
  protected abstract void doDispose();
}
