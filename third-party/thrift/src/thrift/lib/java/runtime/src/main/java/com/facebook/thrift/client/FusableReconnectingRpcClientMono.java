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

  @SuppressWarnings("rawtypes")
  private static final AtomicReferenceFieldUpdater<
          FusableReconnectingRpcClientMono, ConnectionState>
      CONNECTION_STATE =
          AtomicReferenceFieldUpdater.newUpdater(
              FusableReconnectingRpcClientMono.class, ConnectionState.class, "connectionState");

  @SuppressWarnings("rawtypes")
  private static final AtomicIntegerFieldUpdater<InnerSubscription> ENQUEUED =
      AtomicIntegerFieldUpdater.newUpdater(InnerSubscription.class, "enqueued");

  private static final Duration MAX_BACKOFF = Duration.ofSeconds(30);

  /** Fixed timeout for individual connection attempts, independent of backoff delay. */
  private static final Duration CONNECT_TIMEOUT = Duration.ofSeconds(10);

  private enum Phase {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
  }

  /**
   * Immutable composite of connection phase and client reference. By combining both fields into a
   * single object, all state transitions are a single CAS on one AtomicReferenceFieldUpdater — no
   * torn reads between phase and client are possible.
   */
  private static final class ConnectionState {
    final Phase phase;
    final RpcClient client;

    ConnectionState(Phase phase, RpcClient client) {
      this.phase = phase;
      this.client = client;
    }
  }

  private static final ConnectionState INITIAL_STATE =
      new ConnectionState(Phase.DISCONNECTED, null);

  private final RpcClientFactory delegate;

  private final SocketAddress socketAddress;

  private final Queue<InnerSubscription> subscribers;

  private volatile ConnectionState connectionState;
  private volatile String previousExceptionMessage = null;

  private volatile long lastConnectAttemptTs;

  private volatile int wip;

  private volatile int retryCount;

  public FusableReconnectingRpcClientMono(RpcClientFactory delegate, SocketAddress socketAddress) {
    this.delegate = Objects.requireNonNull(delegate);
    this.socketAddress = Objects.requireNonNull(socketAddress);
    this.subscribers = Queues.<InnerSubscription>unboundedMultiproducer().get();
    this.connectionState = INITIAL_STATE;
  }

  @Override
  public void subscribe(CoreSubscriber<? super RpcClient> actual) {
    Objects.requireNonNull(actual);
    ConnectionState snapshot = this.connectionState;
    if (snapshot.phase == Phase.CONNECTED
        && snapshot.client != null
        && !snapshot.client.isDisposed()) {
      fastPath(actual, snapshot.client);
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

    do {
      ConnectionState snapshot = this.connectionState;
      if (snapshot.phase == Phase.CONNECTED) {
        if (snapshot.client != null && snapshot.client.isDisposed()) {
          // Client is disposed but onClose() hasn't fired yet (race window).
          // Transition to DISCONNECTED to trigger reconnection.
          trySetStateDisconnected(snapshot);
          snapshot = this.connectionState;
        } else {
          drainWithRpcClient(snapshot.client);
        }
      }

      // Prune cancelled subscriptions while not CONNECTED. Without this, cancelled
      // subscriptions (e.g., from downstream timeouts) accumulate during prolonged
      // outages and leak memory.
      if (snapshot.phase != Phase.CONNECTED) {
        pruneCancelledSubscribers();
      }

      if (snapshot.phase == Phase.DISCONNECTED) {
        connect(snapshot);
      }

      missed = WIP.addAndGet(this, -missed);
    } while (missed != 0);
  }

  /**
   * Removes cancelled subscriptions from the queue. Called during DISCONNECTED/CONNECTING phases to
   * prevent unbounded queue growth during prolonged outages.
   */
  private void pruneCancelledSubscribers() {
    int limit = subscribers.size();
    for (int i = 0; i < limit; i++) {
      InnerSubscription sub = subscribers.poll();
      if (sub == null) {
        break;
      }
      if (!sub.isCancelled()) {
        subscribers.offer(sub);
      }
    }
  }

  private void drainWithRpcClient(RpcClient rpcClient) {
    while (true) {
      // Check if client died mid-drain. If so, stop emitting — remaining subscribers
      // should wait for the next reconnect rather than receiving a dead client.
      if (rpcClient != null && rpcClient.isDisposed()) {
        return;
      }

      InnerSubscription subscription = subscribers.poll();
      if (subscription == null) {
        return;
      }

      // Skip cancelled subscriptions — do not signal them.
      if (subscription.isCancelled()) {
        continue;
      }

      assert rpcClient != null;

      emit(subscription.actual, rpcClient);
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

  private void connect(ConnectionState expected) {
    // Use null client in CONNECTING state. The old client reference serves no purpose here
    // and would cause the onClose identity guard to false-match during CONNECTING, triggering
    // a spurious disconnect and duplicate connection attempt.
    ConnectionState connecting = new ConnectionState(Phase.CONNECTING, null);
    if (!CONNECTION_STATE.compareAndSet(this, expected, connecting)) {
      return; // another thread already started a connection or changed state
    }

    // Capture this attempt's identity for the callbacks. Each ConnectionState is a unique
    // object, so the CAS in handleIncomingRpcClient/handleConnectionError will fail if
    // a concurrent onClose callback has already moved the state forward.
    final ConnectionState thisAttempt = connecting;

    Optional<Duration> backoffDelay = calculateDuration();

    Mono<RpcClient> connectionMono = delegate.createRpcClient(socketAddress);

    // Apply backoff as a delay BEFORE the connection attempt, not as a timeout on it.
    // Previously MonoTimeoutTransformer was used, which made the backoff act as an
    // increasingly strict timeout — actively cancelling slow-but-valid TCP handshakes
    // and providing zero delay on instant failures (connection refused).
    if (backoffDelay.isPresent()) {
      connectionMono =
          connectionMono.delaySubscription(backoffDelay.get(), RpcResources.getOffLoopScheduler());
    }

    // Apply a fixed, generous timeout to the actual connection attempt.
    connectionMono = connectionMono.timeout(CONNECT_TIMEOUT, RpcResources.getOffLoopScheduler());

    connectionMono
        .subscribeOn(RpcResources.getOffLoopScheduler())
        .subscribe(
            client -> handleIncomingRpcClient(client, thisAttempt),
            error -> handleConnectionError(error, thisAttempt));
  }

  private void handleIncomingRpcClient(RpcClient rpcClient, ConnectionState attempt) {
    previousExceptionMessage = null;

    // CAS from this specific attempt to CONNECTED. If the state has changed since this
    // attempt started (e.g., a concurrent onClose triggered another connect), the CAS
    // fails and this stale result is discarded.
    ConnectionState connected = new ConnectionState(Phase.CONNECTED, rpcClient);
    if (!CONNECTION_STATE.compareAndSet(this, attempt, connected)) {
      // Stale connection attempt — another connect already succeeded or state changed.
      // Dispose the client we just created to avoid leaking it.
      rpcClient.dispose();
      return;
    }

    // On close: CAS-loop to atomically transition from the exact state we observe.
    // This eliminates the TOCTOU race where another thread advances the state between
    // our read and the disconnect write.
    rpcClient
        .onClose()
        .doFinally(
            s -> {
              while (true) {
                ConnectionState current = this.connectionState;
                if (current.client != rpcClient) {
                  return; // another client is installed, this close is stale
                }
                if (trySetStateDisconnected(current)) {
                  return;
                }
                // CAS failed — state changed concurrently, re-read and retry
              }
            })
        .subscribe();

    tryDrain();
  }

  private void handleConnectionError(Throwable t, ConnectionState attempt) {
    // Log exception only if this is the first time we see this exception OR exception message is
    // different
    String currentExceptionMessage = t != null && t.getMessage() != null ? t.getMessage() : "";
    if (previousExceptionMessage == null
        || !previousExceptionMessage.equals(currentExceptionMessage)) {
      previousExceptionMessage = currentExceptionMessage;
      LOGGER.error("error connecting to " + socketAddress, t);
    }

    // CAS from this specific attempt to DISCONNECTED. If the state has changed (e.g.,
    // a concurrent attempt already succeeded), the CAS fails and we do nothing — the
    // stale error must not tear down a good connection.
    ConnectionState disconnected = new ConnectionState(Phase.DISCONNECTED, null);
    if (!CONNECTION_STATE.compareAndSet(this, attempt, disconnected)) {
      return;
    }

    tryDrain();
  }

  /**
   * Atomically transitions from the expected state to DISCONNECTED. Returns true if the CAS
   * succeeded, false if the state was concurrently modified. Callers that observe a stale state
   * should either retry or bail out.
   */
  private boolean trySetStateDisconnected(ConnectionState expected) {
    RpcClient oldClient = expected.client;

    if (oldClient != null) {
      // Use Scannable.from() instead of a hard cast to avoid ClassCastException if the
      // Mono returned by onClose() does not implement Scannable directly.
      Scannable scannable = Scannable.from(oldClient.onClose());
      if (scannable.scanOrDefault(Attr.TERMINATED, false)) {
        oldClient.dispose();
      }
    }

    ConnectionState disconnected = new ConnectionState(Phase.DISCONNECTED, null);
    if (!CONNECTION_STATE.compareAndSet(this, expected, disconnected)) {
      return false;
    }

    tryDrain();

    return true;
  }

  private Optional<Duration> calculateDuration() {
    final long currentTimestamp = System.currentTimeMillis();
    final long timestamp = lastConnectAttemptTs;

    if (currentTimestamp - timestamp > MAX_BACKOFF.toMillis()) {
      retryCount = 0;
      lastConnectAttemptTs = currentTimestamp;
      return Optional.empty();
    }

    final int count = retryCount++;
    if (count > 0) {
      return Optional.of(calculateBackoff(count));
    } else {
      return Optional.empty();
    }
  }

  private Duration calculateBackoff(final int count) {
    final double exp = Math.pow(2, count);
    final int jitter = ThreadLocalRandom.current().nextInt(1000);
    long millis = (long) Math.min(exp + jitter, MAX_BACKOFF.toMillis());
    return Duration.ofMillis(millis);
  }

  /**
   * Per-subscription state that tracks enqueued and cancelled flags. The queue holds
   * InnerSubscription (not raw CoreSubscriber), so drain can skip cancelled entries and request(n)
   * can prevent duplicate enqueuing.
   */
  private class InnerSubscription extends AtomicBoolean
      implements Fuseable.QueueSubscription<RpcClient> {

    final CoreSubscriber<? super RpcClient> actual;
    volatile int enqueued;

    public InnerSubscription(CoreSubscriber<? super RpcClient> actual) {
      this.actual = actual;
    }

    @Override
    public void request(long n) {
      if (!isCancelled() && Operators.validate(n)) {
        // Atomically enqueue once. CAS prevents duplicate queue entries if request(n)
        // is called concurrently (Reactive Streams rule 3.6 allows multiple request calls).
        if (ENQUEUED.compareAndSet(this, 0, 1)) {
          subscribers.offer(this);
          tryDrain();
        }
      }
    }

    boolean isCancelled() {
      return get();
    }

    @Override
    public void cancel() {
      set(true);
    }

    @Override
    public int requestFusion(int requestedMode) {
      return NONE;
    }

    @Override
    public RpcClient poll() {
      return null;
    }

    @Override
    public int size() {
      return 0;
    }

    @Override
    public boolean isEmpty() {
      return true;
    }

    @Override
    public void clear() {}
  }
}
