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

import com.facebook.thrift.legacy.server.LegacyServerTransport;
import com.facebook.thrift.util.SPINiftyMetrics;
import io.netty.channel.Channel;
import io.netty.channel.ChannelFuture;
import java.net.SocketAddress;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.publisher.Mono;

public abstract class BaseServerTransport implements ServerTransport {
  private static final Logger logger = LoggerFactory.getLogger(LegacyServerTransport.class);
  private final Channel channel;
  private final ChannelFuture closeFuture;
  private final Mono<Void> onClose;
  private final SPINiftyMetrics metrics;

  protected BaseServerTransport(
      Channel channel, ChannelFuture closeFuture, Mono<Void> onClose, SPINiftyMetrics metrics) {
    this.channel = channel;
    this.closeFuture = closeFuture;
    this.onClose = onClose;
    this.metrics = metrics;
  }

  @Override
  public SocketAddress getAddress() {
    return channel.localAddress();
  }

  @Override
  public Mono<Void> onClose() {
    return onClose;
  }

  @Override
  public SPINiftyMetrics getNiftyMetrics() {
    return metrics;
  }

  @Override
  public void dispose() {
    channel.close();
  }

  @Override
  public boolean isDisposed() {
    return closeFuture.isDone();
  }
}
