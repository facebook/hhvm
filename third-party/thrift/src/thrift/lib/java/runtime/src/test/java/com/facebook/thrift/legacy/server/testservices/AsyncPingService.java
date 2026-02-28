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
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.atomic.AtomicInteger;

public class AsyncPingService implements PingService.Async {
  private final AtomicInteger counter = new AtomicInteger();

  @Override
  public void close() {}

  @Override
  public ListenableFuture<PingResponse> ping(PingRequest pingRequest) {
    return Futures.immediateFuture(
        new PingResponse.Builder()
            .setResponse(pingRequest.getRequest() + "_pong_" + counter.getAndIncrement())
            .build());
  }

  @Override
  public ListenableFuture<byte[]> pingBinary(PingRequest pingRequest) {
    return Futures.immediateFuture("Hello!".getBytes(StandardCharsets.UTF_8));
  }

  @Override
  public ListenableFuture<PingResponse> pingException(PingRequest pingRequest) {
    if ("npe".equals(pingRequest.getRequest())) {
      return Futures.immediateFailedFuture(new NullPointerException("npe"));
    }

    return Futures.immediateFailedFuture(
        new CustomException.Builder()
            .setMessage("custom exception: " + pingRequest.getRequest())
            .build());
  }

  @Override
  public ListenableFuture<Void> pingVoid(PingRequest pingRequest) {
    System.out.println("got request -> " + pingRequest.toString());
    return Futures.immediateFuture(null);
  }
}
