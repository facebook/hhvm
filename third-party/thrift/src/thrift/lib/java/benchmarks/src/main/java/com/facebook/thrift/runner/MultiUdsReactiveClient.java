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

import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.example.ping.PingRequest;
import com.facebook.thrift.example.ping.PingResponse;
import com.facebook.thrift.example.ping.PingService;
import com.facebook.thrift.example.ping.PingServiceReactiveClient;
import com.facebook.thrift.legacy.client.LegacyRpcClientFactory;
import io.netty.channel.unix.DomainSocketAddress;
import io.netty.util.ResourceLeakDetector;
import java.net.SocketAddress;
import java.time.Duration;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import org.apache.thrift.ProtocolId;
import reactor.core.publisher.Mono;

public class MultiUdsReactiveClient extends AbstractClient<PingService.Reactive> {
  private static final AtomicInteger index = new AtomicInteger();

  public static void main(String... args) {
    ResourceLeakDetector.setLevel(ResourceLeakDetector.Level.DISABLED);
    if (args.length < 2) {
      throw new IllegalArgumentException(
          "Incorrect number of arguments. <warmupSeconds> <benchmarkSeconds>");
    }
    int warmupSeconds = Integer.parseInt(args[0]);
    int benchmarkSeconds = Integer.parseInt(args[1]);

    MultiUdsReactiveClient c = new MultiUdsReactiveClient();
    c.runTest(
        new DomainSocketAddress("/tmp/uds_benchmark.socket"),
        Duration.ofSeconds(warmupSeconds),
        Duration.ofSeconds(benchmarkSeconds));
  }

  private Map<DomainSocketAddress, PingServiceReactiveClient> clients = new ConcurrentHashMap<>();

  @Override
  protected PingService.Reactive getClient(SocketAddress ignore) {

    int andIncrement = index.getAndIncrement() % MultiUdsReactiveServer.NUM_UDS;
    final DomainSocketAddress address =
        new DomainSocketAddress("/tmp/uds_benchmark.socket" + andIncrement);
    return clients.computeIfAbsent(
        address,
        __ -> {
          System.out.println("creating client using uds: " + address);
          LegacyRpcClientFactory rpcClientFactory =
              new LegacyRpcClientFactory(
                  new ThriftClientConfig()
                      .setDisableSSL(true)
                      .setRequestTimeout(
                          io.airlift.units.Duration.succinctDuration(1, TimeUnit.DAYS)));

          return new PingServiceReactiveClient(
              ProtocolId.BINARY, rpcClientFactory.createRpcClient(address).cache());
        });
  }

  @Override
  protected Mono<PingResponse> getResponse(PingService.Reactive reactive, String message) {
    return reactive.ping(new PingRequest(message));
  }
}
