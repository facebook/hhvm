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

package com.facebook.thrift.exceptions;

import java.util.concurrent.TimeoutException;

/**
 * A stackless TimeoutException for RPC timeouts.
 *
 * <p>Overrides fillInStackTrace() to avoid expensive stack trace capture. Stack traces from
 * timer/scheduler threads provide no useful debugging information, and capturing them during
 * timeout storms can worsen cascading failures.
 *
 * <p>Allocate a new instance per timeout; never cache one in a static field. Reactor's operator
 * debug mode attaches an OnAssemblyException to a throwable and reuses it, appending one assembly
 * trace for every operator chain the throwable travels through. A shared instance is never
 * collected, so those traces accumulate for the life of the process and are printed in full by
 * every logger that is handed the throwable.
 */
public class RpcTimeoutException extends TimeoutException {

  public RpcTimeoutException(String message) {
    super(message);
  }

  @Override
  public synchronized Throwable fillInStackTrace() {
    return this;
  }
}
