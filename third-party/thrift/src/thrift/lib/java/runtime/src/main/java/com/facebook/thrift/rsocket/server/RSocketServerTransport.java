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

package com.facebook.thrift.rsocket.server;

import static java.util.Objects.requireNonNull;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.rsocket.transport.reactor.server.ReactorServerCloseable;
import com.facebook.thrift.rsocket.transport.reactor.server.ReactorServerTransport;
import com.facebook.thrift.server.RpcServerHandler;
import com.facebook.thrift.server.ServerTransport;
import com.facebook.thrift.util.SPINiftyMetrics;
import io.rsocket.core.RSocketServer;
import io.rsocket.frame.FrameLengthCodec;
import io.rsocket.frame.decoder.PayloadDecoder;
import java.net.SocketAddress;
import reactor.core.publisher.Mono;

public class RSocketServerTransport implements ServerTransport {

  private static final int MAX_FRAME_SIZE =
      Integer.parseInt(
          System.getProperty(
              "thrift.rsocket-max-frame-size", String.valueOf(FrameLengthCodec.FRAME_LENGTH_MASK)));

  private ReactorServerCloseable closable;

  RSocketServerTransport(ReactorServerCloseable closeable) {
    this.closable = closeable;
  }

  static Mono<RSocketServerTransport> createInstance(
      SocketAddress socketAddress,
      RpcServerHandler rpcServerHandler,
      ThriftServerConfig config,
      SPINiftyMetrics serverMetrics) {
    try {
      requireNonNull(rpcServerHandler, "methodInvoker is null");
      requireNonNull(config, "config is null");

      return RSocketServer.create(new ThriftSocketAcceptor(rpcServerHandler))
          .fragment(MAX_FRAME_SIZE)
          .payloadDecoder(PayloadDecoder.ZERO_COPY)
          .bind(new ReactorServerTransport(socketAddress, config, serverMetrics))
          .map(RSocketServerTransport::new);
    } catch (Exception e) {
      return Mono.error(e);
    }
  }

  @Override
  public SocketAddress getAddress() {
    return closable.getAddress();
  }

  @Override
  public Mono<Void> onClose() {
    return closable.onClose();
  }

  @Override
  public SPINiftyMetrics getNiftyMetrics() {
    return closable.getMetrics();
  }

  @Override
  public void dispose() {
    closable.dispose();
  }

  @Override
  public boolean isDisposed() {
    return closable.isDisposed();
  }
}
