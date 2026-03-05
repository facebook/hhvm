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

package com.facebook.thrift.util;

import com.facebook.thrift.exceptions.RpcTimeoutException;
import com.facebook.thrift.util.resources.RpcResources;
import com.google.common.annotations.VisibleForTesting;
import io.netty.util.Timeout;
import io.netty.util.Timer;
import io.netty.util.TimerTask;
import java.util.Objects;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;
import java.util.concurrent.atomic.AtomicReferenceFieldUpdater;
import java.util.function.Function;
import java.util.function.Supplier;
import org.reactivestreams.Subscription;
import reactor.core.CoreSubscriber;
import reactor.core.Fuseable;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Operators;
import reactor.core.scheduler.Scheduler;
import reactor.util.context.Context;

/**
 * A generic Mono transformer that enforces a timeout using the shared {@link
 * RpcResources#getHashedWheelTimer()}. If the timeout occurs, it signals a {@link TimeoutException}
 * or switches to a fallback Mono. The timeout execution (error emission or fallback subscription)
 * is offloaded to the provided {@link Scheduler}.
 *
 * @param <T> the value type
 *     <h2>Architecture Overview</h2>
 *     <pre>
 * ┌─────────────────────────────────────────────────────────────────────────────────────┐
 * │                              MonoTimeoutTransformer                                 │
 * │                                                                                     │
 * │  ┌─────────────┐      ┌───────────────────┐      ┌─────────────────────────────────┐│
 * │  │   Source    │─────▶│ TimeoutSubscriber │─────▶│         Downstream              ││
 * │  │    Mono     │      │   (CoreSubscriber,│      │       (CoreSubscriber)          ││
 * │  └─────────────┘      │    Subscription,  │      └─────────────────────────────────┘│
 * │                       │    TimerTask,     │                                         │
 * │                       │    Runnable)      │                                         │
 * │                       └────────┬──────────┘                                         │
 * │                                │                                                    │
 * │                                │ schedules                                          │
 * │                                ▼                                                    │
 * │                       ┌──────────────────┐                                          │
 * │                       │ HashedWheelTimer │  (Netty timer thread)                    │
 * │                       │  (RpcResources)  │                                          │
 * │                       └────────┬─────────┘                                          │
 * │                                │                                                    │
 * │                                │ on timeout, offloads to                            │
 * │                                ▼                                                    │
 * │                       ┌──────────────────┐                                          │
 * │                       │    Scheduler     │  (Reactor scheduler thread)              │
 * │                       │  (user-provided) │                                          │
 * │                       └──────────────────┘                                          │
 * └─────────────────────────────────────────────────────────────────────────────────────┘
 * </pre>
 *     <h2>State Machine</h2>
 *     <pre>
 *                    ┌───────────────────────────────────────────────────┐
 *                    │                                                   │
 *                    ▼                                                   │
 *              ┌──────────┐                                              │
 *              │   INIT   │◀─────────────────────────────────────────────┘
 *              │   (0)    │                    (initial state)
 *              └────┬─────┘
 *                   │
 *         ┌─────────┴─────────┬──────────────────┬──────────────────┐
 *         │                   │                  │                  │
 *         │ onNext()          │ timeout fires    │ onComplete()     │ cancel()
 *         │ [CAS succeeds]    │ [CAS succeeds]   │ [CAS succeeds]   │ [getAndSet]
 *         ▼                   │                  │                  │
 *   ┌─────────────┐           │                  │                  │
 *   │VALUE_EMITTED│           │                  │                  │
 *   │    (1)      │           │                  │                  │
 *   └──────┬──────┘           │                  │                  │
 *          │                  │                  │                  │
 *          │ onComplete()     │                  │                  │
 *          │ [CAS succeeds]   │                  │                  │
 *          ▼                  ▼                  ▼                  ▼
 *        ┌─────────────────────────────────────────────────────────────┐
 *        │                      TERMINATED (2)                         │
 *        │   (terminal state - no further state transitions allowed)   │
 *        └─────────────────────────────────────────────────────────────┘
 * </pre>
 *     <h2>Key Fields</h2>
 *     <ul>
 *       <li><b>STATE</b>: Atomic integer controlling the lifecycle. Only one actor can win the race
 *           to transition from INIT to TERMINATED (or VALUE_EMITTED).
 *       <li><b>S</b>: Atomic reference to the active Subscription. Holds either the source
 *           subscription, the fallback subscription, {@code TIMEOUT_MARKER} (during timeout
 *           transition), or {@code cancelledSubscription()} sentinel.
 *       <li><b>TIMEOUT_MARKER</b>: Distinct sentinel used when timeout fires to allow proper
 *           detection of downstream cancellation during the timeout-to-fallback transition.
 *     </ul>
 *     <h2>Common Flows</h2>
 *     <h3>Flow 1: Source emits value before timeout</h3>
 *     <pre>
 *   Source Thread                Timer Thread              Downstream
 *        │                            │                        │
 *        │─── onNext(value) ─────────▶│                        │
 *        │    [STATE: INIT → VALUE_EMITTED]                    │
 *        │    [cancel timer]          │                        │
 *        │                            │                        │
 *        │────────────────────────────┼───── onNext(value) ───▶│
 *        │                            │                        │
 *        │─── onComplete() ──────────▶│                        │
 *        │    [STATE: VALUE_EMITTED → TERMINATED]              │
 *        │                            │                        │
 *        │────────────────────────────┼──── onComplete() ─────▶│
 * </pre>
 *     <h3>Flow 2: Timeout fires before source (no fallback)</h3>
 *     <pre>
 *   Source Thread       Timer Thread (Netty)      Scheduler Thread       Downstream
 *        │                     │                        │                    │
 *        │                     │─ run(Timeout) ────────▶│                    │
 *        │                     │  [STATE: INIT → TERMINATED]                 │
 *        │                     │  [S → TIMEOUT_MARKER, cancel source]        │
 *        │                     │  [schedule(this)]      │                    │
 *        │                     │                        │                    │
 *        │                     │                        │─ run() ───────────▶│
 *        │                     │                        │  [emit TimeoutException]
 *        │                     │                        │                    │
 *        │── onNext(late) ────▶│                        │                    │
 *        │   [CAS fails, STATE is TERMINATED]           │                    │
 *        │   [value dropped]   │                        │                    │
 * </pre>
 *     <h3>Flow 3: Timeout fires, fallback succeeds</h3>
 *     <pre>
 *   Timer Thread          Scheduler Thread         Fallback Source        Downstream
 *        │                      │                        │                    │
 *        │─ run(Timeout) ──────▶│                        │                    │
 *        │  [STATE → TERMINATED]│                        │                    │
 *        │  [S → TIMEOUT_MARKER]│                        │                    │
 *        │                      │                        │                    │
 *        │                      │─ run() ───────────────▶│                    │
 *        │                      │  [S.compareAndSet(TIMEOUT_MARKER, null)]    │
 *        │                      │  [subscribe FallbackSubscriber]             │
 *        │                      │                        │                    │
 *        │                      │◀── onSubscribe(s) ─────│                    │
 *        │                      │    [Operators.set(S, parent, s)]            │
 *        │                      │    [s.request(MAX)]    │                    │
 *        │                      │                        │                    │
 *        │                      │◀── onNext(fallbackVal)─│                    │
 *        │                      │────────────────────────┼── onNext() ───────▶│
 *        │                      │                        │                    │
 *        │                      │◀── onComplete() ───────│                    │
 *        │                      │────────────────────────┼── onComplete() ───▶│
 * </pre>
 *     <h2>Race Conditions and Solutions</h2>
 *     <h3>Race 1: Timeout vs Source onNext</h3>
 *     <p>Both timer thread and source thread try to win simultaneously.
 *     <pre>
 *   Source Thread                          Timer Thread
 *        │                                      │
 *        │── onNext() ──────────────────────────│── run(Timeout) ──
 *        │   STATE.compareAndSet(INIT, VALUE)   │   STATE.compareAndSet(INIT, TERMINATED)
 *        │                                      │
 *        └──────────── ONE WINS (CAS) ──────────┘
 * </pre>
 *     <p><b>Solution:</b> Atomic CAS on STATE. Only one succeeds. Loser's signal is dropped.
 *     <h3>Race 2: cancel() during fallback subscription setup</h3>
 *     <p>Downstream calls cancel() while we're setting up the fallback.
 *     <pre>
 *   Scheduler Thread                              Downstream Thread
 *        │                                              │
 *        │─ S.compareAndSet(CANCELLED, null) ──────────▶│
 *        │   [S is now null]                            │
 *        │                                              │── cancel() ──
 *        │                                              │   [STATE already TERMINATED]
 *        │                                              │   [Operators.terminate(S)]
 *        │                                              │   [S → CANCELLED]
 *        │                                              │
 *        │─ fallback.subscribe(FallbackSubscriber) ────▶│
 *        │                                              │
 *        │◀── FallbackSubscriber.onSubscribe(s) ────────│
 *        │    [Operators.set(S, parent, s)]             │
 *        │    [S is CANCELLED → rejects, cancels s]     │
 *        │                                              │
 *        └────────── FALLBACK PROPERLY CANCELLED ───────┘
 * </pre>
 *     <p><b>Solution:</b>
 *     <ul>
 *       <li>{@code cancel()} ALWAYS calls {@code Operators.terminate(S, this)}, even if STATE was
 *           already TERMINATED. This ensures S becomes CANCELLED.
 *       <li>{@code FallbackSubscriber.onSubscribe()} uses {@code Operators.set()} which atomically
 *           checks if S is CANCELLED and rejects the subscription if so.
 *     </ul>
 *     <h3>Race 3: cancel() before S reset in fallback path</h3>
 *     <p>cancel() is called after timeout but before scheduler executes run().
 *     <pre>
 *   Timer Thread              Downstream Thread           Scheduler Thread
 *        │                          │                           │
 *        │─ run(Timeout) ──────────▶│                           │
 *        │  [S → TIMEOUT_MARKER]    │                           │
 *        │  [schedule(this)]        │                           │
 *        │                          │                           │
 *        │                          │── cancel() ──────────────▶│
 *        │                          │   [Operators.terminate(S)]│
 *        │                          │   [S: TIMEOUT_MARKER → CANCELLED]
 *        │                          │                           │
 *        │                          │                           │─ run() ──
 *        │                          │                           │  [S.compareAndSet(TIMEOUT_MARKER, null)]
 *        │                          │                           │  [FAILS - S is CANCELLED]
 *        │                          │                           │  [return without subscribing fallback]
 *        │                          │                           │
 *        └────────── FALLBACK PROPERLY SKIPPED ────────────────┘
 * </pre>
 *     <p><b>Solution:</b> Use a distinct {@code TIMEOUT_MARKER} sentinel in run(Timeout) instead of
 *     {@code cancelledSubscription()}. This allows {@code Operators.terminate()} called by {@code
 *     cancel()} to successfully transition S from TIMEOUT_MARKER to cancelledSubscription(). The
 *     scheduler's run() method detects this by attempting a CAS from TIMEOUT_MARKER to null; if the
 *     CAS fails, cancellation occurred and we skip the fallback.
 *     <h2>Fusion Optimization</h2>
 *     <p>When {@link RpcResources#enableOperatorFusion()} is enabled:
 *     <ul>
 *       <li><b>Source fusion:</b> If source is {@code Mono.just(v)} or {@code Mono.empty()}, emit
 *           immediately without scheduling a timer.
 *       <li><b>Fallback fusion:</b> If fallback is a scalar, emit directly on the scheduler thread
 *           without creating a FallbackSubscriber.
 *     </ul>
 */
public final class MonoTimeoutTransformer<T> implements Function<Mono<T>, Mono<T>> {

  private final Scheduler scheduler;
  private final long delay;
  private final TimeUnit unit;
  private final Mono<T> fallback;
  private final Supplier<Timer> timerSupplier;

  public MonoTimeoutTransformer(Scheduler scheduler, long delay, TimeUnit unit) {
    this(scheduler, delay, unit, null);
  }

  public MonoTimeoutTransformer(Scheduler scheduler, long delay, TimeUnit unit, Mono<T> fallback) {
    this(scheduler, delay, unit, fallback, RpcResources::getHashedWheelTimer);
  }

  /**
   * Package-private constructor for testing with a custom timer supplier.
   *
   * <p>The timer is obtained lazily at subscription time (not construction time) to handle cases
   * where {@link RpcResources#shutdown()} is called and then resources are re-initialized. This
   * ensures we always use the current active timer rather than a stale reference to a stopped
   * timer.
   *
   * @param scheduler the scheduler to offload timeout handling to
   * @param delay the timeout delay
   * @param unit the time unit for the delay
   * @param fallback optional fallback Mono to use on timeout
   * @param timerSupplier supplier that provides the timer at subscription time
   */
  @VisibleForTesting
  MonoTimeoutTransformer(
      Scheduler scheduler,
      long delay,
      TimeUnit unit,
      Mono<T> fallback,
      Supplier<Timer> timerSupplier) {
    this.scheduler = Objects.requireNonNull(scheduler, "scheduler");
    if (delay < 0) {
      throw new IllegalArgumentException("delay must be non-negative, was: " + delay);
    }
    this.delay = delay;
    this.unit = Objects.requireNonNull(unit, "unit");
    this.fallback = fallback;
    this.timerSupplier = Objects.requireNonNull(timerSupplier, "timerSupplier");
  }

  @Override
  public Mono<T> apply(Mono<T> source) {
    return new MonoTimeout<>(source, scheduler, delay, unit, fallback, timerSupplier);
  }

  /** Internal Operator implementation. */
  private static final class MonoTimeout<T> extends Mono<T> {
    private final Mono<T> source;
    private final Scheduler scheduler;
    private final long delay;
    private final TimeUnit unit;
    private final Mono<T> fallback;
    private final Supplier<Timer> timerSupplier;

    MonoTimeout(
        Mono<T> source,
        Scheduler scheduler,
        long delay,
        TimeUnit unit,
        Mono<T> fallback,
        Supplier<Timer> timerSupplier) {
      this.source = Objects.requireNonNull(source, "source");
      this.scheduler = scheduler;
      this.delay = delay;
      this.unit = unit;
      this.fallback = fallback;
      this.timerSupplier = timerSupplier;
    }

    @Override
    @SuppressWarnings("unchecked")
    public void subscribe(CoreSubscriber<? super T> actual) {
      // --- FUSION OPTIMIZATION START ---
      // If the source is a constant (Mono.just or Mono.empty), we don't need a timer.
      // We can resolve it immediately. This is "Macro-Fusion".
      if (RpcResources.enableOperatorFusion() && source instanceof Fuseable.ScalarCallable) {
        try {
          T value = ((Fuseable.ScalarCallable<T>) source).call();
          if (value == null) {
            Operators.complete(actual);
          } else {
            // Use specialized scalar subscription for max throughput
            actual.onSubscribe(Operators.scalarSubscription(actual, value));
          }
          return;
        } catch (Throwable t) {
          Operators.error(actual, t);
          return;
        }
      }
      // --- FUSION OPTIMIZATION END ---

      // Get timer lazily at subscription time to handle RpcResources restart
      Timer timer = timerSupplier.get();
      TimeoutSubscriber<T> parent =
          new TimeoutSubscriber<>(actual, delay, unit, scheduler, fallback, timer);
      actual.onSubscribe(parent);
      parent.start(source);
    }
  }

  /** The Subscriber that manages the race between the source and the Netty Timer. */
  private static final class TimeoutSubscriber<T>
      implements CoreSubscriber<T>, Subscription, TimerTask, Runnable {

    private final CoreSubscriber<? super T> actual;
    private final long delay;
    private final TimeUnit unit;
    private final Scheduler scheduler;
    private final Mono<T> fallback;

    // Manages the active subscription (Source or Fallback)
    volatile Subscription s;

    @SuppressWarnings("rawtypes")
    private static final AtomicReferenceFieldUpdater<TimeoutSubscriber, Subscription> S =
        AtomicReferenceFieldUpdater.newUpdater(TimeoutSubscriber.class, Subscription.class, "s");

    // Manages the state of the sequence
    volatile int state;
    private static final AtomicIntegerFieldUpdater<TimeoutSubscriber> STATE =
        AtomicIntegerFieldUpdater.newUpdater(TimeoutSubscriber.class, "state");

    private static final int STATE_INIT = 0;
    private static final int STATE_VALUE_EMITTED = 1;
    private static final int STATE_TERMINATED = 2;

    /**
     * Sentinel subscription used to mark that the timeout has fired and we're transitioning to the
     * fallback path. This is distinct from {@link Operators#cancelledSubscription()} to allow
     * proper detection of downstream cancellation during the timeout-to-fallback transition.
     *
     * <p>When timeout fires, S is set to TIMEOUT_MARKER. If downstream calls cancel(),
     * Operators.terminate() will successfully change S from TIMEOUT_MARKER to
     * cancelledSubscription(). The scheduler's run() method can then detect this by checking if its
     * CAS from TIMEOUT_MARKER fails.
     */
    private static final Subscription TIMEOUT_MARKER =
        new Subscription() {
          @Override
          public void request(long n) {
            // NOP - this is just a marker
          }

          @Override
          public void cancel() {
            // NOP - this is just a marker
          }

          @Override
          public String toString() {
            return "TIMEOUT_MARKER";
          }
        };

    private volatile Timeout timeoutTask;
    private final Timer timer;

    TimeoutSubscriber(
        CoreSubscriber<? super T> actual,
        long delay,
        TimeUnit unit,
        Scheduler scheduler,
        Mono<T> fallback,
        Timer timer) {
      this.actual = actual;
      this.delay = delay;
      this.unit = unit;
      this.scheduler = scheduler;
      this.fallback = fallback;
      this.timer = timer;
    }

    void start(Mono<T> source) {
      // Subscribe to source - timer is started in onSubscribe() after source setup completes.
      // This matches Reactor's timeout semantics where connection/setup time doesn't count
      // against the timeout, only the actual request/response time.
      source.subscribe(this);
    }

    @Override
    public Context currentContext() {
      return actual.currentContext();
    }

    @Override
    public void onSubscribe(Subscription s) {
      if (Operators.set(S, this, s)) {
        // Start timeout timer AFTER source has completed its subscription setup.
        // This ensures connection establishment time doesn't count against the timeout,
        // matching Reactor's built-in timeout() semantics.
        this.timeoutTask = timer.newTimeout(this, delay, unit);
        s.request(Long.MAX_VALUE);
      }
    }

    @Override
    public void onNext(T t) {
      if (STATE.compareAndSet(this, STATE_INIT, STATE_VALUE_EMITTED)) {
        cancelTimer();
        actual.onNext(t);
      } else {
        Operators.onNextDropped(t, actual.currentContext());
      }
    }

    @Override
    public void onComplete() {
      int current = state;
      if (current == STATE_VALUE_EMITTED) {
        if (STATE.compareAndSet(this, STATE_VALUE_EMITTED, STATE_TERMINATED)) {
          actual.onComplete();
        }
      } else if (STATE.compareAndSet(this, STATE_INIT, STATE_TERMINATED)) {
        cancelTimer();
        actual.onComplete();
      }
    }

    @Override
    public void onError(Throwable t) {
      if (STATE.getAndSet(this, STATE_TERMINATED) != STATE_TERMINATED) {
        cancelTimer();
        actual.onError(t);
      } else {
        Operators.onErrorDropped(t, actual.currentContext());
      }
    }

    @Override
    public void request(long n) {
      Subscription current = s;
      if (current != null) {
        current.request(n);
      }
    }

    @Override
    public void cancel() {
      // 1. Try to transition to TERMINATED to prevent TimeoutTask from winning later.
      int prev = STATE.getAndSet(this, STATE_TERMINATED);

      // 2. If we were NOT already terminated (meaning we were INIT or VALUE_EMITTED),
      //    we need to cancel the timer to prevent it from firing.
      if (prev != STATE_TERMINATED) {
        cancelTimer();
      }

      // 3. ALWAYS terminate the subscription holder.
      //    - If normal cancel: Cancels the upstream source.
      //    - If late cancel (fallback active): Cancels the fallback subscription.
      Operators.terminate(S, this);
    }

    private void cancelTimer() {
      Timeout task = this.timeoutTask;
      if (task != null) {
        task.cancel();
      }
    }

    // --- TimerTask Implementation (Netty Thread) ---
    @Override
    public void run(Timeout timeout) {
      // Try to win the race against onNext/onComplete/onError
      if (STATE.compareAndSet(this, STATE_INIT, STATE_TERMINATED)) {
        // 1. We won. Cancel the upstream source immediately to stop work.
        //    Use TIMEOUT_MARKER (not cancelledSubscription) so that if downstream
        //    calls cancel(), Operators.terminate() can successfully transition
        //    S from TIMEOUT_MARKER to cancelledSubscription(), which we detect in run().
        Subscription current = S.getAndSet(this, TIMEOUT_MARKER);
        if (current != null && current != Operators.cancelledSubscription()) {
          current.cancel();
        }

        // 2. Offload the timeout signal/fallback execution to the Reactor Scheduler
        try {
          scheduler.schedule(this);
        } catch (Throwable t) {
          actual.onError(
              new RpcTimeoutException(
                  "Timeout occurred but Scheduler rejected execution: " + t.getMessage()));
        }
      }
    }

    // --- Runnable Implementation (Scheduler Thread) ---
    @Override
    @SuppressWarnings("unchecked")
    public void run() {
      // Check if downstream cancelled between timeout firing and scheduler execution.
      // If cancel() was called, Operators.terminate() changed S from TIMEOUT_MARKER
      // to cancelledSubscription().
      if (s == Operators.cancelledSubscription()) {
        return; // Downstream canceled, don't emit anything
      }

      if (fallback == null) {
        // No fallback - emit timeout error using stackless singleton for performance
        actual.onError(RpcTimeoutException.INSTANCE);
        return;
      }

      // --- FALLBACK FUSION OPTIMIZATION ---
      // If fallback is a scalar value and fusion is enabled, emit immediately.
      if (RpcResources.enableOperatorFusion() && fallback instanceof Fuseable.ScalarCallable) {
        try {
          T v = ((Fuseable.ScalarCallable<T>) fallback).call();
          if (v == null) {
            actual.onComplete();
          } else {
            actual.onNext(v);
            actual.onComplete();
          }
        } catch (Throwable t) {
          actual.onError(t);
        }
        return;
      }

      // --- STANDARD FALLBACK ---
      // CAS from TIMEOUT_MARKER to null. If this fails, it means cancel() was called
      // and S is now cancelledSubscription() - we should not proceed with fallback.
      if (S.compareAndSet(this, TIMEOUT_MARKER, null)) {
        // We do NOT reset STATE to INIT. We keep it TERMINATED.
        // This ensures that if the original source (if it behaved badly) tries to emit,
        // this.onNext() will reject it.
        // The FallbackSubscriber bypasses this.onNext() and talks directly to actual.
        fallback.subscribe(new FallbackSubscriber<>(actual, this));
      }
    }

    /**
     * A simple pass-through subscriber for the fallback. It bridges the fallback's signals directly
     * to the downstream, bypassing the state checks of the parent TimeoutSubscriber.
     *
     * <p>Includes defensive cancellation checks to ensure signals are not delivered after
     * downstream has cancelled.
     */
    private static final class FallbackSubscriber<T> implements CoreSubscriber<T> {
      private final CoreSubscriber<? super T> actual;
      private final TimeoutSubscriber<T> parent;

      FallbackSubscriber(CoreSubscriber<? super T> actual, TimeoutSubscriber<T> parent) {
        this.actual = actual;
        this.parent = parent;
      }

      @Override
      public void onSubscribe(Subscription s) {
        // Use Operators.set to register the fallback subscription on the parent.
        // If S is already cancelledSubscription() (due to cancel() being called),
        // Operators.set() will cancel the incoming subscription and return false.
        if (Operators.set(S, parent, s)) {
          s.request(Long.MAX_VALUE);
        }
        // If set() returned false, s was already cancelled by Operators.set()
      }

      @Override
      public void onNext(T t) {
        // Defensive check: don't deliver if cancelled
        if (parent.s == Operators.cancelledSubscription()) {
          Operators.onNextDropped(t, actual.currentContext());
          return;
        }
        actual.onNext(t);
      }

      @Override
      public void onError(Throwable t) {
        // Defensive check: don't deliver if cancelled
        if (parent.s == Operators.cancelledSubscription()) {
          Operators.onErrorDropped(t, actual.currentContext());
          return;
        }
        actual.onError(t);
      }

      @Override
      public void onComplete() {
        // Defensive check: don't deliver if cancelled
        if (parent.s == Operators.cancelledSubscription()) {
          return;
        }
        actual.onComplete();
      }

      @Override
      public Context currentContext() {
        return actual.currentContext();
      }
    }
  }
}
