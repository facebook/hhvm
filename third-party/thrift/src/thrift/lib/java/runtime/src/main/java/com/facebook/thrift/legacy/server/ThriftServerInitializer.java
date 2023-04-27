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

package com.facebook.thrift.legacy.server;

import static com.google.common.base.Preconditions.checkArgument;
import static java.util.Objects.requireNonNull;

import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.util.MetricsChannelDuplexHandler;
import com.facebook.thrift.util.SPINiftyMetrics;
import io.airlift.units.DataSize;
import io.airlift.units.Duration;
import io.netty.channel.Channel;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelPipeline;
import io.netty.handler.flush.FlushConsolidationHandler;
import io.netty.handler.logging.LogLevel;
import io.netty.handler.logging.LoggingHandler;
import io.netty.handler.ssl.OptionalSslHandler;
import io.netty.handler.ssl.SslContext;
import java.util.Optional;
import java.util.function.Supplier;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ThriftServerInitializer extends ChannelInitializer<Channel> {
  private final Logger logger = LoggerFactory.getLogger(ThriftServerInitializer.class);
  private final RpcServerHandler rpcServerHandler;
  private final DataSize maxFrameSize;
  private final Duration requestTimeout;
  private final Optional<SslContext> sslContext;
  private final boolean allowPlainText;
  private final boolean assumeClientsSupportOutOfOrderResponses;
  private final SPINiftyMetrics metrics;
  private final int connectionLimit;

  public ThriftServerInitializer(
      RpcServerHandler rpcServerHandler,
      DataSize maxFrameSize,
      Duration requestTimeout,
      Optional<Supplier<SslContext>> sslContextSupplier,
      boolean allowPlainText,
      boolean assumeClientsSupportOutOfOrderResponses,
      SPINiftyMetrics metrics,
      int connectionLimit) {
    requireNonNull(rpcServerHandler, "rpcServerHandler is null");
    requireNonNull(maxFrameSize, "maxFrameSize is null");
    requireNonNull(requestTimeout, "requestTimeout is null");
    requireNonNull(sslContextSupplier, "sslContextSupplier is null");
    checkArgument(
        allowPlainText || sslContextSupplier.isPresent(),
        "Plain text is not allowed, but SSL is not configured");

    this.rpcServerHandler = rpcServerHandler;
    this.maxFrameSize = maxFrameSize;
    this.requestTimeout = requestTimeout;
    this.sslContext = sslContextSupplier.map(Supplier::get);
    this.allowPlainText = allowPlainText;
    this.assumeClientsSupportOutOfOrderResponses = assumeClientsSupportOutOfOrderResponses;
    this.metrics = metrics;
    this.connectionLimit = connectionLimit;
  }

  @Override
  protected void initChannel(Channel channel) {
    if (connectionLimit > 0) {
      final int channelCount = metrics.getChannelCount();
      if (channelCount >= connectionLimit) {
        metrics.incrementRejectedConnections();
        logger.error("closing connection because limit " + connectionLimit + " has been exceeded");
        channel.close();
        return;
      }
    }

    if (logger.isDebugEnabled()) {
      logger.debug("accepting connection from " + channel.remoteAddress());
    }

    ChannelPipeline pipeline = channel.pipeline();

    pipeline.addLast(
        new LoggingHandler(LogLevel.TRACE),
        new MetricsChannelDuplexHandler(metrics),
        new FlushConsolidationHandler(256, true));

    if (sslContext.isPresent()) {
      if (allowPlainText) {
        pipeline.addLast(new OptionalSslHandler(sslContext.get()));
      } else {
        pipeline.addLast(sslContext.get().newHandler(channel.alloc()));
      }
    }

    pipeline.addLast(
        new ThriftProtocolDetection(
            new ThriftServerHandler(rpcServerHandler, requestTimeout),
            maxFrameSize,
            assumeClientsSupportOutOfOrderResponses));
  }
}
