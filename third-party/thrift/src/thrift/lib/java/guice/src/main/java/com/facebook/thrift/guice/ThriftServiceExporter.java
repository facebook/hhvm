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

package com.facebook.thrift.guice;

import static com.google.inject.multibindings.Multibinder.newSetBinder;

import com.facebook.swift.service.ThriftEventHandler;
import com.facebook.thrift.server.CompositeRpcServerHandler;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.server.RpcServerHandlerBuilder;
import com.google.inject.Binder;
import com.google.inject.Inject;
import com.google.inject.Provider;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.stream.Collectors;

public class ThriftServiceExporter {
  private final Binder binder;

  public ThriftServiceExporter(Binder binder) {
    this.binder = binder;
  }

  public static ThriftServiceExporter binder(Binder binder) {
    return new ThriftServiceExporter(binder);
  }

  public ThriftServiceExporter addEventHandler(ThriftEventHandler eventHandler) {
    Objects.requireNonNull(eventHandler);
    newSetBinder(binder, ThriftEventHandler.class).addBinding().toInstance(eventHandler);
    return this;
  }

  public ThriftServiceExporter addEventHandler(Class<? extends ThriftEventHandler> clazz) {
    newSetBinder(binder, ThriftEventHandler.class).addBinding().to(clazz);
    return this;
  }

  @SuppressWarnings("rawtypes")
  public ThriftServiceExporter addServerHandler(RpcServerHandlerBuilder... serverHandlers) {
    Objects.requireNonNull(serverHandlers);
    for (RpcServerHandlerBuilder sh : serverHandlers) {
      newSetBinder(binder, RpcServerHandlerBuilder.class).addBinding().toInstance(sh);
    }
    return this;
  }

  public static class RpcServerHandlerProvider implements Provider<RpcServerHandler> {
    @SuppressWarnings("rawtypes")
    private final Set<RpcServerHandlerBuilder> rpcServerHandlerBuilders;

    private final Set<ThriftEventHandler> eventHandlers;

    @Inject
    public RpcServerHandlerProvider(
        Set<RpcServerHandlerBuilder> rpcServerHandlerBuilders,
        Set<ThriftEventHandler> eventHandlers) {
      this.rpcServerHandlerBuilders = rpcServerHandlerBuilders;
      this.eventHandlers = eventHandlers;
    }

    @Override
    public RpcServerHandler get() {
      List<ThriftEventHandler> eventHandlers = new ArrayList<>(this.eventHandlers);
      List<RpcServerHandler> rpcServerHandlers =
          rpcServerHandlerBuilders.stream()
              .map(
                  rpcServerHandlerBuilder ->
                      rpcServerHandlerBuilder.setEventHandlers(eventHandlers).build())
              .collect(Collectors.toList());

      if (rpcServerHandlers.size() == 1) {
        return rpcServerHandlers.get(0);
      } else {
        return new CompositeRpcServerHandler(rpcServerHandlers);
      }
    }
  }
}
