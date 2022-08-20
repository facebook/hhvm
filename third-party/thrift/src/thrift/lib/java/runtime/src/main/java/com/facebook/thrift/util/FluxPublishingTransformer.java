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
import reactor.core.publisher.Flux;

/**
 * If processing is forced off the event loop this publish the events from a flux on a different
 * thread pool
 *
 * @param <T>
 */
public class FluxPublishingTransformer<T> implements Function<Flux<T>, Flux<T>> {
  private static final FluxPublishingTransformer INSTANCE = new FluxPublishingTransformer<>();

  private FluxPublishingTransformer() {}

  @Override
  public Flux<T> apply(Flux<T> flux) {
    if (RpcResources.isForceExecutionOffEventLoop()) {
      return flux.publishOn(RpcResources.getOffLoopScheduler());
    } else {
      return flux;
    }
  }

  @SuppressWarnings("unchecked")
  public static <T> FluxPublishingTransformer<T> getInstance() {
    return INSTANCE;
  }
}
