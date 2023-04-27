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

package com.facebook.thrift.legacy.server.testservices;

import com.facebook.thrift.example.ping.CustomException;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.atomic.AtomicInteger;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

public class ReactivePingService implements PingService.Reactive {
  private final AtomicInteger counter = new AtomicInteger();

  @Override
  public void dispose() {}

  @Override
  public Mono<PingResponse> ping(PingRequest pingRequest) {
    return Mono.just(
        new PingResponse.Builder()
            .setResponse(pingRequest.getRequest() + "_pong_" + counter.getAndIncrement())
            .build());
  }

  @Override
  public Mono<byte[]> pingBinary(PingRequest pingRequest) {
    return Mono.just("hello!".getBytes(StandardCharsets.UTF_8));
  }

  @Override
  public Mono<PingResponse> pingException(PingRequest pingRequest) {
    if ("npe".equals(pingRequest.getRequest())) {
      return Mono.error(new NullPointerException("npe"));
    }
    return Mono.error(
        new CustomException.Builder()
            .setMessage("custom exception: " + pingRequest.getRequest())
            .build());
  }

  @Override
  public Mono<Void> pingVoid(PingRequest pingRequest) {
    return Mono.empty();
  }

  @Override
  public Flux<PingResponse> streamOfPings(PingRequest request, int numberOfPings) {
    return Flux.range(1, numberOfPings)
        .map(i -> new PingResponse.Builder().setResponse("pong " + i).build());
  }
}
