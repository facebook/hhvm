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
import com.facebook.thrift.payload.Writer;
import java.time.Duration;
import java.time.temporal.ChronoUnit;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.StreamPayloadMetadata;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

/** Adds load information to ResponseRpcMetadata */
public final class LoadHeaderRpcServerHandler implements RpcServerHandler {
  private static final long START_IN_MILLIS = System.currentTimeMillis();

  private static final String LOAD_HEADER = "load";

  private static final long MIN_DECAY_SECS = 1;

  private final AtomicLong outstandingRequests;
  private final RpcServerHandler delegate;
  private final long decayInSeconds;

  public LoadHeaderRpcServerHandler(RpcServerHandler delegate) {
    this(delegate, Duration.ofSeconds(240));
  }

  public LoadHeaderRpcServerHandler(RpcServerHandler delegate, Duration decay) {
    this.delegate = delegate;
    this.outstandingRequests = new AtomicLong();
    this.decayInSeconds = calculateDecay(decay);
  }

  private long calculateDecay(Duration decay) {
    return Math.max(MIN_DECAY_SECS, decay.get(ChronoUnit.SECONDS));
  }

  @Override
  public Mono<ServerResponsePayload> singleRequestSingleResponse(ServerRequestPayload payload) {
    long scaledLoad = getScaledLoad();
    return delegate
        .singleRequestSingleResponse(payload)
        .map(
            serverResponsePayload ->
                (ServerResponsePayload)
                    new LoadHeaderServerResponsePayloadWrapper(serverResponsePayload, scaledLoad))
        .doFinally(s -> outstandingRequests.decrementAndGet());
  }

  long getScaledLoad() {
    return Math.round(getScalingFactor() * outstandingRequests.incrementAndGet());
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

  private static long getTimeSinceStartInSeconds() {
    return TimeUnit.MILLISECONDS.toSeconds(System.currentTimeMillis() - START_IN_MILLIS);
  }

  /**
   * Use a decay function y = 100 e^(-0.05x) + 1, where y is the scaling factor and x is time in
   * seconds. The limit is 1, crosses y axis a 101. It has a 6x load factor at 1 minute and 1.25x
   * load factor at 2 minutes and ~1x load factor at decayInSeconds.
   *
   * @return a decaying scaling factor for load. A higher number means we'll receive less traffic.
   *     Inversely the longer the service runs, the more decay and the closer the value will get to
   *     1. When 1 is returned, the server will return a load header matching it's exact load
   */
  double getScalingFactor() {
    long upTimeSec = getTimeSinceStartInSeconds();

    if (upTimeSec >= 0 && upTimeSec < decayInSeconds) {
      return (100 * Math.exp(-0.05 * upTimeSec)) + 1.0;
    } else {
      return 1;
    }
  }

  private class LoadHeaderServerResponsePayloadWrapper implements ServerResponsePayload {
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
