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
import java.util.Objects;
import org.apache.thrift.ClientPushMetadata;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

/**
 * An abstract {@link RpcClient} that delegates calls to another RpcClient. Subclasses should
 * override methods where they want to change the behavior before calling the delegate. If methods
 * aren't override the method call will go to the delegate.
 */
public abstract class DelegatingRpcClient implements RpcClient {
  private final RpcClient delegate;

  public DelegatingRpcClient(RpcClient delegate) {
    this.delegate = Objects.requireNonNull(delegate);
  }

  @Override
  public Mono<Void> onClose() {
    return delegate.onClose();
  }

  @Override
  public void dispose() {
    delegate.dispose();
  }

  @Override
  public boolean isDisposed() {
    return delegate.isDisposed();
  }

  @Override
  public <T> Mono<ClientResponsePayload<T>> singleRequestSingleResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    return delegate.singleRequestSingleResponse(payload, options);
  }

  @Override
  public <T> Mono<Void> singleRequestNoResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    return delegate.singleRequestNoResponse(payload, options);
  }

  @Override
  public <T, K> Flux<ClientResponsePayload<K>> singleRequestStreamingResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    return delegate.singleRequestStreamingResponse(payload, options);
  }

  @Override
  public <T, K> Flux<ClientResponsePayload<K>> streamingRequestStreamingResponse(
      Publisher<ClientRequestPayload<T>> payloads, RpcOptions options) {
    return delegate.streamingRequestStreamingResponse(payloads, options);
  }

  @Override
  public Mono<Void> metadataPush(ClientPushMetadata clientMetadata, RpcOptions options) {
    return delegate.metadataPush(clientMetadata, options);
  }

  protected RpcClient getDelegate() {
    return delegate;
  }
}
