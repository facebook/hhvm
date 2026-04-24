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

import java.util.Objects;
import reactor.core.publisher.Mono;

/**
 * Legacy typed-client source.
 *
 * <p>This preserves pre-v2 semantics: acquiring delegates directly to the historical {@code
 * Mono<RpcClient>} and {@link #dispose()} is a no-op.
 */
public final class LegacyRpcClientSource implements RpcClientSource {
  private final Mono<RpcClient> rpcClientMono;

  public LegacyRpcClientSource(Mono<? extends RpcClient> rpcClientMono) {
    this.rpcClientMono = Mono.from(Objects.requireNonNull(rpcClientMono));
  }

  @Override
  public Mono<RpcClient> acquire() {
    return rpcClientMono;
  }

  @Override
  public void dispose() {}

  @Override
  public boolean isDisposed() {
    return false;
  }
}
