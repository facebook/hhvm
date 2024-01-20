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

package com.facebook.thrift.util;

import com.facebook.thrift.util.resources.RpcResources;
import java.util.function.Function;
import reactor.core.publisher.Mono;

/**
 * If processing is forced off the event loop this publish the events from a mon on a different
 * thread pool
 *
 * @param <T>
 */
public class MonoPublishingTransformer<T> implements Function<Mono<T>, Mono<T>> {
  private static final MonoPublishingTransformer INSTANCE = new MonoPublishingTransformer<>();

  private MonoPublishingTransformer() {}

  @Override
  public Mono<T> apply(Mono<T> mono) {
    if (RpcResources.isForceExecutionOffEventLoop()) {
      return mono.publishOn(RpcResources.getClientOffLoopScheduler());
    } else {
      return mono;
    }
  }

  @SuppressWarnings("unchecked")
  public static <T> MonoPublishingTransformer<T> getInstance() {
    return INSTANCE;
  }
}
