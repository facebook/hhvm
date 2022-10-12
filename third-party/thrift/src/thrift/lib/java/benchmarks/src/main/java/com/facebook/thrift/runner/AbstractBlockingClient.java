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

package com.facebook.thrift.runner;

import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import io.netty.util.concurrent.FastThreadLocalThread;
import reactor.core.publisher.Mono;
import reactor.core.scheduler.Scheduler;
import reactor.core.scheduler.Schedulers;

public abstract class AbstractBlockingClient extends AbstractClient<PingService> {
  private static final Scheduler SCHEDULER =
      Schedulers.newBoundedElastic(
          CONCURRENCY * 10,
          1024,
          (Runnable r) -> {
            FastThreadLocalThread thread = new FastThreadLocalThread(r);
            thread.setName("netty4-rpc-worker-%d");
            thread.setDaemon(true);
            return thread;
          },
          60);

  private final boolean requireSync;

  AbstractBlockingClient(boolean requireSync) {
    this.requireSync = requireSync;
  }

  @Override
  protected final Mono<PingResponse> getResponse(PingService service, String message) {
    return Mono.fromSupplier(
            () -> {
              if (requireSync) {
                synchronized (service) {
                  return service.ping(new PingRequest(message));
                }
              } else {
                return service.ping(new PingRequest(message));
              }
            })
        .subscribeOn(SCHEDULER);
  }
}
