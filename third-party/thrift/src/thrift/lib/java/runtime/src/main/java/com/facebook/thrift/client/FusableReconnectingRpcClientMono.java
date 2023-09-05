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

package com.facebook.thrift.client;

import com.facebook.thrift.util.MonoTimeoutTransformer;
import com.facebook.thrift.util.resources.RpcResources;
import java.net.SocketAddress;
import java.time.Duration;
import java.util.Objects;
import java.util.Optional;
import java.util.Queue;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;
import java.util.concurrent.atomic.AtomicReferenceFieldUpdater;
import org.reactivestreams.Subscription;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.CoreSubscriber;
import reactor.core.Fuseable;
import reactor.core.Scannable;
import reactor.core.Scannable.Attr;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Operators;
import reactor.util.concurrent.Queues;

final class FusableReconnectingRpcClientMono extends Mono<RpcClient> implements Fuseable {

  private static final Logger LOGGER =
      LoggerFactory.getLogger(FusableReconnectingRpcClientMono.class);

  private static final AtomicIntegerFieldUpdater<FusableReconnectingRpcClientMono> WIP =
      AtomicIntegerFieldUpdater.newUpdater(FusableReconnectingRpcClientMono.class, "wip");

  private static final AtomicReferenceFieldUpdater<FusableReconnectingRpcClientMono, State> STATE =
      AtomicReferenceFieldUpdater.newUpdater(
          FusableReconnectingRpcClientMono.class, State.class, "state");

  private static final AtomicReferenceFieldUpdater<FusableReconnectingRpcClientMono, RpcClient>
      RPC_CLIENT =
          AtomicReferenceFieldUpdater.newUpdater(
              FusableReconnectingRpcClientMono.class, RpcClient.class, "rpcClient");

  private static final long MAX_TIMEOUT_MS = 30_000;

  private enum State {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
  }

  private final RpcClientFactory delegate;

  private final SocketAddress socketAddress;

  private final Queue<CoreSubscriber<? super RpcClient>> subscribers;

  private volatile RpcClient rpcClient;
  private volatile State state;

  private volatile long lastConnectAttemptTs;

  private volatile int wip;

  private volatile int retryCount;

  public FusableReconnectingRpcClientMono(RpcClientFactory delegate, SocketAddress socketAddress) {
    this.delegate = Objects.requireNonNull(delegate);
    this.socketAddress = Objects.requireNonNull(socketAddress);
    this.subscribers = Queues.<CoreSubscriber<? super RpcClient>>unboundedMultiproducer().get();
    this.state = State.DISCONNECTED;
  }

  @Override
  public void subscribe(CoreSubscriber<? super RpcClient> actual) {
    Objects.requireNonNull(actual);
    RpcClient current = this.rpcClient;
    if (state == State.CONNECTED && current != null) {
      fastPath(actual, current);
    } else {
      slowPath(actual);
    }
  }

  private void fastPath(CoreSubscriber<? super RpcClient> actual, RpcClient current) {
    Subscription subscription = Operators.scalarSubscription(actual, current);
    actual.onSubscribe(subscription);
  }

  private void slowPath(CoreSubscriber<? super RpcClient> actual) {
    actual.onSubscribe(new InnerSubscription(actual));
  }

  public void tryDrain() {
    if (WIP.getAndIncrement(this) == 0) {
      drain();
    }
  }

  private void drain() {
    int missed = 1;
    State state;

    do {
      state = this.state;
      if (state == State.CONNECTED) {
        drainWithRpcClient(rpcClient);
      }

      if (state == State.DISCONNECTED) {
        connect();
      }

      missed = WIP.addAndGet(this, -missed);
    } while (missed != 0);
  }

  private void drainWithRpcClient(RpcClient rpcClient) {
    while (true) {
      CoreSubscriber<? super RpcClient> subscriber = subscribers.poll();
      if (subscriber == null) {
        return;
      }

      assert rpcClient != null;

      emit(subscriber, rpcClient);
    }
  }

  private void emit(CoreSubscriber<? super RpcClient> subscriber, RpcClient rpcClient) {
    Objects.requireNonNull(subscriber, "cannot emit if subscriber is null");
    Objects.requireNonNull(rpcClient, "cannot emit if rpc client is null");
    try {
      subscriber.onNext(rpcClient);
      subscriber.onComplete();
    } catch (Throwable t) {
      LOGGER.error("uncaught error when emitting rpc client to subscriber", t);
      subscriber.onError(t);
    }
  }

  private void connect() {
    STATE.set(this, State.CONNECTING);

    Optional<Duration> duration = calculateDuration();

    Mono<RpcClient> connectionMono = delegate.createRpcClient(socketAddress);

    if (duration.isPresent()) {
      Duration d = duration.get();
      connectionMono =
          connectionMono.transform(
              new MonoTimeoutTransformer<>(
                  RpcResources.getClientOffLoopScheduler(), d.toMillis(), TimeUnit.MILLISECONDS));
    }

    connectionMono
        .subscribeOn(RpcResources.getOffLoopScheduler())
        .subscribe(this::handleIncomingRpcClient, this::handleConnectionError);
  }

  private void handleIncomingRpcClient(RpcClient rpcClient) {
    State oldState;
    RpcClient oldClient;
    boolean stateChanged;
    boolean clientUpdated;

    do {
      oldState = this.state;
      oldClient = this.rpcClient;
      stateChanged = STATE.compareAndSet(this, oldState, State.CONNECTED);
      clientUpdated = RPC_CLIENT.compareAndSet(this, oldClient, rpcClient);
    } while (!stateChanged && !clientUpdated);

    rpcClient.onClose().doFinally(s -> setStateDisconnected()).subscribe();

    tryDrain();
  }

  private void handleConnectionError(Throwable t) {
    LOGGER.error("error connecting to " + socketAddress, t);

    STATE.set(this, State.DISCONNECTED);

    tryDrain();
  }

  private void setStateDisconnected() {
    RpcClient oldClient = this.rpcClient;

    if (oldClient != null) {
      Scannable scannable = (Scannable) oldClient.onClose();
      if (scannable.scanOrDefault(Attr.TERMINATED, false)) {
        oldClient.dispose();
      }
    }

    STATE.set(this, State.DISCONNECTED);

    tryDrain();
  }

  private Optional<Duration> calculateDuration() {
    final long currentTimestamp = System.currentTimeMillis();
    final long timestamp = lastConnectAttemptTs;

    if (currentTimestamp - timestamp > MAX_TIMEOUT_MS) {
      retryCount = 0;
      lastConnectAttemptTs = currentTimestamp;
      return Optional.empty();
    }

    final int count = retryCount++;
    if (count > 0) {
      long backoffValue = calculateDuration(count);
      return Optional.of(Duration.ofMillis(backoffValue));
    } else {
      return Optional.empty();
    }
  }

  private long calculateDuration(final int count) {
    final double exp = Math.pow(2, count);
    final int jitter = ThreadLocalRandom.current().nextInt(1000);
    return (long) Math.min(exp + jitter, MAX_TIMEOUT_MS);
  }

  private class InnerSubscription extends AtomicBoolean implements Subscription {

    private final CoreSubscriber<? super RpcClient> actual;

    public InnerSubscription(CoreSubscriber<? super RpcClient> actual) {
      this.actual = actual;
    }

    @Override
    public void request(long n) {
      if (!isTerminated() && Operators.validate(n)) {
        subscribers.offer(actual);
        tryDrain();
      }
    }

    private boolean isTerminated() {
      return get();
    }

    @Override
    public void cancel() {
      set(true);
    }
  }
}
