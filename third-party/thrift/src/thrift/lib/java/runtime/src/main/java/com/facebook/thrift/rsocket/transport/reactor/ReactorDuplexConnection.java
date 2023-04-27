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

package com.facebook.thrift.rsocket.transport.reactor;

import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.channel.EventLoop;
import io.rsocket.frame.FrameLengthCodec;
import io.rsocket.internal.BaseDuplexConnection;
import java.time.Duration;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.core.scheduler.Scheduler;
import reactor.core.scheduler.Schedulers;
import reactor.netty.Connection;

public class ReactorDuplexConnection extends BaseDuplexConnection {
  private Connection connection;

  private static final int WINDOW_SIZE = Integer.getInteger("thrift.rsocket.sendWindowSize", 256);

  private static final long WINDOW_TIMEOUT =
      Integer.getInteger("thrift.rsocket.sendWindowTimeoutMs", 20);

  private final Scheduler eventLoopScheduler;

  public ReactorDuplexConnection(Connection connection) {
    this.connection = connection;

    EventLoop eventExecutors = connection.channel().eventLoop();

    eventLoopScheduler = Schedulers.fromExecutor(eventExecutors);

    connection
        .channel()
        .closeFuture()
        .addListener(
            future -> {
              if (!isDisposed()) dispose();
            });
  }

  @Override
  public Mono<Void> send(Publisher<ByteBuf> frames) {
    if (connection instanceof Mono) {
      return connection.outbound().sendObject(Mono.from(frames).map(this::encode)).then();
    } else {
      return connection
          .outbound()
          .sendGroups(
              Flux.from(frames)
                  .map(this::encode)
                  .windowTimeout(
                      WINDOW_SIZE, Duration.ofMillis(WINDOW_TIMEOUT), eventLoopScheduler, true))
          .then();
    }
  }

  @Override
  public Flux<ByteBuf> receive() {
    return connection.inbound().receive().map(ReactorDuplexConnection::decode);
  }

  private ByteBuf encode(ByteBuf frame) {
    return FrameLengthCodec.encode(alloc(), frame.readableBytes(), frame);
  }

  private static ByteBuf decode(ByteBuf frame) {
    return FrameLengthCodec.frame(frame).retain();
  }

  @Override
  public ByteBufAllocator alloc() {
    return RpcResources.getByteBufAllocator();
  }

  @Override
  protected void doOnClose() {
    if (!connection.isDisposed()) {
      connection.dispose();
    }
  }
}
