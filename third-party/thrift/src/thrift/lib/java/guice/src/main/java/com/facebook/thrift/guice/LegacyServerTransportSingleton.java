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

package com.facebook.thrift.guice;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.legacy.server.LegacyServerTransport;
import com.facebook.thrift.legacy.server.LegacyServerTransportFactory;
import com.facebook.thrift.server.RpcServerHandler;
import com.google.inject.Inject;
import com.google.inject.Singleton;
import io.netty.channel.unix.DomainSocketAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

@Singleton
public class LegacyServerTransportSingleton implements ServerTransportSingleton {
  private static final Logger LOGGER =
      LoggerFactory.getLogger(LegacyServerTransportSingleton.class);

  private LegacyServerTransport serverTransport;

  @Inject
  public LegacyServerTransportSingleton(RpcServerHandler handler, ThriftServerConfig config) {
    try {
      LegacyServerTransportFactory factory = new LegacyServerTransportFactory(config);

      SocketAddress address =
          config.isEnableUDS()
              ? new DomainSocketAddress(config.getUdsPath())
              : new InetSocketAddress(config.getBindAddress(), config.getPort());

      this.serverTransport = factory.createServerTransport(address, handler).block();

      assert serverTransport != null;
      LOGGER.info("Legacy Thrift Java Server bound to address " + serverTransport.getAddress());
    } catch (Throwable t) {
      LOGGER.error("error starting Thrift Java server using Legacy Server Transport", t);
    }
  }
}
