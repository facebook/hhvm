/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executors;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;

import com.facebook.watchman.bser.BserDeserializer;
import com.facebook.watchman.bser.BserSerializer;

import com.google.common.base.Optional;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;
import org.hamcrest.Matchers;
import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

public class WatchmanConnectionTest extends WatchmanTestBase {

  private static final String CLOCK = "c:123:1234";

  @Rule
  public ExpectedException thrown = ExpectedException.none();

  /**
   * Test that sending a request with a response available will actually return that response.
   */
  @SuppressWarnings("unchecked")
  @Test
  public void sendCommandTest() throws ExecutionException, InterruptedException, IOException {
    Map<String, Object> mockResponse = new HashMap<>();
    mockResponse.put("clock", CLOCK);
    mObjectQueue.put(mockResponse);

    WatchmanConnection connection = new WatchmanConnection(mIncomingMessageGetter, mOutgoingMessageStream);
    connection.start();
    Map<String, Object> receivedResponse = connection.run(Arrays.asList("clock", "/a/b/c")).get();

    deepObjectEquals(mockResponse, receivedResponse);
  }

  /**
   * Test that requesting two commands to be sent will block the latter until the former receives
   * a response.
   */
  @Test
  public void singleCommandTest() throws IOException, ExecutionException, InterruptedException {
    final List<Object> firstMessage = Arrays.<Object>asList("clock", "/a/b/c");
    final List<Object> secondMessage = Arrays.<Object>asList("watch-project", "/a/b/c");
    final Map<String, Object> firstResponse = new HashMap<>();
    firstResponse.put("clock", CLOCK);
    final Semaphore commandSentSemaphore = new Semaphore(0);

    WatchmanConnection connection = new WatchmanConnection(
        mIncomingMessageGetter,
        mOutgoingMessageStream,
        Optional.<WatchmanConnection.WatchmanCommandListener>of(
            new WatchmanConnection.WatchmanCommandListener() {

              @Override
              public void onStart() {

              }

              @Override
              public void onSent() {
                commandSentSemaphore.release();
              }

              @Override
              public void onReceived() {

              }
            }));
    connection.start();
    ListenableFuture<Map<String, Object>> firstFuture = connection.run(firstMessage);
    ListenableFuture<Map<String, Object>> secondFuture = connection.run(secondMessage);

    ByteArrayOutputStream expected = new ByteArrayOutputStream();
    BserSerializer serializer = new BserSerializer();
    serializer.serializeToStream(firstMessage, expected);
    commandSentSemaphore.acquire();
    Assert.assertArrayEquals(expected.toByteArray(), this.mOutgoingMessageStream.toByteArray());

    mObjectQueue.put(new HashMap<String, Object>());
    firstFuture.get();
    serializer.serializeToStream(secondMessage, expected);
    commandSentSemaphore.acquire();
    Assert.assertArrayEquals(expected.toByteArray(), this.mOutgoingMessageStream.toByteArray());
  }

  /**
   * Test that receiving a message which contains an "error" label will result in an error thrown.
   */
  @SuppressWarnings("ThrowableInstanceNeverThrown")
  @Test
  public void exceptionThrownTest() throws InterruptedException, ExecutionException {
    final String reason = "please throw";
    WatchmanException expected = new WatchmanException(reason);
    thrown.expect(ExecutionException.class);
    thrown.expectCause(Matchers.allOf(
        Matchers.isA(WatchmanException.class),
        Matchers.hasToString(expected.toString())));

    final List<Object> request = Arrays.<Object>asList("clock", "/a/b/c"); // irrelevant
    final Map<String, Object> response = new HashMap<>();
    response.put("error", reason);
    mObjectQueue.put(response);

    WatchmanConnection connection = new WatchmanConnection(mIncomingMessageGetter, mOutgoingMessageStream);
    connection.start();

    connection.run(request).get();
  }

  /**
   * Test that receiving a unilateral update with a request under processing will deliver the
   * unilateral message to the unilateral handler and **then** return the request's response to the
   * requester.
   */
  @Test
  public void testUnilateralMessageBeforeResponse()
      throws ExecutionException, InterruptedException {
    final String unilateralLabel = "unifoo";
    final List<String> request = Collections.singletonList("version");
    final Map<String, Object> response = ImmutableMap.<String, Object>of("version", "irrelevant");
    final Map<String, Object> unilateral =
        ImmutableMap.<String, Object>of(unilateralLabel, "bar");

    // following two are AtomicBooleans instead of booleans because we need them final (used in
    // anonymous inner classes) and non-final; synchronization is offered by the Semaphore
    final AtomicBoolean callbackConditionRespected = new AtomicBoolean(false);
    final AtomicBoolean responseConditionRespected = new AtomicBoolean(false);
    final AtomicBoolean callbackHasRun = new AtomicBoolean(false);
    final AtomicBoolean responseHasRun = new AtomicBoolean(false);
    final Semaphore conditionSemaphore = new Semaphore(1);
    final CountDownLatch waitedResponses = new CountDownLatch(2);

    Callback callbackListener = new Callback() {
      @Override
      public void call(Map<String, Object> message) throws Exception {
        try {
          conditionSemaphore.acquire();
          callbackConditionRespected.set(!responseHasRun.get());
          callbackHasRun.set(true);
          conditionSemaphore.release();
        } catch (InterruptedException ignored) {
        } finally {
          waitedResponses.countDown();
        }
      }
    };

    WatchmanConnection connection = new WatchmanConnection(
        mIncomingMessageGetter,
        mOutgoingMessageStream,
        Optional.<Collection<String>>of(Collections.singletonList(unilateralLabel)),
        Optional.<Callback>of(callbackListener));
    connection.start();

    connection.run(request).addListener(
        new Runnable() {
          @Override
          public void run() {
            try {
              conditionSemaphore.acquire();
              responseConditionRespected.set(callbackHasRun.get());
              responseHasRun.set(true);
              conditionSemaphore.release();
            } catch (InterruptedException ignored) {
            } finally {
              waitedResponses.countDown();
            }
          }
        },
        Executors.newSingleThreadExecutor());

    mObjectQueue.put(unilateral);
    mObjectQueue.put(response);

    waitedResponses.await();
    Assert.assertTrue(callbackConditionRespected.get());
    Assert.assertTrue(responseConditionRespected.get());
  }

  /**
   * Test that receiving a WatchmanConnection#close before the commands in the queue have finished
   * executing will make all the commands throw with an error. Also, only the first command should
   * have been sent over the wire, and since no response is provided, the subsequent commands are
   * blocked by the first one waiting.
   *
   * Note that we must wait for the first message to have successfully been transmitted.
   */
  @Test
  public void testCurrentRequestCanceled()
      throws InterruptedException, IOException {
    final List<String> request = ImmutableList.of("version");
    final int nrRequests = 5;
    final CountDownLatch requestSpinner = new CountDownLatch(nrRequests);
    final CountDownLatch closeSpinner = new CountDownLatch(nrRequests);
    final Semaphore oneMessageSent = new Semaphore(0);

    WatchmanConnection connection = new WatchmanConnection(
        mIncomingMessageGetter,
        mOutgoingMessageStream,
        Optional.<WatchmanConnection.WatchmanCommandListener>of(
            new WatchmanConnection.WatchmanCommandListener() {

              @Override
              public void onStart() {}

              @Override
              public void onSent() {
                requestSpinner.countDown();
                oneMessageSent.release();
              }

              @Override
              public void onReceived() {}
            }));
    connection.start();

    List<AtomicBoolean> conditions = new ArrayList<AtomicBoolean>();
    for (int i=0; i<nrRequests; ++i) {
      final AtomicBoolean conditionRespected = new AtomicBoolean(false);
      conditions.add(conditionRespected);
      ListenableFuture<Map<String, Object>> futureRequest = connection.run(request);
      Futures.addCallback(
          futureRequest,
          new FutureCallback<Map<String, Object>>() {
            @Override
            public void onSuccess(Map<String, Object> result) {
              conditionRespected.set(false);
              closeSpinner.countDown();
            }

            @Override
            public void onFailure(Throwable t) {
              conditionRespected.set(true);
              closeSpinner.countDown();
            }
          });
    }

    oneMessageSent.acquire();
    connection.close();
    if (!requestSpinner.await(3, TimeUnit.SECONDS)) {
      Assert.fail("Requests time limit exceeded");
    }
    if (!closeSpinner.await(3, TimeUnit.SECONDS)) {
      Assert.fail("Close time limit exceeded");
    }

    ByteArrayOutputStream expected = new ByteArrayOutputStream();
    BserSerializer serializer = new BserSerializer();
    serializer.serializeToStream(request, expected);
    Assert.assertArrayEquals(expected.toByteArray(), this.mOutgoingMessageStream.toByteArray());

    int entry = 0;
    for (AtomicBoolean condition: conditions) {
      Assert.assertTrue(
          "Asserting condition for request #" + Integer.toString(entry),
          condition.get());
      entry++;
    }
  }

  @Test
  public void propagatesDeserializationException() throws Exception {
    final AtomicReference<BserDeserializer.BserEofException> expectedExceptionRef = new AtomicReference<>();
    final Semaphore commandSentSemaphore = new Semaphore(0);
    WatchmanConnection.WatchmanCommandListener commandListener =
        new WatchmanConnection.WatchmanCommandListener() {
          @Override public void onStart() { }
          @Override public void onSent() { commandSentSemaphore.release(); }
          @Override public void onReceived() { }
        };
    Callable<Map<String, Object>> incomingMessageGetter = new Callable<Map<String, Object>>() {
      @Override
      public Map<String, Object> call() throws Exception {
        commandSentSemaphore.acquire();
        BserDeserializer.BserEofException e = new BserDeserializer.BserEofException("expected");
        expectedExceptionRef.set(e);
        throw e;
      }
    };
    WatchmanConnection connection = new WatchmanConnection(
        incomingMessageGetter,
        mOutgoingMessageStream,
        Optional.<WatchmanConnection.WatchmanCommandListener>of(commandListener));
    connection.start();
    ListenableFuture<Map<String, Object>> listenableFuture = connection.run(Arrays.asList("clock", "/a/b/c"));
    try {
      Map<String, Object> receivedResponse = listenableFuture.get(5, TimeUnit.SECONDS);
      Assert.fail("Should not get a response, got " + receivedResponse.toString());
    } catch (ExecutionException executionException) {
      Assert.assertSame(expectedExceptionRef.get(), executionException.getCause());
    }
  }

  @Test
  public void propagatesInterruptionException() throws Exception {
    final Semaphore enteredCallable = new Semaphore(0);
    final Semaphore exitedCallable = new Semaphore(0);
    final AtomicReference<Thread> threadRef = new AtomicReference<>();
    final AtomicReference<InterruptedException> expectedException = new AtomicReference<>();
    Callable<Map<String, Object>> incomingMessageGetter = new Callable<Map<String, Object>>() {
      @Override
      public Map<String, Object> call() throws Exception {
        try {
          threadRef.set(Thread.currentThread());
          enteredCallable.release();
          new Semaphore(0).acquire(); // wait for interruption
          return new HashMap<>();
        } catch (InterruptedException e) {
          expectedException.set(e);
          throw e;
        } finally {
          exitedCallable.release();
        }
      }
    };
    WatchmanConnection connection = new WatchmanConnection(incomingMessageGetter, mOutgoingMessageStream);
    connection.start();
    ListenableFuture<Map<String, Object>> listenableFuture = connection.run(Arrays.asList("clock", "/a/b/c"));
    enteredCallable.acquire();
    threadRef.get().interrupt();
    Assert.assertTrue("Callable should have exited", exitedCallable.tryAcquire(5, TimeUnit.SECONDS));
    try {
      listenableFuture.get(5, TimeUnit.SECONDS);
      Assert.fail("Should not be able to get a result from an interrupted operation");
    } catch (InterruptedException e) {
      // This is reasonable behavior...
      Assert.assertSame("If InterruptedException, should be the one thrown by thread",
          expectedException.get(), e);
    } catch (ExecutionException e) {
      // ...and this is also reasonable behavior.
      Assert.assertSame("If ExecutionException, cause should be exception that interrupted thread",
          expectedException.get(), e.getCause());
    }
  }
}

