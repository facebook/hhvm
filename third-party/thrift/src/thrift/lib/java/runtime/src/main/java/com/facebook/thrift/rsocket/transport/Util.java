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

package com.facebook.thrift.rsocket.transport;

import io.netty.channel.ChannelFuture;
import io.netty.util.concurrent.Future;
import io.netty.util.concurrent.GenericFutureListener;
import reactor.core.publisher.Mono;

public final class Util {
  private Util() {}

  public static Mono<Void> toMono(ChannelFuture future) {
    return Mono.create(
        sink -> {
          GenericFutureListener<Future<? super Void>> listener =
              result -> {
                if (result.isDone()) {
                  if (future.isSuccess()) {
                    sink.success();
                  } else {
                    sink.error(result.cause());
                  }
                }
              };

          future.addListener(listener);
          sink.onDispose(
              () -> {
                future.removeListener(listener);
                future.cancel(true);
              });
        });
  }
}
