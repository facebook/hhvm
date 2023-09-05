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

import com.facebook.thrift.util.resources.RpcResources;
import java.net.SocketAddress;
import java.time.Duration;
import java.util.Objects;
import java.util.Optional;
import java.util.Queue;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;
import org.reactivestreams.Subscription;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import reactor.core.CoreSubscriber;
import reactor.core.Disposable;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Operators;
import reactor.util.concurrent.Queues;

final class ReconnectingRpcClientMono extends Mono<RpcClient> {
  private static final Logger LOGGER = LoggerFactory.getLogger(ReconnectingRpcClientMono.class);

  private static final AtomicIntegerFieldUpdater<ReconnectingRpcClientMono> WIP =
      AtomicIntegerFieldUpdater.newUpdater(ReconnectingRpcClientMono.class, "wip");

  private static final long RETRY_TIMEOUT_MS = 180_000;

  private static final long MAX_TIMEOUT_MS = 30_000;

  private static final long BASE_TIMEOUT_MS = 100;

  private enum State {
    DISCONNECTED,
    CONNECTED,
    CONNECTING
  }

  private final RpcClientFactory delegate;

  private final SocketAddress socketAddress;

  private final Queue<CoreSubscriber<? super RpcClient>> subscribers;

  private volatile RpcClient rpcClient;

  private volatile Disposable rpcClientOnCloseDisposable;

  private volatile State state;

  private volatile long lastConnectAttemptTs;

  private volatile int wip;

  private volatile int retryCount;

  public ReconnectingRpcClientMono(RpcClientFactory delegate, SocketAddress socketAddress) {
    this.delegate = Objects.requireNonNull(delegate);
    this.socketAddress = Objects.requireNonNull(socketAddress);
    this.subscribers = Queues.<CoreSubscriber<? super RpcClient>>unboundedMultiproducer().get();
    this.state = State.DISCONNECTED;
  }

  @Override
  public void subscribe(CoreSubscriber<? super RpcClient> actual) {
    Objects.requireNonNull(actual).onSubscribe(new InnerSubscription(actual));
  }

  private void changeState(State state) {
    this.state = state;
  }

  private void setStateDisconnected() {
    changeState(State.DISCONNECTED);

    if (rpcClient != null) {
      rpcClient.dispose();
      rpcClient = null;
    }
  }

  private void setStateConnected() {
    changeState(State.CONNECTED);

    assert rpcClient != null;

    tryDrain();
  }

  private void setStateConnecting() {
    changeState(State.CONNECTING);
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
      } else if (state == State.DISCONNECTED) {
        connect();
      }

      missed = WIP.addAndGet(this, -missed);
    } while (missed != 0);
  }

  private void drainWithRpcClient(RpcClient rpcClient) {
    while (true) {
      CoreSubscriber<? super RpcClient> subscriber = subscribers.peek();
      if (subscriber == null) {
        break;
      }

      if (rpcClient == null) {
        continue;
      }

      emit(subscriber, rpcClient);
    }
  }

  private void emit(CoreSubscriber<? super RpcClient> subscriber, RpcClient rpcClient) {
    Objects.requireNonNull(subscriber, "cannot emit if subscriber is null");
    Objects.requireNonNull(rpcClient, "cannot emit if rpc client is null");
    try {
      if (rpcClient.isDisposed()) {
        return;
      }
      subscriber.onNext(rpcClient);
      subscriber.onComplete();
      subscribers.poll();
    } catch (Throwable t) {
      LOGGER.error("uncaught error when emitting rpc client to subscriber");
    }
  }

  private void handleIncomingRpcClient(RpcClient rpcClient) {
    if (rpcClientOnCloseDisposable != null && !rpcClientOnCloseDisposable.isDisposed()) {
      rpcClientOnCloseDisposable.dispose();
    }

    this.rpcClient = rpcClient;
    this.rpcClientOnCloseDisposable =
        rpcClient.onClose().doFinally(s -> setStateDisconnected()).subscribe();

    setStateConnected();
  }

  private void handleConnectionError(Throwable t) {
    setStateDisconnected();

    LOGGER.info(
        "error connecting to {} due to {} (current state = {}, wip = {}, retry count = {})",
        socketAddress,
        (t != null && t.getMessage() != null) ? t.getMessage() : "unknown",
        state,
        wip,
        retryCount);

    if (!subscribers.isEmpty()) {
      tryDrain();
    }
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

  private void connect() {
    setStateConnecting();
    Optional<Duration> duration = calculateDuration();

    Mono<RpcClient> connectionMono = delegate.createRpcClient(socketAddress);

    if (duration.isPresent()) {
      connectionMono =
          Mono.delay(duration.get(), RpcResources.getOffLoopScheduler()).then(connectionMono);
    }

    connectionMono.subscribe(this::handleIncomingRpcClient, this::handleConnectionError);
  }

  private class InnerSubscription extends AtomicBoolean implements Subscription {
    private final CoreSubscriber<? super RpcClient> actual;

    public InnerSubscription(CoreSubscriber<? super RpcClient> actual) {
      this.actual = actual;
    }

    @Override
    public void request(long n) {
      if (!isCancelled() && Operators.validate(n)) {
        emit();
      }
    }

    private void emit() {
      final int wip = WIP.getAndIncrement(ReconnectingRpcClientMono.this);
      final RpcClient r = ReconnectingRpcClientMono.this.rpcClient;
      if (state == State.CONNECTED && wip == 0 && r != null) {
        fastPath(r);
      } else {
        slowPath(wip);
      }
    }

    private void fastPath(RpcClient rpcClient) {
      fastPathDrain(rpcClient);
      ReconnectingRpcClientMono.this.emit(actual, rpcClient);
    }

    private void fastPathDrain(RpcClient rpcClient) {
      int missed = 1;
      do {
        drainWithRpcClient(rpcClient);
        missed = WIP.addAndGet(ReconnectingRpcClientMono.this, -missed);
      } while (missed != 0);
    }

    private void slowPath(int wip) {
      subscribers.offer(actual);
      if (wip == 0) {
        drain();
      } else {
        tryDrain();
      }
    }

    private boolean isCancelled() {
      return get();
    }

    @Override
    public void cancel() {
      set(true);
    }
  }
}
