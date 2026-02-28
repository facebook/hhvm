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

import com.facebook.thrift.example.blocking.BlockingTestService;
import com.facebook.thrift.example.blocking.PingRequest;
import com.facebook.thrift.example.blocking.PingResponse;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;
import reactor.core.publisher.Flux;

public class AsyncBlockingTestService implements BlockingTestService.Async {
  @Override
  public void close() {}

  @Override
  public ListenableFuture<PingResponse> generatePingWithBlockingCode(PingRequest pingRequest) {
    Integer integer = Flux.range(1, 10).blockLast();
    return Futures.immediateFuture(
        new PingResponse.Builder().setResponse(integer.toString()).build());
  }
}
