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

package com.facebook.thrift.util.resources;

import static com.google.common.base.Preconditions.checkNotNull;

import java.util.HashSet;
import java.util.Set;
import reactor.core.publisher.Hooks;
import reactor.core.publisher.Operators;
import reactor.core.scheduler.Schedulers;

public class ContextPropagationRegistry {

  private ContextPropagationRegistry() {}

  private static final Set<String> propagationKeys = new HashSet<>();

  private static boolean hooksRegistered = false;

  /**
   * Register a context propagation key. Keys passed here will be propagated in thread hops within
   * thrift. Data corresponding to these keys should be present in `RequestContext`. First
   * invocation to this method also register reactor hooks needed to propagate thread locals across
   * threads.
   *
   * @param key
   */
  public static void registerContextPropagationKey(String key) {
    checkNotNull(key, "Context prop key is null");
    // Register `ContextPropRunnable` and `ContextPropSubscriber` on first invocation.
    if (!hooksRegistered) {
      Schedulers.onScheduleHook("context.propagation", ContextPropRunnable::new);
      Hooks.onLastOperator(
          Operators.lift((scannable, subscriber) -> new ContextPropSubscriber<>(subscriber)));
      hooksRegistered = true;
    }
    propagationKeys.add(key);
  }

  /**
   * Returns ContextProp Keys to be used for copying context across threads.
   *
   * @return ContextProp Keys
   */
  public static Set<String> getContextPropagationKeys() {
    return propagationKeys;
  }

  /**
   * Returns if context prop is registered.
   *
   * @return hooksRegistered
   */
  public static boolean isContextPropEnabled() {
    return hooksRegistered;
  }
}
