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

package com.facebook.thrift.client;

import com.facebook.thrift.payload.ClientRequestPayload;
import com.facebook.thrift.payload.ClientResponsePayload;
import java.util.Map;
import org.apache.thrift.ClientPushMetadata;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

public class TokenPassingRpcClient extends DelegatingRpcClient {
  private final Map<String, String> headerTokens;

  public TokenPassingRpcClient(RpcClient delegate, Map<String, String> headerTokens) {
    super(delegate);
    this.headerTokens = headerTokens;
  }

  @Override
  public <T> Mono<ClientResponsePayload<T>> singleRequestSingleResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    return getDelegate()
        .singleRequestSingleResponse(payload.withAdditionalHeader(headerTokens), options);
  }

  @Override
  public <T> Mono<Void> singleRequestNoResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    return getDelegate()
        .singleRequestNoResponse(payload.withAdditionalHeader(headerTokens), options);
  }

  @Override
  public <T, K> Flux<ClientResponsePayload<K>> singleRequestStreamingResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    return getDelegate()
        .singleRequestStreamingResponse(payload.withAdditionalHeader(headerTokens), options);
  }

  @Override
  public Mono<Void> metadataPush(ClientPushMetadata clientMetadata, RpcOptions options) {
    return getDelegate().metadataPush(clientMetadata, options);
  }

  @Override
  public <T, K> Flux<ClientResponsePayload<K>> streamingRequestStreamingResponse(
      Publisher<ClientRequestPayload<T>> payloads, RpcOptions options) {
    return getDelegate()
        .streamingRequestStreamingResponse(
            Flux.from(payloads).map(payload -> payload.withAdditionalHeader(headerTokens)),
            options);
  }
}
