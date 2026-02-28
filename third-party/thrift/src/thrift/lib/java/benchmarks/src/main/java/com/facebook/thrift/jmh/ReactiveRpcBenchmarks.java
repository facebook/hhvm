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

package com.facebook.thrift.jmh;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.client.RpcClientFactory;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.example.ping.PingServiceReactiveClient;
import com.facebook.thrift.example.ping.PingServiceRpcServerHandler;
import com.facebook.thrift.legacy.server.LegacyServerTransportFactory;
import com.facebook.thrift.util.SPINiftyMetrics;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.channel.unix.DomainSocketAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import org.apache.thrift.ProtocolId;
import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.Level;
import org.openjdk.jmh.annotations.Param;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.infra.Blackhole;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.core.scheduler.Schedulers;

public class ReactiveRpcBenchmarks {
  @State(Scope.Benchmark)
  public static class Input {
    private Blackhole bh;
    private PingService.Reactive pingService;

    @Param({"TBinary", "TCompact"})
    String protocol;

    @Param({"1000", "10000"})
    int count;

    @Param({"UDP", "TCP"})
    String socketType;

    ProtocolId protocolId;

    @Setup(Level.Trial)
    public void setup(Blackhole bh) {
      this.bh = bh;

      if ("TBinary".equals(protocol)) {
        protocolId = ProtocolId.BINARY;
      } else {
        protocolId = ProtocolId.COMPACT;
      }

      SocketAddress socketAddress;
      if ("UDP".equals(socketType)) {
        socketAddress = new DomainSocketAddress("/tmp/jmh.socket");
      } else {
        socketAddress = new InetSocketAddress(9000);
      }

      System.out.println("Starting Ping Service...");
      PingServiceRpcServerHandler serverHandler =
          new PingServiceRpcServerHandler(new ReactivePingService(), Collections.emptyList());

      LegacyServerTransportFactory factory =
          new LegacyServerTransportFactory(
              new ThriftServerConfig().setSslEnabled(false).setUdsPath("/tmp/jmh.socket"));
      factory.createServerTransport(socketAddress, serverHandler, new SPINiftyMetrics()).block();
      System.out.println("Ping Service started...");

      System.out.println("Connecting Ping Service Client...");
      this.pingService = createClient(socketAddress);
      System.out.println("Ping Service Client connected...");
    }

    private PingService.Reactive createClient(SocketAddress socketAddress) {
      RpcClientFactory clientFactory =
          RpcClientFactory.builder()
              .setThriftClientConfig(
                  new ThriftClientConfig().setDisableSSL(true).setProtocol(protocolId))
              .build();

      return new PingServiceReactiveClient(
          protocolId, clientFactory.createRpcClient(socketAddress));
    }

    @Benchmark
    public void benchmark(Input input) throws Exception {
      Object ping =
          Flux.range(0, count)
              .parallel(128)
              .runOn(Schedulers.fromExecutor(RpcResources.getEventLoopGroup()))
              .concatMap(
                  __ -> pingService.ping(new PingRequest.Builder().setRequest("ping").build()))
              .doOnError(Throwable::printStackTrace)
              .sequential()
              .blockLast();

      input.bh.consume(ping);
    }
  }

  private static class ReactivePingService implements PingService.Reactive {
    @Override
    public Mono<PingResponse> ping(PingRequest pingRequest) {
      return Mono.just(new PingResponse.Builder().setResponse("pong").build());
    }

    @Override
    public Mono<byte[]> pingBinary(PingRequest pingRequest) {
      return Mono.just("hello!".getBytes(StandardCharsets.UTF_8));
    }

    @Override
    public Mono<PingResponse> pingException(PingRequest pingRequest) {
      throw new UnsupportedOperationException();
    }

    @Override
    public Mono<Void> pingVoid(PingRequest pingRequest) {
      return Mono.empty();
    }

    @Override
    public Flux<PingResponse> streamOfPings(PingRequest request, int numberOfPings) {
      return Flux.empty();
    }

    @Override
    public void dispose() {}
  }
}
