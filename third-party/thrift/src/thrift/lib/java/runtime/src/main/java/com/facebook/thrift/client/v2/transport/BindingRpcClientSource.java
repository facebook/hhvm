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

package com.facebook.thrift.client.v2.transport;

import com.facebook.thrift.client.RpcClient;
import com.facebook.thrift.client.RpcClientSource;
import com.facebook.thrift.client.v2.manager.RpcClientBinding;
import java.util.Objects;
import reactor.core.publisher.Mono;

/** Manager-backed {@link RpcClientSource} implementation used by the v2 runtime. */
public final class BindingRpcClientSource implements RpcClientSource {
  private final RpcClientBinding binding;

  public BindingRpcClientSource(RpcClientBinding binding) {
    this.binding = Objects.requireNonNull(binding);
  }

  @Override
  public Mono<RpcClient> acquire() {
    return binding.acquire();
  }

  @Override
  public void dispose() {
    binding.dispose();
  }

  @Override
  public boolean isDisposed() {
    return binding.isDisposed();
  }
}
