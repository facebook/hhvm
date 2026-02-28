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

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.example.ping.PingServiceRpcServerHandler;
import com.facebook.thrift.legacy.server.LegacyServerTransport;
import com.facebook.thrift.legacy.server.LegacyServerTransportFactory;
import com.facebook.thrift.util.SPINiftyMetrics;
import io.netty.channel.unix.DomainSocketAddress;
import io.netty.util.ResourceLeakDetector;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.LockSupport;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

public class MultiUdsReactiveServer {
  public static int NUM_UDS = Runtime.getRuntime().availableProcessors() * 2;

  private static final Logger logger = LoggerFactory.getLogger(MultiUdsReactiveServer.class);

  private static final AtomicInteger index = new AtomicInteger();

  public static void main(String... args) {
    ResourceLeakDetector.setLevel(ResourceLeakDetector.Level.DISABLED);

    List<LegacyServerTransport> transports = new ArrayList<>();

    logger.info("create server handler");
    PingServiceRpcServerHandler serverHandler =
        new PingServiceRpcServerHandler(new PingServiceImpl(), Collections.emptyList());

    logger.info("starting servers");

    for (int i = 0; i < NUM_UDS; i++) {
      int ix = index.getAndIncrement();
      LegacyServerTransportFactory transportFactory =
          new LegacyServerTransportFactory(
              new ThriftServerConfig()
                  .setSslEnabled(false)
                  .setUdsPath("/tmp/uds_benchmark.socket" + ix));
      DomainSocketAddress socketAddress = new DomainSocketAddress("/tmp/uds_benchmark.socket" + ix);
      LegacyServerTransport transport =
          transportFactory
              .createServerTransport(socketAddress, serverHandler, new SPINiftyMetrics())
              .block();
      System.out.println("creating server listen on uds: " + socketAddress);
      transports.add(transport);
    }

    LockSupport.park();
  }

  public static class PingServiceImpl implements PingService.Reactive {
    @Override
    public void dispose() {}

    @Override
    public Mono<PingResponse> ping(PingRequest pingRequest) {
      return Mono.just(new PingResponse(pingRequest.getRequest()));
    }

    @Override
    public Mono<byte[]> pingBinary(PingRequest pingRequest) {
      return null;
    }

    @Override
    public Mono<PingResponse> pingException(PingRequest pingRequest) {
      return Mono.error(new UnsupportedOperationException("not implemented"));
    }

    @Override
    public Mono<Void> pingVoid(PingRequest pingRequest) {
      return null;
    }

    @Override
    public Flux<PingResponse> streamOfPings(PingRequest pingRequest, int i) {
      return null;
    }
  }
}
