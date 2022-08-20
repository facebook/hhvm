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

package com.facebook.thrift.util;

import com.facebook.thrift.rsocket.transport.reactor.server.RSocketProtocolDetector;
import com.google.common.util.concurrent.ThreadFactoryBuilder;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelHandler;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.epoll.Epoll;
import io.netty.channel.epoll.EpollEventLoopGroup;
import io.netty.channel.kqueue.KQueue;
import io.netty.channel.kqueue.KQueueEventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.handler.flush.FlushConsolidationHandler;
import io.netty.util.concurrent.FastThreadLocalThread;
import io.netty.util.concurrent.Future;
import io.netty.util.concurrent.GenericFutureListener;
import java.util.Objects;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ThreadFactory;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.publisher.Mono;
import reactor.core.publisher.MonoSink;
import reactor.core.scheduler.NonBlocking;
import reactor.netty.Connection;

public final class NettyUtil {
  private static final Logger LOGGER = LoggerFactory.getLogger(NettyUtil.class);

  public static RSocketProtocolDetector getRSocketProtocolDetector(Connection connection) {
    return new RSocketProtocolDetector(connection);
  }

  public static FlushConsolidationHandler getDefaultThriftFlushConsolidationHandler() {
    return new FlushConsolidationHandler(256, true);
  }

  public static ChannelHandler getRSocketLengthFieldBasedFrameDecoder() {
    return new RSocketLengthCodec();
  }

  /**
   * Converts a Netty {@link ChannelFuture} to a reactor-core Mono&lt;Void&gt;
   *
   * @param future the future you want to convert
   * @return mono representing the ChannelFuture.
   */
  public static Mono<Void> toMono(ChannelFuture future) {
    try {
      Objects.requireNonNull(future, "future");
      if (future.isDone()) {
        if (future.isSuccess()) {
          return Mono.empty();
        } else if (future.isCancelled()) {
          return Mono.error(new CancellationException());
        } else {
          return Mono.error(future.cause());
        }
      }

      return Mono.create(
          sink -> {
            if (future.isDone()) {
              handleFutureForSink(sink, future);
            } else {
              GenericFutureListener<Future<? super Void>> listener =
                  result -> handleFutureForSink(sink, result);

              future.addListener(listener);
              sink.onDispose(
                  () -> {
                    future.removeListener(listener);
                    future.cancel(true);
                  });
            }
          });
    } catch (Throwable t) {
      return Mono.error(t);
    }
  }

  /**
   * This methods maps the events from a Netty {@link Future} and maps them to a reactor-core {@link
   * MonoSink}.
   *
   * @param sink The sink to receive events
   * @param future The future that produced the events
   */
  private static void handleFutureForSink(MonoSink<Void> sink, Future<? super Void> future) {
    if (future.isSuccess()) {
      sink.success();
    } else if (future.isCancelled()) {
      sink.error(new CancellationException());
    } else {
      sink.error(future.cause());
    }
  }

  public static EventLoopGroup createEventLoopGroup(
      int numThreadsForEventLoop, String threadPrefix) {
    EventLoopGroup eventLoop;
    if (Epoll.isAvailable()) {
      eventLoop =
          new EpollEventLoopGroup(numThreadsForEventLoop, daemonThreadFactory(threadPrefix));
    } else if (KQueue.isAvailable()) {
      eventLoop =
          new KQueueEventLoopGroup(numThreadsForEventLoop, daemonThreadFactory(threadPrefix));
    } else {
      eventLoop = new NioEventLoopGroup(numThreadsForEventLoop, daemonThreadFactory(threadPrefix));
    }
    LOGGER.info("Using '{}' with '{}' threads.", eventLoop.getClass(), numThreadsForEventLoop);
    return eventLoop;
  }

  private static ThreadFactory daemonThreadFactory(String threadPrefix) {
    return new ThreadFactoryBuilder()
        .setNameFormat(threadPrefix + "-%d")
        .setDaemon(true)
        .setUncaughtExceptionHandler(
            (t, e) -> LOGGER.error("uncaught exception on thread {}", t.getName(), e))
        .setThreadFactory(ThriftEventLoopThread::new)
        .build();
  }

  /**
   * Thread used by Thrift for a Netty Eventloop. It immplements {@link FastThreadLocalThread} which
   * is special thread local used by Netty 4 esp for ByteBufs. It also is marked with {@
   * NonBlocking} interface from reactor-core. This means that if someone tries to call
   * block()/blockFirst()/blockLast() methods, which could block the eventloop, it cause an
   * exception to thrown preventing this.
   */
  private static class ThriftEventLoopThread extends FastThreadLocalThread implements NonBlocking {
    ThriftEventLoopThread(Runnable r) {
      super(r);
    }
  }
}
