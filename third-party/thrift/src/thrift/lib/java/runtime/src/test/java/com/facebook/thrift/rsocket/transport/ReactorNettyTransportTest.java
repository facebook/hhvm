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

package com.facebook.thrift.rsocket.transport;

import com.facebook.swift.service.ThriftServerConfig;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.rsocket.transport.reactor.client.ReactorClientTransport;
import com.facebook.thrift.rsocket.transport.reactor.server.ReactorServerCloseable;
import com.facebook.thrift.rsocket.transport.reactor.server.ReactorServerTransport;
import com.facebook.thrift.util.RpcServerUtils;
import com.facebook.thrift.util.SPINiftyMetrics;
import io.rsocket.Payload;
import io.rsocket.RSocket;
import io.rsocket.core.RSocketConnector;
import io.rsocket.core.RSocketServer;
import io.rsocket.util.DefaultPayload;
import java.net.InetSocketAddress;
import java.time.Duration;
import java.util.Objects;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;
import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import reactor.core.Disposable;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Hooks;
import reactor.core.publisher.Mono;
import reactor.core.scheduler.Schedulers;
import reactor.test.StepVerifier;

public class ReactorNettyTransportTest {
  private static final int NULL_METADATA = 0;
  private static final int EMPTY_STRING_METADATA = 1;
  private static final String MOCK_DATA = "test-data";
  private static final String MOCK_METADATA = "metadata";
  private static final String LARGE_DATA = generate();
  private static final Payload LARGE_PAYLOAD = DefaultPayload.create(LARGE_DATA, LARGE_DATA);

  private final TransportPair TRANSPORT_PAIR = new TransportPair();

  @Before
  public void setUp() {
    Hooks.onOperatorDebug();
  }

  @After
  public void close() {
    getTransportPair().dispose();
    Hooks.resetOnOperatorDebug();
  }

  // makes 10 fireAndForget requests
  @Test
  public void fireAndForget10() {
    Flux.range(1, 10)
        .flatMap(i -> getClient().fireAndForget(createTestPayload(i)))
        .as(StepVerifier::create)
        .expectNextCount(0)
        .expectComplete()
        .verify(getTimeout());
  }

  // makes 10 fireAndForget with Large Payload in Requests
  @Test
  public void largePayloadFireAndForget10() {
    Flux.range(1, 10)
        .flatMap(i -> getClient().fireAndForget(LARGE_PAYLOAD))
        .as(StepVerifier::create)
        .expectNextCount(0)
        .expectComplete()
        .verify(getTimeout());
  }

  @Test
  public void metadataPush10() {
    Flux.range(1, 10)
        .flatMap(i -> getClient().metadataPush(DefaultPayload.create("", "test-metadata")))
        .as(StepVerifier::create)
        .expectNextCount(0)
        .expectComplete()
        .verify(getTimeout());
  }

  @Test
  public void largePayloadMetadataPush10() {
    Flux.range(1, 10)
        .flatMap(i -> getClient().metadataPush(DefaultPayload.create("", LARGE_DATA)))
        .as(StepVerifier::create)
        .expectNextCount(0)
        .expectComplete()
        .verify(getTimeout());
  }

  @Test
  public void requestChannel0() {
    getClient()
        .requestChannel(Flux.empty())
        .as(StepVerifier::create)
        .expectNextCount(0)
        .expectErrorMatches(
            error ->
                (error instanceof CancellationException)
                    && error.getMessage().contains("Empty Source"))
        .verify(getTimeout());
  }

  @Test
  public void requestChannel1() {
    getClient()
        .requestChannel(Mono.just(createTestPayload(0)))
        .as(StepVerifier::create)
        .expectNextCount(1)
        .expectComplete()
        .verify(getTimeout());
  }

  @Test
  public void requestChannel200_000() {
    Flux<Payload> payloads = Flux.range(0, 200_000).map(this::createTestPayload);

    getClient()
        .requestChannel(payloads)
        .as(StepVerifier::create)
        .expectNextCount(200_000)
        .expectComplete()
        .verify(getTimeout());
  }

  @Test
  public void largePayloadRequestChannel200() {
    Flux<Payload> payloads = Flux.range(0, 200).map(__ -> LARGE_PAYLOAD);

    getClient()
        .requestChannel(payloads)
        .as(StepVerifier::create)
        .expectNextCount(200)
        .expectComplete()
        .verify(getTimeout());
  }

  @Test
  public void requestChannel20_000() {
    AtomicInteger sent = new AtomicInteger();

    Flux<Payload> payloads =
        Flux.range(0, 20_000)
            .map(metadataPresent -> createTestPayload(7))
            .doOnNext(
                payload -> {
                  if (sent.getAndIncrement() % 1_000 == 0) {
                    System.out.println("sent count -> " + sent);
                  }
                });

    getClient()
        .requestChannel(payloads)
        .doOnNext(this::assertChannelPayload)
        .as(StepVerifier::create)
        .expectNextCount(20_000)
        .expectComplete()
        .verify(getTimeout());
  }

  @Test
  public void requestChannel3() {
    AtomicLong requested = new AtomicLong();
    Flux<Payload> payloads =
        Flux.range(0, 3).doOnRequest(requested::addAndGet).map(this::createTestPayload);

    getClient()
        .requestChannel(payloads)
        .as(publisher -> StepVerifier.create(publisher, 3))
        .expectNextCount(3)
        .expectComplete()
        .verify(getTimeout());

    Assert.assertEquals(3L, requested.get());
  }

  @Test
  public void requestChannel512() {
    Flux<Payload> payloads = Flux.range(0, 512).map(this::createTestPayload);

    Flux.range(0, 1024)
        .flatMap(
            v -> Mono.fromRunnable(() -> check(payloads)).subscribeOn(Schedulers.elastic()), 12)
        .blockLast();
  }

  @Test
  public void requestResponse1() {
    getClient()
        .requestResponse(createTestPayload(1))
        .doOnNext(this::assertPayload)
        .as(StepVerifier::create)
        .expectNextCount(1)
        .expectComplete()
        .verify(getTimeout());
  }

  @Test
  public void requestResponse10() {
    Flux.range(1, 10)
        .flatMap(
            i -> getClient().requestResponse(createTestPayload(i)).doOnNext(this::assertPayload))
        .as(StepVerifier::create)
        .expectNextCount(10)
        .expectComplete()
        .verify(getTimeout());
  }

  @Test
  public void requestResponse100() {
    Flux.range(1, 100)
        .flatMap(i -> getClient().requestResponse(createTestPayload(i)).map(Payload::getDataUtf8))
        .as(StepVerifier::create)
        .expectNextCount(100)
        .expectComplete()
        .verify(getTimeout());
  }

  @Test
  public void largePayloadRequestResponse100() {
    Flux.range(1, 100)
        .flatMap(i -> getClient().requestResponse(LARGE_PAYLOAD).map(Payload::getDataUtf8))
        .as(StepVerifier::create)
        .expectNextCount(100)
        .expectComplete()
        .verify(getTimeout());
  }

  @Test
  public void requestResponse10_000() {
    Flux.range(1, 10_000)
        .flatMap(i -> getClient().requestResponse(createTestPayload(i)).map(Payload::getDataUtf8))
        .as(StepVerifier::create)
        .expectNextCount(10_000)
        .expectComplete()
        .verify(getTimeout());
  }

  @Test
  public void requestStream10_000() {
    getClient()
        .requestStream(createTestPayload(3))
        .doOnNext(this::assertPayload)
        .as(StepVerifier::create)
        .expectNextCount(10_000)
        .expectComplete()
        .verify(getTimeout());
  }

  @Test
  public void requestStream5() {
    getClient()
        .requestStream(createTestPayload(3))
        .doOnNext(this::assertPayload)
        .take(5)
        .as(StepVerifier::create)
        .expectNextCount(5)
        .expectComplete()
        .verify(getTimeout());
  }

  @Test
  public void requestStreamDelayedRequestN() {
    getClient()
        .requestStream(createTestPayload(3))
        .take(10)
        .as(StepVerifier::create)
        .thenRequest(5)
        .expectNextCount(5)
        .thenRequest(5)
        .expectNextCount(5)
        .expectComplete()
        .verify(getTimeout());
  }

  RSocket getClient() {
    return getTransportPair().getClient();
  }

  public Duration getTimeout() {
    return Duration.ofMinutes(2);
  }

  public TransportPair getTransportPair() {
    return TRANSPORT_PAIR;
  }

  void check(Flux<Payload> payloads) {
    getClient()
        .requestChannel(payloads)
        .as(StepVerifier::create)
        .expectNextCount(512)
        .as("expected 512 items")
        .expectComplete()
        .verify(getTimeout());
  }

  void assertPayload(Payload p) {
    TransportPair transportPair = getTransportPair();
    if (!transportPair.expectedPayloadData().equals(p.getDataUtf8())
        || !transportPair.expectedPayloadMetadata().equals(p.getMetadataUtf8())) {
      throw new IllegalStateException("Unexpected payload");
    }
  }

  void assertChannelPayload(Payload p) {
    if (!MOCK_DATA.equals(p.getDataUtf8()) || !MOCK_METADATA.equals(p.getMetadataUtf8())) {
      throw new IllegalStateException("Unexpected payload");
    }
  }

  private static String generate() {
    byte[] bytes = new byte[1 << 18];
    for (int i = 0; i < bytes.length; i++) {
      byte b = (byte) ThreadLocalRandom.current().nextInt(33, 126);
      bytes[i] = b;
    }

    return new String(bytes);
  }

  private Payload createTestPayload(int metadataPresent) {
    String metadata;

    switch (metadataPresent % 5) {
      case NULL_METADATA:
        metadata = null;
        break;
      case EMPTY_STRING_METADATA:
        metadata = "";
        break;
      default:
        metadata = MOCK_METADATA;
    }

    return DefaultPayload.create(MOCK_DATA, metadata);
  }

  static final class TransportPair implements Disposable {
    private static final String data = "hello world";
    private static final String metadata = "metadata";

    private final RSocket client;

    private final ReactorServerCloseable server;

    public TransportPair() {

      server =
          RSocketServer.create((setup, sendingSocket) -> Mono.just(new TestRSocket(data, metadata)))
              .bind(
                  new ReactorServerTransport(
                      new InetSocketAddress("localhost", RpcServerUtils.findFreePort()),
                      new ThriftServerConfig().setSslEnabled(false),
                      new SPINiftyMetrics()))
              .block();
      client =
          RSocketConnector.connectWith(
                  new ReactorClientTransport(
                      Objects.requireNonNull(server).getAddress(),
                      new ThriftClientConfig().setDisableSSL(true)))
              .doOnError(Throwable::printStackTrace)
              .block();
    }

    @Override
    public void dispose() {
      server.dispose();
    }

    RSocket getClient() {
      return client;
    }

    public String expectedPayloadData() {
      return data;
    }

    public String expectedPayloadMetadata() {
      return metadata;
    }
  }
}
