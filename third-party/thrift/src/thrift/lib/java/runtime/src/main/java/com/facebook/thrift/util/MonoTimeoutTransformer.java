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

import com.facebook.thrift.util.resources.RpcResources;
import io.netty.channel.EventLoop;
import io.netty.util.HashedWheelTimer;
import io.netty.util.Timeout;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.function.Function;
import org.reactivestreams.Subscription;
import reactor.core.CoreSubscriber;
import reactor.core.Fuseable;
import reactor.core.publisher.BaseSubscriber;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Operators;
import reactor.core.publisher.SignalType;
import reactor.core.scheduler.Scheduler;

/**
 * TimeoutTransform is used to do I/O timeouts in the client and server that use Netty 4. It uses
 * the {@link HashedWheelTimer} to provide timeouts. When a timeout occurs it will schedule the
 * error on the provided Netty 4 {@link EventLoop}. There are two options when a timeout occurs. The
 * first option is this calls will emit a {@link TimeoutException}. The second option is that if
 * provide a Mono fallback it will subscribe to that when timed out.
 *
 * @param <T>
 */
public final class MonoTimeoutTransformer<T> implements Function<Mono<T>, Mono<T>> {
  private final Scheduler scheduler;
  private final long delay;
  private final TimeUnit unit;
  private final Mono<T> fallback;

  public MonoTimeoutTransformer(Scheduler scheduler, long delay, TimeUnit unit) {
    this(scheduler, delay, unit, null);
  }

  public MonoTimeoutTransformer(Scheduler scheduler, long delay, TimeUnit unit, Mono<T> fallback) {
    this.scheduler = scheduler;
    this.delay = delay;
    this.unit = unit;
    this.fallback = fallback;
  }

  @Override
  public Mono<T> apply(Mono<T> source) {
    return new TimeoutMono<>(source, scheduler, delay, unit, fallback);
  }

  private static class TimeoutMono<T> extends Mono<T> {
    private final Mono<T> source;
    private final Scheduler scheduler;
    private final long delay;
    private final TimeUnit unit;
    private final Mono<T> fallback;

    public TimeoutMono(
        Mono<T> source, Scheduler scheduler, long delay, TimeUnit unit, Mono<T> fallback) {
      this.source = source;
      this.scheduler = scheduler;
      this.delay = delay;
      this.unit = unit;
      this.fallback = fallback;
    }

    @Override
    public void subscribe(CoreSubscriber<? super T> actual) {
      if (RpcResources.enableOperatorFusion()) {
        // If the source mono is a ScalarCallable, there might not be a need for a timeout. Get the
        // value from the call() method. If it returns a value just emit that directly without
        // applying a timeout
        // because there's already a value.
        if (source instanceof Fuseable.ScalarCallable<?>) {
          Fuseable.ScalarCallable<T> callable = (Fuseable.ScalarCallable<T>) source;
          try {
            T t = callable.call();
            if (t != null) {
              Subscription subscription = Operators.scalarSubscription(actual, t);
              actual.onSubscribe(subscription);
              return;
            }
          } catch (Throwable t) {
            Operators.error(actual, t);
            return;
          }
        }
      }

      TimeoutSubscription<T> subscription =
          new TimeoutSubscription<>(source, actual, scheduler, delay, unit, fallback);
      actual.onSubscribe(subscription);
    }
  }

  private static class TimeoutSubscription<T> implements Subscription {
    private final Mono<T> source;
    private final CoreSubscriber<? super T> actual;
    private final Scheduler scheduler;
    private final long delay;
    private final TimeUnit unit;
    private final Mono<T> fallback;
    private SourceSubscriber<T> sourceSubscriber;

    public TimeoutSubscription(
        Mono<T> source,
        CoreSubscriber<? super T> actual,
        Scheduler scheduler,
        long delay,
        TimeUnit unit,
        Mono<T> fallback) {
      this.source = source;
      this.actual = actual;
      this.scheduler = scheduler;
      this.delay = delay;
      this.unit = unit;
      this.fallback = fallback;
    }

    @Override
    public void request(long ignore) {
      // ignore request n because any valid request n means emit.
      if (Operators.validate(ignore)) {
        sourceSubscriber = new SourceSubscriber<>(this.actual, scheduler, delay, unit, fallback);
        source.subscribe(sourceSubscriber);
      } else {
        Operators.error(actual, new IllegalArgumentException("invalid request n, " + ignore));
      }
    }

    @Override
    public void cancel() {
      if (sourceSubscriber != null) {
        sourceSubscriber.dispose();
      }
    }
  }

  private static class SourceSubscriber<T> extends BaseSubscriber<T> {
    private final CoreSubscriber<? super T> actual;
    private final Scheduler scheduler;
    private final long delay;
    private final TimeUnit unit;
    private final Mono<T> fallback;
    private final Timeout timeout;

    public SourceSubscriber(
        CoreSubscriber<? super T> actual,
        Scheduler scheduler,
        long delay,
        TimeUnit unit,
        Mono<T> fallback) {
      this.actual = actual;
      this.scheduler = scheduler;
      this.delay = delay;
      this.unit = unit;
      this.fallback = fallback;
      this.timeout = RpcResources.getHashedWheelTimer().newTimeout(this::doTimeout, delay, unit);
    }

    @Override
    protected void hookOnSubscribe(Subscription subscription) {
      subscription.request(Long.MAX_VALUE);
    }

    private void doTimeout(Timeout timeout) {
      if (!isDisposed() && !timeout.isCancelled()) {
        doTimeout();
      }
    }

    private void doTimeout() {
      if (fallback == null) {
        doTimeoutException();
      } else {
        doTimeoutFallback();
      }
    }

    private void doTimeoutException() {
      TimeoutException timeoutException =
          new TimeoutException("request timed out after " + delay + " " + unit.toString());
      scheduler.schedule(() -> actual.onError(timeoutException));
    }

    private void doTimeoutFallback() {
      scheduler.schedule(
          () -> {
            try {
              fallback.subscribe(actual::onNext, actual::onError, actual::onComplete);
            } catch (Throwable t) {
              actual.onError(t);
            }
          });
    }

    @Override
    protected void hookOnNext(T value) {
      timeout.cancel();
      actual.onNext(value);
    }

    @Override
    protected void hookOnComplete() {
      actual.onComplete();
    }

    @Override
    protected void hookOnError(Throwable throwable) {
      actual.onError(throwable);
    }

    @Override
    protected void hookFinally(SignalType type) {
      if (!timeout.isCancelled()) {
        timeout.cancel();
      }
    }
  }
}
