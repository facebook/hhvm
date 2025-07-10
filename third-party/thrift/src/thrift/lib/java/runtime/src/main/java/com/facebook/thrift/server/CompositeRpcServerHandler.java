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

import static org.apache.thrift.TApplicationException.UNKNOWN_METHOD;

import com.facebook.thrift.payload.ServerRequestPayload;
import com.facebook.thrift.payload.ServerResponsePayload;
import com.facebook.thrift.util.RpcPayloadUtil;
import com.google.common.annotations.VisibleForTesting;
import com.google.common.base.Preconditions;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import org.apache.thrift.TApplicationException;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

/**
 * CompositeRpcServerHandler takes a one or more {@link RpcServerHandler} instances delegates
 * service calls. This lets you mix together different service implementations without having them
 * all implemented in a single class.
 */
public class CompositeRpcServerHandler implements RpcServerHandler {
  @VisibleForTesting final Map<String, RpcServerHandler> methodMap;

  public CompositeRpcServerHandler(List<RpcServerHandler> handlers) {
    this.methodMap = new HashMap<>();
    Preconditions.checkArgument(
        handlers != null && handlers.size() > 0, "handlers should not be null or empty.");
    for (RpcServerHandler h : handlers) {
      addServiceNameToMethodMapping(h);
    }
  }

  private void addServiceNameToMethodMapping(RpcServerHandler handler) {
    for (Map.Entry<String, RpcServerHandler> entry : handler.getMethodMap().entrySet()) {
      if (methodMap.containsKey(entry.getKey())) {
        throw new IllegalArgumentException(
            "Error when attempting to add "
                + entry.getKey()
                + " to CompositeRpcServerHandler from "
                + handler.getClass().getName()
                + " due to a duplicated entry from "
                + methodMap.get(entry.getKey()).getClass().getName());
      } else {
        methodMap.put(entry.getKey(), entry.getValue());
      }
    }
  }

  @Override
  public Mono<ServerResponsePayload> singleRequestSingleResponse(ServerRequestPayload payload) {
    final String name = payload.getRequestRpcMetadata().getName();
    final RpcServerHandler delegate = methodMap.get(name);

    if (delegate == null) {
      return Mono.just(
          RpcPayloadUtil.fromTApplicationException(
              new TApplicationException(UNKNOWN_METHOD, "Unknown method : '" + name + "'"),
              payload.getRequestRpcMetadata(),
              null));
    }

    return delegate.singleRequestSingleResponse(payload);
  }

  @Override
  public Mono<Void> singleRequestNoResponse(ServerRequestPayload payload) {
    final String name = payload.getRequestRpcMetadata().getName();
    final RpcServerHandler delegate = methodMap.get(name);

    if (delegate == null) {
      return Mono.error(new IllegalStateException("no RpcServerHandler found for " + name));
    }

    return delegate.singleRequestNoResponse(payload);
  }

  @Override
  public Flux<ServerResponsePayload> singleRequestStreamingResponse(ServerRequestPayload payload) {
    final String name = payload.getRequestRpcMetadata().getName();
    final RpcServerHandler delegate = methodMap.get(name);

    if (delegate == null) {
      return Flux.error(new IllegalStateException("no RpcServerHandler found for " + name));
    }

    return delegate.singleRequestStreamingResponse(payload);
  }

  @Override
  public Flux<ServerResponsePayload> streamingRequestStreamingResponse(
      ServerRequestPayload initial, Publisher<ServerRequestPayload> payloads) {
    final String name = initial.getRequestRpcMetadata().getName();
    final RpcServerHandler delegate = methodMap.get(name);

    if (delegate == null) {
      return Flux.error(new IllegalStateException("no RpcServerHandler found for " + name));
    }

    return delegate.streamingRequestStreamingResponse(initial, payloads);
  }

  @Override
  public Map<String, RpcServerHandler> getMethodMap() {
    return methodMap;
  }
}
