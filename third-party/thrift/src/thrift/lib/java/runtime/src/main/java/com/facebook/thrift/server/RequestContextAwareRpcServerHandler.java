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

import com.facebook.thrift.payload.ServerRequestPayload;
import com.facebook.thrift.payload.ServerResponsePayload;
import com.facebook.thrift.util.RpcServerUtils;
import java.util.Map;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

/**
 * Wraps an RpcServerHandler and makes the {@link com.facebook.nifty.core.RequestContext} available
 * to the delegated RpcServerHandler
 */
public class RequestContextAwareRpcServerHandler implements RpcServerHandler {

  private final RpcServerHandler delegate;

  public RequestContextAwareRpcServerHandler(RpcServerHandler delegate) {
    this.delegate = delegate;
  }

  @Override
  public Mono<ServerResponsePayload> singleRequestSingleResponse(ServerRequestPayload payload) {
    return RpcServerUtils.decorateWithRequestContext(
        payload.getRequestContext(), delegate.singleRequestSingleResponse(payload));
  }

  @Override
  public Mono<Void> singleRequestNoResponse(ServerRequestPayload payload) {
    return RpcServerUtils.decorateWithRequestContext(
        payload.getRequestContext(), delegate.singleRequestNoResponse(payload));
  }

  @Override
  public Flux<ServerResponsePayload> singleRequestStreamingResponse(ServerRequestPayload payload) {

    return RpcServerUtils.decorateWithRequestContext(
        payload.getRequestContext(), delegate.singleRequestStreamingResponse(payload));
  }

  @Override
  public Flux<ServerResponsePayload> streamingRequestStreamingResponse(
      ServerRequestPayload initial, Publisher<ServerRequestPayload> payloads) {
    return RpcServerUtils.decorateWithRequestContext(
        initial.getRequestContext(), delegate.streamingRequestStreamingResponse(initial, payloads));
  }

  @Override
  public Map<String, RpcServerHandler> getMethodMap() {
    return delegate.getMethodMap();
  }
}
