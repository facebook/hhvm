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
import java.util.Collections;
import java.util.Map;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

public interface RpcServerHandler {
  default Mono<ServerResponsePayload> singleRequestSingleResponse(ServerRequestPayload payload) {
    return Mono.error(new UnsupportedOperationException());
  }

  default Mono<Void> singleRequestNoResponse(ServerRequestPayload payload) {
    return Mono.error(new UnsupportedOperationException());
  }

  default Flux<ServerResponsePayload> singleRequestStreamingResponse(ServerRequestPayload payload) {
    return Flux.error(new UnsupportedOperationException());
  }

  // TODO: since D29640861, the corresponding RpcKind is Sink
  default Flux<ServerResponsePayload> streamingRequestStreamingResponse(
      ServerRequestPayload initial, Publisher<ServerRequestPayload> payloads) {
    return Flux.error(new UnsupportedOperationException());
  }

  /**
   * Returns a map that contains string to the underlying RpcServerHandler serves the request.
   *
   * @return A map of service name to RpcServerHandler
   */
  default Map<String, RpcServerHandler> getMethodMap() {
    return Collections.emptyMap();
  }
}
