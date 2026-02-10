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

import static com.google.common.util.concurrent.MoreExecutors.directExecutor;

import com.facebook.nifty.core.RequestContext;
import com.facebook.nifty.core.RequestContexts;
import com.facebook.thrift.client.ResponseWrapper;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.MoreExecutors;
import com.google.common.util.concurrent.SettableFuture;
import com.google.common.util.concurrent.UncheckedExecutionException;
import java.util.Optional;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.function.Supplier;
import org.reactivestreams.Subscription;
import reactor.core.Fuseable.ScalarCallable;
import reactor.core.publisher.BaseSubscriber;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Operators;
import reactor.core.scheduler.Scheduler;
import reactor.util.context.Context;

public final class FutureUtil {

  /**
   * Takes a Supplier of ListenableFuture and schedules the future's execution on the {@link
   * reactor.core.scheduler.Scheduler} provided.
   *
   * @param futureSupplier a supplier that returns a ListenableFuture to be scheduled on another
   *     thread
   * @param scheduler a scheduler to schedule the ListenableFuture on
   * @param <T>
   * @return the mono wrapping the future
   */
  public static <T> Mono<T> toScheduledMono(
      final Supplier<ListenableFuture<T>> futureSupplier, Scheduler scheduler) {
    return Mono.<T>create(
            sink -> {
              ListenableFuture<T> future = futureSupplier.get();

              if (future.isDone()) {
                if (!future.isCancelled()) {
                  try {
                    final T t = future.get();
                    if (t == null) {
                      sink.success();
                    } else {
                      sink.success(t);
                    }
                  } catch (Throwable t) {
                    if (t instanceof ExecutionException) {
                      sink.error(t.getCause());
                    }
                    sink.error(t);
                  }
                } else {
                  sink.error(new CancellationException());
                }
              } else {
                sink.onDispose(() -> future.cancel(true));

                future.addListener(
                    () -> {
                      if (!future.isCancelled()) {
                        try {
                          final T t = future.get();
                          if (t == null) {
                            sink.success();
                          } else {
                            sink.success(t);
                          }
                        } catch (Throwable t) {
                          if (t instanceof ExecutionException) {
                            sink.error(t.getCause());
                          }
                          sink.error(t);
                        }
                      } else {
                        sink.error(new CancellationException());
                      }
                    },
                    MoreExecutors.directExecutor());
              }
            })
        .subscribeOn(scheduler);
  }

  public static <T> Mono<T> toMono(final Supplier<ListenableFuture<T>> futureSupplier) {
    return Mono.deferContextual(
        contextView -> {
          Optional<RequestContext> requestContext = RequestContext.tryContextView(contextView);
          requestContext.ifPresent(RequestContexts::setCurrentContext);
          return FutureUtil.toMono(futureSupplier.get());
        });
  }

  /**
   * Takes Google ListenableFuture and returns a reactor-core Mono
   *
   * @param future the future to wrap
   * @return the mono wrapping the future
   */
  public static <T> Mono<T> toMono(final ListenableFuture<T> future) {
    if (future.isDone()) {
      if (!future.isCancelled()) {
        try {
          final T t = future.get();
          if (t == null) {
            return Mono.empty();
          }
          return Mono.just(t);
        } catch (Throwable t) {
          if (t instanceof ExecutionException) {
            return Mono.error(t.getCause());
          } else if (t instanceof UncheckedExecutionException) {
            return Mono.error(t.getCause());
          }
          return Mono.error(t);
        }
      } else {
        return Mono.error(new CancellationException());
      }
    }

    return Mono.create(
        sink -> {
          sink.onDispose(() -> future.cancel(true));

          future.addListener(
              () -> {
                if (!future.isCancelled()) {
                  try {
                    final T t = future.get();
                    if (t == null) {
                      sink.success();
                    } else {
                      sink.success(t);
                    }
                  } catch (Throwable t) {
                    if (t instanceof ExecutionException) {
                      sink.error(t.getCause());
                    } else if (t instanceof UncheckedExecutionException) {
                      sink.error(t.getCause());
                    } else {
                      sink.error(t);
                    }
                  }
                } else {
                  sink.error(new CancellationException());
                }
              },
              MoreExecutors.directExecutor());
        });
  }

  /**
   * Takes a reactor-core Mono and returns a Google ListenableFuture
   *
   * @param mono the mono to wrap
   * @return The ListenableFuture wrapping the mono
   */
  public static <T> ListenableFuture<T> toListenableFuture(final Mono<T> mono) {
    SettableFuture<T> settableFuture = SettableFuture.create();
    try {
      if (mono instanceof ScalarCallable<?>) {
        ScalarCallable<T> scalarCallable = (ScalarCallable<T>) mono;
        T call = scalarCallable.call();
        settableFuture.set(call);
      } else {
        mono.subscribe(new ListenableFutureSubscriber<>(settableFuture));
      }
    } catch (Throwable t) {
      settableFuture.setException(t);
    }

    return settableFuture;
  }

  private static class ListenableFutureSubscriber<T> extends BaseSubscriber<T> {
    private final SettableFuture<T> settableFuture;

    public ListenableFutureSubscriber(SettableFuture<T> settableFuture) {
      this.settableFuture = settableFuture;
    }

    @Override
    protected void hookOnSubscribe(Subscription subscription) {
      // Propagate cancellation from ListenableFuture back to Mono subscription
      // This ensures resources (e.g., ByteBuf) are released when future is cancelled
      settableFuture.addListener(
          () -> {
            if (settableFuture.isCancelled()) {
              dispose();
            }
          },
          MoreExecutors.directExecutor());

      request(Long.MAX_VALUE);
    }

    @Override
    protected void hookOnNext(T value) {
      if (!settableFuture.isDone()) {
        settableFuture.set(value);
      } else {
        Operators.onNextDropped(value, Context.empty());
      }
    }

    @Override
    protected void hookOnError(Throwable throwable) {
      if (!settableFuture.isDone()) {
        settableFuture.setException(throwable);
      } else {
        Operators.onErrorDropped(throwable, Context.empty());
      }
    }

    @Override
    protected void hookOnCancel() {
      if (!settableFuture.isDone()) {
        settableFuture.cancel(true);
      }
    }

    @Override
    protected void hookOnComplete() {
      if (!settableFuture.isDone()) {
        settableFuture.set(null);
      }
    }
  }

  /**
   * Transforms Listenable<ResponseWrapper<T>> to ListenableFuture<T> as used by consumers
   *
   * @param wrapper
   * @param <T>
   * @return ListenableFuture<T> to be used by consumer.
   */
  public static <T> ListenableFuture<T> transform(ListenableFuture<ResponseWrapper<T>> wrapper) {
    return Futures.transform(
        wrapper,
        input -> {
          if (input != null) {
            return input.getData();
          } else {
            return null;
          }
        },
        directExecutor());
  }

  /**
   * Gets ResponseWrapper<T> from ListenableFuture<ResponseWrapper<T> unwrapping execution exception
   * if needed.
   *
   * @param future
   * @param <T>
   * @return ResponseWrapper<T> from future
   * @throws Throwable
   */
  public static <T> ResponseWrapper<T> get(ListenableFuture<ResponseWrapper<T>> future)
      throws Throwable {
    try {
      return future.get();
    } catch (ExecutionException e) {
      throw e.getCause();
    } catch (InterruptedException e) {
      Thread.currentThread().interrupt();
      throw e;
    }
  }
}
