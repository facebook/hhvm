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

import io.netty.channel.ChannelDuplexHandler;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelPromise;
import java.net.SocketAddress;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.netty.Connection;
import reactor.netty.ConnectionObserver;

/**
 * A Netty ChannelDuplexHandler that logs events related to channel closure to help diagnose why a
 * connection was terminated.
 *
 * <p>It logs:
 *
 * <p>1. When a close operation is actively initiated.
 *
 * <p>2. When an exception occurs in the pipeline.
 *
 * <p>3. When the channel becomes inactive (the actual closure).
 */
public class ConnectionClosureMetricsHandler extends ChannelDuplexHandler
    implements ConnectionObserver {
  private static final Logger log = LoggerFactory.getLogger(ConnectionClosureMetricsHandler.class);
  private volatile boolean closeInitiated = false;
  private volatile State previousState = null;

  /** ChannelDuplexHandler Methods */

  /**
   * Called when the channel is closed and is no longer active. This is the final state of a closed
   * connection.
   *
   * @param ctx The context of the handler.
   * @throws Exception
   */
  @Override
  public void channelInactive(ChannelHandlerContext ctx) throws Exception {
    log.error(
        "CHANNEL INACTIVE: Connection is now closed for [{}]. Close initiated by us: {}",
        ctx.channel().remoteAddress(),
        closeInitiated,
        new Exception("Stack trace for channelInactive"));

    super.channelInactive(ctx);
  }

  /**
   * Called when an exception is caught in the pipeline. This is a primary source for understanding
   * unexpected connection drops (e.g., IOExceptions, SSLHandshakeExceptions).
   *
   * @param ctx The context of the handler.
   * @param cause The exception that was caught.
   * @throws Exception
   */
  @Override
  public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) throws Exception {
    log.error(
        "EXCEPTION CAUGHT: An exception occurred in the pipeline for [{}]. This is a likely cause"
            + " for connection closure.",
        ctx.channel().remoteAddress(),
        cause);

    super.exceptionCaught(ctx, cause);
  }

  /**
   * Called when an outbound 'close' operation is requested. This indicates that our application is
   * initiating the closure.
   *
   * @param ctx The context of the handler.
   * @param promise A promise that will be notified when the close operation is complete.
   * @throws Exception
   */
  @Override
  public void close(ChannelHandlerContext ctx, ChannelPromise promise) throws Exception {
    closeInitiated = true;
    SocketAddress remoteAddress = ctx.channel().remoteAddress();
    log.error(
        "CLOSE REQUESTED: Application is initiating a connection closure to [{}].",
        remoteAddress,
        new Exception("Stack trace for close() request"));

    // Add a listener to the promise to log the result of the close operation.
    promise.addListener(
        future -> {
          if (future.isSuccess()) {
            log.error("CLOSE SUCCEEDED: Successfully closed connection to [{}].", remoteAddress);
          } else {
            log.error(
                "CLOSE FAILED: Failed to close connection to [{}].", remoteAddress, future.cause());
          }
        });

    super.close(ctx, promise);
  }

  /** ConnectionObserver Methods */
  @Override
  public void onUncaughtException(Connection connection, Throwable error) {
    log.error("UNCAUGHT EXCEPTION: Connection [{}]", connection.channel().remoteAddress());

    onStateChange(connection, State.DISCONNECTING);
  }

  @Override
  public void onStateChange(Connection connection, State newState) {
    log.error(
        "STATECHANGE: Connection [{}] state transition [{}] -> [{}].",
        connection.channel().remoteAddress(),
        previousState,
        newState);
    previousState = newState;
  }
}
