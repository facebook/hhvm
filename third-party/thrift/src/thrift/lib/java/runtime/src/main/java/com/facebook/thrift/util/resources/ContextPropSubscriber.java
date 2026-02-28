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

/** Subscriber that will forward ContextData for keys provided in ContextPropagationRegistry */
package com.facebook.thrift.util.resources;

import com.facebook.nifty.core.RequestContext;
import com.facebook.nifty.core.RequestContexts;
import java.util.HashMap;
import java.util.Map;
import org.reactivestreams.Subscription;
import reactor.core.CoreSubscriber;

public class ContextPropSubscriber<T> implements CoreSubscriber<T> {
  final CoreSubscriber<T> delegate;
  final Map<String, Object> contextData = new HashMap<>();

  public ContextPropSubscriber(CoreSubscriber<T> delegate) {
    this.delegate = delegate;
  }

  @Override
  public void onSubscribe(Subscription s) {

    delegate.onSubscribe(s);
    // Copy the context data from the current context. onSubscribe will happen on current thread.
    RequestContext context = RequestContexts.getOrCreateCurrentContext();
    for (String key : ContextPropagationRegistry.getContextPropagationKeys()) {
      if (context.getContextData(key) != null) {
        this.contextData.put(key, context.getContextData(key));
      }
    }
  }

  // Set context data for the captured contextData keys before doing onNext, onError, onComplete
  @Override
  public void onNext(T t) {
    setContextData();
    delegate.onNext(t);
  }

  @Override
  public void onError(Throwable t) {
    setContextData();
    delegate.onError(t);
  }

  @Override
  public void onComplete() {
    setContextData();
    delegate.onComplete();
  }

  private void setContextData() {
    // Set the new context data for the captured contextData keys
    if (!this.contextData.isEmpty()) {
      RequestContext context = RequestContexts.getOrCreateCurrentContext();
      this.contextData.forEach((key, value) -> context.setContextData(key, value));
    }
  }
}
