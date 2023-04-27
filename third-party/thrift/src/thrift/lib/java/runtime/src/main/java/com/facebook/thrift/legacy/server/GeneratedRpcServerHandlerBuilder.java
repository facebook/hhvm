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

package com.facebook.thrift.legacy.server;

import com.facebook.swift.service.ThriftEventHandler;
import com.facebook.thrift.server.CompositeRpcServerHandler;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.server.RpcServerHandlerBuilder;
import com.google.common.annotations.VisibleForTesting;
import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import reactor.core.Exceptions;

public class GeneratedRpcServerHandlerBuilder {
  public static RpcServerHandler generatedRpcServerHandler(
      List<Object> sources, List<ThriftEventHandler> eventHandlers) {
    List<RpcServerHandler> serverHandlers = new ArrayList<>();
    for (Object serviceImpl : sources) {
      Class<?> serviceImplClass = serviceImpl.getClass();
      Set<Class<?>> thriftInterfaces = getAllThriftInterfaces(serviceImplClass);

      for (Class<?> thriftInterface : thriftInterfaces) {
        RpcServerHandlerBuilder<?> serverHandlerBuilder =
            getServerHandlerBuilder(serviceImpl, thriftInterface);
        RpcServerHandler handler = serverHandlerBuilder.setEventHandlers(eventHandlers).build();
        serverHandlers.add(handler);
      }
    }

    return new CompositeRpcServerHandler(serverHandlers);
  }

  @SuppressWarnings("unchecked")
  @VisibleForTesting
  static <T> RpcServerHandlerBuilder<T> getServerHandlerBuilder(
      Object impl, Class<T> serverInterface) {
    MethodHandle serverHandlerBuilderMethodHandle =
        serverHandlerBuilderMethodHandle(serverInterface);
    try {
      return (RpcServerHandlerBuilder<T>) serverHandlerBuilderMethodHandle.invoke(impl);
    } catch (Throwable t) {
      throw Exceptions.propagate(t);
    }
  }

  private static MethodHandle serverHandlerBuilderMethodHandle(Class<?> serverInterface) {
    MethodHandles.Lookup lookup = MethodHandles.lookup();
    try {
      return lookup.findStatic(
          serverInterface,
          "serverHandlerBuilder",
          MethodType.methodType(RpcServerHandlerBuilder.class, serverInterface));
    } catch (Throwable t) {
      throw Exceptions.propagate(t);
    }
  }

  @VisibleForTesting
  static Set<Class<?>> getAllThriftInterfaces(Class<?> clz) {
    Set<Class<?>> set = new HashSet<>();
    getAllThriftInterfaces(clz, set);
    return set;
  }

  private static void getAllThriftInterfaces(Class<?> clz, Set<Class<?>> visited) {
    if (clz.getSuperclass() != null) {
      getAllThriftInterfaces(clz.getSuperclass(), visited);
    }
    for (Class<?> c : clz.getInterfaces()) {
      if (com.facebook.thrift.util.ThriftService.class.isAssignableFrom(c)) {
        visited.add(c);
      }
    }
  }
}
