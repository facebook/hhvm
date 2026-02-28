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

package com.facebook.thrift.rsocket.client;

import com.facebook.thrift.client.DelegatingRpcClient;
import com.facebook.thrift.client.RpcClient;
import com.facebook.thrift.client.RpcOptions;
import com.facebook.thrift.model.StreamResponse;
import com.facebook.thrift.payload.ClientRequestPayload;
import com.facebook.thrift.payload.ClientResponsePayload;
import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.protocol.TProtocolType;
import io.netty.buffer.Unpooled;
import java.util.function.Function;
import org.apache.thrift.ClientPushMetadata;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

public final class HeaderAwareRSocketRpcClient extends DelegatingRpcClient {

  private static final String HEADER_KEY = "header_response";

  public HeaderAwareRSocketRpcClient(RpcClient delegate) {
    super(delegate);
  }

  @Override
  public <T> Mono<ClientResponsePayload<T>> singleRequestSingleResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    return getDelegate().singleRequestSingleResponse(payload, options);
  }

  @Override
  public <T> Mono<Void> singleRequestNoResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    return getDelegate().singleRequestNoResponse(payload, options);
  }

  @Override
  public Mono<Void> metadataPush(ClientPushMetadata clientMetadata, RpcOptions options) {
    return getDelegate().metadataPush(clientMetadata, options);
  }

  @Override
  public <T, K> Flux<ClientResponsePayload<K>> singleRequestStreamingResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    return getDelegate()
        .<T, K>singleRequestStreamingResponse(payload, options)
        .map(new HeaderResponseHandler<>(payload.getResponseReader()));
  }

  @Override
  public <T, K> Flux<ClientResponsePayload<K>> streamingRequestStreamingResponse(
      Publisher<ClientRequestPayload<T>> payloads, RpcOptions options) {
    return getDelegate().streamingRequestStreamingResponse(payloads, options);
  }

  /**
   * Converts header ClientResponsePayload<StreamResponse<Object>> to
   * ClientResponsePayload<StreamResponse<k>>
   *
   * @param <T> Generic type of request object
   * @param <K> Generic type of response object
   */
  private static class HeaderResponseHandler<T, K>
      implements Function<ClientResponsePayload<K>, ClientResponsePayload<K>> {

    private final Reader<T> responseReader;

    private HeaderResponseHandler(Reader<T> responseReader) {
      this.responseReader = responseReader;
    }

    @Override
    @SuppressWarnings({"rawtypes", "unchecked"})
    public ClientResponsePayload<K> apply(ClientResponsePayload<K> kClientResponsePayload) {
      StreamResponse response = ((StreamResponse) kClientResponsePayload.getData());
      if (response != null
          && response.isSetData()
          && response.getData() instanceof String
          && response.getData().equals(HEADER_KEY)) {

        K streamResponse =
            (K)
                StreamResponse.fromData(
                    responseReader.read(TProtocolType.TBinary.apply(Unpooled.EMPTY_BUFFER)));

        return ClientResponsePayload.createStreamResult(
            streamResponse,
            kClientResponsePayload.getResponseRpcMetadata(),
            kClientResponsePayload.getStreamPayloadMetadata(),
            kClientResponsePayload.getBinaryHeaders(),
            kClientResponsePayload.getStreamId());
      }
      return kClientResponsePayload;
    }
  }
}
