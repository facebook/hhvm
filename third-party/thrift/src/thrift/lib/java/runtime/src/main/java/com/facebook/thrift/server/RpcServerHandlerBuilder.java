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

package com.facebook.thrift.server;

import com.facebook.swift.service.ThriftEventHandler;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

public abstract class RpcServerHandlerBuilder<T> {
  protected final T impl;
  protected List<ThriftEventHandler> eventHandlers = Collections.emptyList();

  public RpcServerHandlerBuilder(T impl) {
    this.impl = impl;
  }

  public RpcServerHandlerBuilder<T> setEventHandlers(List<ThriftEventHandler> eventHandlers) {
    this.eventHandlers = Objects.requireNonNull(eventHandlers);
    return this;
  }

  public abstract RpcServerHandler build();
}
