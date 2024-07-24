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

package com.facebook.nifty.core;

public class RequestContexts {
  private static ThreadLocal<RequestContext> threadLocalContext = new ThreadLocal<>();

  private RequestContexts() {}

  /**
   * Gets the thread-local {@link NiftyRequestContext} for the Thrift request that is being
   * processed on the current thread.
   *
   * @return The {@link NiftyRequestContext} of the current request
   */
  public static RequestContext getCurrentContext() {
    RequestContext currentContext = threadLocalContext.get();
    return currentContext;
  }

  /**
   * Sets the thread-local context for the currently running request.
   *
   * <p>This is normally called only by the server, but it can also be useful to call when
   * dispatching to another thread (e.g. a thread in an ExecutorService) if the code that will run
   * on that thread might also be interested in the {@link RequestContext}
   */
  public static void setCurrentContext(RequestContext requestContext) {
    threadLocalContext.set(requestContext);
  }

  /**
   * Gets the thread-local context for the currently running request
   *
   * <p>This is normally called only by the server, but it can also be useful to call when cleaning
   * up a context
   */
  public static void clearCurrentContext() {
    threadLocalContext.remove();
  }
}
