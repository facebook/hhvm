/*
 * Copyright (c) 2017-2021 VMware Inc. or its affiliates, All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.facebook.thrift.util.resources;

import static org.assertj.core.api.Assertions.*;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.TimeUnit;
import org.junit.Assert;
import org.junit.Test;
import reactor.core.Disposable;
import reactor.core.Exceptions;
import reactor.core.scheduler.Scheduler;
import reactor.core.scheduler.Schedulers;

/** @author Stephane Maldini */
public abstract class AbstractSchedulerTest {

  protected abstract Scheduler scheduler();

  protected boolean shouldCheckInterrupted() {
    return false;
  }

  protected boolean shouldCheckDisposeTask() {
    return true;
  }

  protected boolean shouldCheckMassWorkerDispose() {
    return true;
  }

  protected boolean shouldCheckDirectTimeScheduling() {
    return true;
  }

  protected boolean shouldCheckWorkerTimeScheduling() {
    return true;
  }

  @Test(timeout = 10000)
  public final void directScheduleAndDispose() throws Exception {
    Scheduler s = scheduler();

    try {
      assertThat(s.isDisposed()).isFalse();
      CountDownLatch latch = new CountDownLatch(1);
      CountDownLatch latch2 = shouldCheckDisposeTask() ? new CountDownLatch(1) : null;

      try {
        Disposable d =
            s.schedule(
                () -> {
                  try {
                    latch.countDown();
                    if (latch2 != null
                        && !latch2.await(10, TimeUnit.SECONDS)
                        && shouldCheckInterrupted()) {
                      Assert.fail("Should have interrupted");
                    }
                  } catch (InterruptedException e) {
                  }
                });

        latch.await();
        if (shouldCheckDisposeTask()) {
          assertThat(d.isDisposed()).isFalse();
        } else {
          d.isDisposed(); // noop
        }
        d.dispose();
        d.dispose(); // noop
      } catch (Throwable schedulingError) {
        fail("unexpected scheduling error", schedulingError);
      }

      Thread.yield();

      if (latch2 != null) {
        latch2.countDown();
      }

      s.dispose();
      s.dispose(); // noop

      if (s == Schedulers.immediate()) {
        return;
      }
      assertThat(s.isDisposed()).isTrue();

      try {
        Disposable d =
            s.schedule(
                () -> {
                  if (shouldCheckInterrupted()) {
                    try {
                      Thread.sleep(10000);
                    } catch (InterruptedException e) {
                      Thread.currentThread().interrupt();
                    }
                  }
                });

        d.dispose();
        assertThat(d.isDisposed()).isTrue();
      } catch (Throwable schedulingError) {
        assertThat(schedulingError).isInstanceOf(RejectedExecutionException.class);
      }
    } finally {
      s.dispose();
      s.dispose(); // noop
    }
  }

  @Test(timeout = 10000)
  public final void workerScheduleAndDispose() throws Exception {
    Scheduler s = scheduler();
    try {
      Scheduler.Worker w = s.createWorker();

      assertThat(w.isDisposed()).isFalse();
      CountDownLatch latch = new CountDownLatch(1);
      CountDownLatch latch2 = shouldCheckDisposeTask() ? new CountDownLatch(1) : null;

      try {
        Disposable d =
            w.schedule(
                () -> {
                  try {
                    latch.countDown();
                    if (latch2 != null
                        && !latch2.await(10, TimeUnit.SECONDS)
                        && shouldCheckInterrupted()) {
                      Assert.fail("Should have interrupted");
                    }
                  } catch (InterruptedException e) {
                  }
                });

        latch.await();
        if (shouldCheckDisposeTask()) {
          assertThat(d.isDisposed()).isFalse();
        }
        d.dispose();
        d.dispose(); // noop
      } catch (Throwable schedulingError) {
        fail("unexpected scheduling error", schedulingError);
      }

      Thread.yield();

      if (latch2 != null) {
        latch2.countDown();
      }

      Disposable[] massCancel;
      boolean hasErrors = false;
      if (shouldCheckMassWorkerDispose()) {
        int n = 10;
        massCancel = new Disposable[n];
        Throwable[] errors = new Throwable[n];
        Thread current = Thread.currentThread();
        for (int i = 0; i < n; i++) {
          try {
            massCancel[i] =
                w.schedule(
                    () -> {
                      if (current == Thread.currentThread()) {
                        return;
                      }
                      try {
                        Thread.sleep(5000);
                      } catch (InterruptedException ie) {

                      }
                    });
          } catch (RejectedExecutionException ree) {
            errors[i] = ree;
            hasErrors = true;
          }
        }
      } else {
        massCancel = null;
      }
      w.dispose();
      w.dispose(); // noop
      assertThat(w.isDisposed()).isTrue();

      if (massCancel != null) {
        assertThat(hasErrors).as("mass cancellation errors").isFalse();
        for (Disposable _d : massCancel) {
          assertThat(_d.isDisposed()).isTrue();
        }
      }

      assertThatExceptionOfType(RejectedExecutionException.class)
          .isThrownBy(() -> w.schedule(() -> {}))
          .isSameAs(Exceptions.failWithRejected());
    } finally {
      s.dispose();
      s.dispose(); // noop
    }
  }

  @Test(timeout = 10000)
  public final void directScheduleAndDisposeDelay() throws Exception {
    Scheduler s = scheduler();

    try {
      assertThat(s.isDisposed()).isFalse();

      if (!shouldCheckDirectTimeScheduling()) {
        assertThatExceptionOfType(RejectedExecutionException.class)
            .isThrownBy(() -> s.schedule(() -> {}, 10, TimeUnit.MILLISECONDS))
            .as("Scheduler marked as not supporting time scheduling")
            .isSameAs(Exceptions.failWithRejected());
        return;
      }

      CountDownLatch latch = new CountDownLatch(1);
      CountDownLatch latch2 = new CountDownLatch(1);
      Disposable d =
          s.schedule(
              () -> {
                try {
                  latch.countDown();
                  latch2.await(10, TimeUnit.SECONDS);
                } catch (InterruptedException e) {
                }
              },
              10,
              TimeUnit.MILLISECONDS);
      // will throw if not scheduled

      latch.await();
      assertThat(d.isDisposed()).isFalse();
      d.dispose();

      Thread.yield();

      latch2.countDown();

      s.dispose();
      assertThat(s.isDisposed()).isTrue();

      assertThatExceptionOfType(RejectedExecutionException.class)
          .isThrownBy(() -> s.schedule(() -> {}));
    } finally {
      s.dispose();
    }
  }

  @Test(timeout = 10000)
  public final void workerScheduleAndDisposeDelay() throws Exception {
    Scheduler s = scheduler();
    Scheduler.Worker w = s.createWorker();

    try {
      assertThat(w.isDisposed()).isFalse();

      if (!shouldCheckWorkerTimeScheduling()) {
        assertThatExceptionOfType(RejectedExecutionException.class)
            .isThrownBy(() -> w.schedule(() -> {}, 10, TimeUnit.MILLISECONDS))
            .as("Worker marked as not supporting time scheduling")
            .isSameAs(Exceptions.failWithRejected());
        return;
      }

      CountDownLatch latch = new CountDownLatch(1);
      CountDownLatch latch2 = new CountDownLatch(1);
      Disposable d =
          w.schedule(
              () -> {
                try {
                  latch.countDown();
                  latch2.await(10, TimeUnit.SECONDS);
                } catch (InterruptedException e) {
                }
              },
              10,
              TimeUnit.MILLISECONDS);
      // will throw if rejected

      latch.await();
      assertThat(d.isDisposed()).isFalse();
      d.dispose();

      Thread.yield();

      latch2.countDown();

      w.dispose();
      assertThat(w.isDisposed()).isTrue();

      assertThatExceptionOfType(RejectedExecutionException.class)
          .isThrownBy(() -> w.schedule(() -> {}))
          .isSameAs(Exceptions.failWithRejected());
    } finally {
      w.dispose();
      s.dispose();
    }
  }

  @Test(timeout = 10000)
  public final void directScheduleAndDisposePeriod() throws Exception {
    Scheduler s = scheduler();

    try {
      assertThat(s.isDisposed()).isFalse();

      if (!shouldCheckDirectTimeScheduling()) {
        assertThatExceptionOfType(RejectedExecutionException.class)
            .isThrownBy(() -> s.schedule(() -> {}, 10, TimeUnit.MILLISECONDS))
            .as("Scheduler marked as not supporting time scheduling")
            .isSameAs(Exceptions.failWithRejected());
        return;
      }

      CountDownLatch latch = new CountDownLatch(2);
      CountDownLatch latch2 = new CountDownLatch(1);
      Disposable d =
          s.schedulePeriodically(
              () -> {
                try {
                  latch.countDown();
                  if (latch.getCount() == 0) {
                    latch2.await(10, TimeUnit.SECONDS);
                  }
                } catch (InterruptedException e) {
                }
              },
              10,
              10,
              TimeUnit.MILLISECONDS);
      // will throw if rejected

      assertThat(d.isDisposed()).isFalse();

      latch.await();
      d.dispose();

      Thread.yield();

      latch2.countDown();

      s.dispose();
      assertThat(s.isDisposed()).isTrue();

      assertThatExceptionOfType(RejectedExecutionException.class)
          .isThrownBy(() -> s.schedule(() -> {}));
    } finally {
      s.dispose();
    }
  }

  @Test(timeout = 10000)
  public final void workerScheduleAndDisposePeriod() throws Exception {
    Scheduler s = scheduler();
    Scheduler.Worker w = s.createWorker();

    try {
      assertThat(w.isDisposed()).isFalse();

      if (!shouldCheckWorkerTimeScheduling()) {
        assertThatExceptionOfType(RejectedExecutionException.class)
            .isThrownBy(() -> w.schedule(() -> {}, 10, TimeUnit.MILLISECONDS))
            .as("Worker marked as not supporting time scheduling")
            .isSameAs(Exceptions.failWithRejected());
        return;
      }

      CountDownLatch latch = new CountDownLatch(1);
      CountDownLatch latch2 = new CountDownLatch(1);
      Disposable c =
          w.schedulePeriodically(
              () -> {
                try {
                  latch.countDown();
                  latch2.await(10, TimeUnit.SECONDS);
                } catch (InterruptedException e) {
                }
              },
              10,
              10,
              TimeUnit.MILLISECONDS);
      Disposable d = c;
      // will throw if rejected

      latch.await();
      assertThat(d.isDisposed()).isFalse();
      d.dispose();

      Thread.yield();

      latch2.countDown();

      w.dispose();
      assertThat(w.isDisposed()).isTrue();

      assertThatExceptionOfType(RejectedExecutionException.class)
          .isThrownBy(() -> w.schedule(() -> {}))
          .isSameAs(Exceptions.failWithRejected());
    } finally {
      w.dispose();
      s.dispose();
    }
  }
}
