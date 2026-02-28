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

package com.facebook.thrift.loadbalancing;

import com.facebook.thrift.payload.ServerRequestPayload;
import com.facebook.thrift.payload.ServerResponsePayload;
import com.facebook.thrift.payload.Writer;
import com.facebook.thrift.server.RpcServerHandler;
import io.netty.util.internal.StringUtil;
import java.util.HashMap;
import java.util.Map;
import java.util.function.Function;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.StreamPayloadMetadata;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

/** Adds load information to ResponseRpcMetadata */
public final class LoadHeaderRpcServerHandler implements RpcServerHandler {
  private static final String LOAD_HEADER = "load";
  private static final String DEFAULT_LOAD = "default";

  private final RpcServerHandler delegate;
  private final Function<String, Long> counterProvider;
  private final LoadHeaderSupplier loadSupplier;

  public LoadHeaderRpcServerHandler(
      RpcServerHandler delegate, Function<String, Long> counterProvider) {
    this(delegate, counterProvider, new OutstandingRequestLoadSupplier());
  }

  public LoadHeaderRpcServerHandler(
      RpcServerHandler delegate,
      Function<String, Long> counterProvider,
      LoadHeaderSupplier loadSupplier) {
    this.delegate = delegate;
    this.counterProvider = counterProvider;
    this.loadSupplier = loadSupplier;
  }

  @Override
  public Mono<ServerResponsePayload> singleRequestSingleResponse(ServerRequestPayload payload) {
    if (payload.getRequestContext().getRequestHeader().containsKey(LOAD_HEADER)) {
      String loadHeader = payload.getRequestContext().getRequestHeader().get(LOAD_HEADER);

      if (DEFAULT_LOAD.equals(loadHeader)) {
        // Default, uses the loadSupplier class which can be default outstanding requests, or
        // customized supplier
        loadSupplier.onRequest();
        return delegate
            .singleRequestSingleResponse(payload)
            .map(applyDefaultLoadHeader())
            .doFinally(s -> loadSupplier.doFinally());
      } else if (!StringUtil.isNullOrEmpty(loadHeader)) {
        // If client passes "load":"foo", then we attempt to load the "foo" counter from the counter
        // provider supplying the client with the arbitrary counter requested
        return delegate.singleRequestSingleResponse(payload).map(applyCustomLoadHeader(loadHeader));
      }
    }

    // Default returns no load header at all
    return delegate.singleRequestSingleResponse(payload);
  }

  /**
   * Applies load header to the server response payload from the load supplier class, this by
   * default is the number of active and queue requests, but can be a custom load supplier that
   * server owner may pass to service framework
   *
   * @return LoadHeaderServerResponsePayloadWrapper with load header applied
   */
  private Function<ServerResponsePayload, ServerResponsePayload> applyDefaultLoadHeader() {
    return (serverResponsePayload) ->
        new LoadHeaderServerResponsePayloadWrapper(serverResponsePayload, loadSupplier.getLoad());
  }

  /**
   * Applies load header to the server response payload via the counter provider. When the client
   * sends a custom value via the header field this value is looked up via the counter provider,
   * which by default is the fb303 service registered with service framework.
   *
   * @param loadHeader String, fb303 key that will be returned as the load value
   * @return LoadHeaderServerResponsePayloadWrapper with load header applied
   */
  private Function<ServerResponsePayload, ServerResponsePayload> applyCustomLoadHeader(
      String loadHeader) {
    return (serverResponsePayload) ->
        new LoadHeaderServerResponsePayloadWrapper(
            serverResponsePayload, counterProvider.apply(loadHeader));
  }

  @Override
  public Mono<Void> singleRequestNoResponse(ServerRequestPayload payload) {
    return delegate.singleRequestNoResponse(payload);
  }

  @Override
  public Flux<ServerResponsePayload> singleRequestStreamingResponse(ServerRequestPayload payload) {
    return delegate.singleRequestStreamingResponse(payload);
  }

  @Override
  public Flux<ServerResponsePayload> streamingRequestStreamingResponse(
      ServerRequestPayload initial, Publisher<ServerRequestPayload> payloads) {
    return delegate.streamingRequestStreamingResponse(initial, payloads);
  }

  @Override
  public Map<String, RpcServerHandler> getMethodMap() {
    return delegate.getMethodMap();
  }

  private static class LoadHeaderServerResponsePayloadWrapper implements ServerResponsePayload {
    private final ServerResponsePayload delegate;
    private final ResponseRpcMetadata responseRpcMetadata;

    public LoadHeaderServerResponsePayloadWrapper(ServerResponsePayload delegate, long load) {
      this.delegate = delegate;
      this.responseRpcMetadata =
          createResponseRpcMetadataWithLoad(delegate.getResponseRpcMetadata(), load);
    }

    private ResponseRpcMetadata createResponseRpcMetadataWithLoad(
        ResponseRpcMetadata src, long load) {
      Map<String, String> otherMetadata = addLoadHeaderToMap(src.getOtherMetadata(), load);
      return new ResponseRpcMetadata.Builder(src)
          .setLoad(load)
          .setOtherMetadata(otherMetadata)
          .build();
    }

    private Map<String, String> addLoadHeaderToMap(Map<String, String> src, long load) {
      Map<String, String> target = new HashMap<>();

      if (src != null && !src.isEmpty()) {
        target.putAll(src);
      }

      target.put(LOAD_HEADER, String.valueOf(load));
      return target;
    }

    @Override
    public Writer getDataWriter() {
      return delegate.getDataWriter();
    }

    @Override
    public boolean isTApplicationException() {
      return delegate.isTApplicationException();
    }

    @Override
    public ResponseRpcMetadata getResponseRpcMetadata() {
      return responseRpcMetadata;
    }

    @Override
    public StreamPayloadMetadata getStreamPayloadMetadata() {
      return delegate.getStreamPayloadMetadata();
    }

    @Override
    public boolean isStreamingResponse() {
      return delegate.isStreamingResponse();
    }
  }
}
