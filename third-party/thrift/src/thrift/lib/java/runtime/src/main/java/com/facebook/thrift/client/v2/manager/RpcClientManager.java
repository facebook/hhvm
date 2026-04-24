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
import reactor.core.Disposable;
import reactor.core.publisher.Mono;

/**
 * Lifecycle owner for an RPC connection source.
 *
 * <p>A manager sits between generated/typed clients and lower-level transport creation. It answers
 * two questions:
 *
 * <ul>
 *   <li>Which live {@link RpcClient} should I use for this call?
 *   <li>What does it mean to shut this connection source down permanently?
 * </ul>
 *
 * <p>Different implementations represent different acquisition strategies such as a single cached
 * transport, a reconnecting source, a load-balanced set of children, or a tier-level host pool.
 */
public interface RpcClientManager extends Disposable {
  Mono<RpcClient> acquire();

  Mono<Void> onClose();
}
