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

import com.facebook.nifty.core.RequestContext;
import com.facebook.nifty.core.RequestContexts;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/** Runnable that will forward ContextData for keys provided in ContextPropagationRegistry */
public final class ContextPropRunnable implements Runnable {
  private final Runnable runnable;
  private final Map<String, Object> contextData = new HashMap<>();
  private Set<String> contextDataKeys = ContextPropagationRegistry.getContextPropagationKeys();

  public ContextPropRunnable(Runnable runnable) {
    this.runnable = runnable;

    // If there are no context data keys to propagate, we can just run the runnable directly
    if (!contextDataKeys.isEmpty()) {
      RequestContext requestContext = RequestContexts.getOrCreateCurrentContext();

      // Copy the context data for the keys we care about
      for (String key : contextDataKeys) {
        if (requestContext.getContextData(key) != null) {
          this.contextData.put(key, requestContext.getContextData(key));
        }
      }
    }
  }

  @Override
  public void run() {
    Map<String, Object> oldContextData = new HashMap<>();
    try {
      // If there are no context data keys to propagate, we can just run the runnable directly
      if (!contextDataKeys.isEmpty()) {
        RequestContext requestContext = RequestContexts.getOrCreateCurrentContext();

        // Copy the "old" context data for the keys we care about
        for (String key : this.contextDataKeys) {
          if (requestContext.getContextData(key) != null) {
            oldContextData.put(key, requestContext.getContextData(key));
          }
        }

        // Set the new context data for the captured contextData keys
        this.contextData.forEach((key, value) -> requestContext.setContextData(key, value));
      }
      runnable.run();
    } finally {
      if (!contextDataKeys.isEmpty()) {
        // Restore the old context data for the keys
        final RequestContext requestContext = RequestContexts.getCurrentContext();
        oldContextData.forEach((key, value) -> requestContext.setContextData(key, value));
      }
    }
  }
}
