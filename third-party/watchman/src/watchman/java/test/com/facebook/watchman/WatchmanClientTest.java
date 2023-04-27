/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

import com.facebook.watchman.bser.BserDeserializer;

import com.google.common.base.Supplier;
import com.google.common.collect.ImmutableMap;
import org.hamcrest.Matchers;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.mockito.Mockito;

public class WatchmanClientTest extends WatchmanTestBase {

  private WatchmanClient mClient;
  private Boolean withWatchProject;

  @Rule
  public ExpectedException thrown = ExpectedException.none();

  /* @Before methods in subclasses are run AFTER @Before methods of the base class */
  @Before
  public void setClient() throws IOException {
    withWatchProject = true;
    mClient = new WatchmanClientImpl(
        mIncomingMessageGetter,
        mOutgoingMessageStream,
        new Supplier<Boolean>() {
          @Override
          public Boolean get() {
            return withWatchProject;
          }
        });
    mClient.start();
  }

  @SuppressWarnings("unchecked")
  @Test
  public void subscribeTriggersListenerTest() throws InterruptedException {
    Map<String, Object> subscriptionReply = new HashMap<>();
    subscriptionReply.put("version", "1.2.3");
    // "subscribe" value should be private to WatchmanClient so we could use any mock string
    subscriptionReply.put("subscribe", "sub-0");
    mObjectQueue.put(subscriptionReply);

    Map<String, Object> subscribeEvent = new HashMap<>();
    subscribeEvent.put("version", "1.2.3");
    subscribeEvent.put("clock", "c:123:1234");
    subscribeEvent.put("files", Arrays.asList("/foo/bar", "/foo/baz"));
    subscribeEvent.put("root", "/foo");
    subscribeEvent.put("subscription", "sub-0");
    mObjectQueue.put(subscribeEvent);

    final CountDownLatch latch = new CountDownLatch(1);
    final AtomicReference<Map<String, Object>> result = new AtomicReference<>();
    mClient.subscribe(Paths.get("/foo"), null, new Callback() {
      @Override
      public void call(Map<String, Object> event) {
        result.set(event);
        latch.countDown();
      }
    });

    if (! latch.await(10, TimeUnit.SECONDS)) {
      Assert.fail();
    }

    deepObjectEquals(subscribeEvent, result.get());
  }

  /**
   * Test the case when we get a unilateral message from Watchman (a subscription update event)
   * before the answer to the command we have just sent. We expect that the response to the watch
   * request is delivered, and not the subscription update event.
   */
  @SuppressWarnings("unchecked")
  @Test
  public void watchProjectWithUnilateralTest() throws ExecutionException, InterruptedException {
    Map<String, Object> dummyUnilateralMessage = new HashMap<>();
    dummyUnilateralMessage.put("version", "1.2.3");
    dummyUnilateralMessage.put("clock", "c:123:1234");
    dummyUnilateralMessage.put("files", Arrays.asList("/foo/bar", "/foo/baz"));
    dummyUnilateralMessage.put("root", "/foo");
    dummyUnilateralMessage.put("subscription", "sub-0");
    mObjectQueue.put(dummyUnilateralMessage);

    Map<String, Object> mockResponse = new HashMap<>();
    mockResponse.put("version", "1.2.3");
    mockResponse.put("watch", "/foo/bar");
    mockResponse.put("relative_path", "/foo");
    mObjectQueue.put(mockResponse);

    Map<String, Object> receivedResponse = mClient.watch(Paths.get("/foo/bar")).get();
    deepObjectEquals(mockResponse, receivedResponse);
  }

  /**
   * Test that the watch-project request sent by WatchmanClient respects the interface of Watchman
   */
  @SuppressWarnings("unchecked")
  @Test
  public void watchProjectRequestTest() throws IOException, ExecutionException, InterruptedException {
    String PATH = "/foo/bar";

    mObjectQueue.put(new HashMap<String, Object>()); // response irrelevant
    mClient.watch(Paths.get(PATH)).get();

    ByteArrayInputStream in = new ByteArrayInputStream(mOutgoingMessageStream.toByteArray());
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    List<Object> request = (List<Object>) deserializer.deserializeBserValue(in);

    //noinspection RedundantCast
    deepObjectEquals(
        Arrays.<Object>asList("watch-project", PATH),
        request);
  }

  /**
   * Test that the watch-del request sent by WatchmanClient respects the interface of Watchman
   */
  @SuppressWarnings("unchecked")
  @Test
  public void watchDelRequestTest() throws IOException, ExecutionException, InterruptedException {
    String PATH = "/foo/bar";

    mObjectQueue.put(new HashMap<String, Object>()); // response irrelevant
    mClient.watchDel(Paths.get(PATH)).get();

    ByteArrayInputStream in = new ByteArrayInputStream(mOutgoingMessageStream.toByteArray());
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    List<Object> request = (List<Object>) deserializer.deserializeBserValue(in);

    //noinspection RedundantCast
    deepObjectEquals(
        Arrays.<Object>asList("watch-del", PATH),
        request);
  }

  /**
   * Test that requesting a watch when watch-project is unavailable throws a WatchmanException whose
   * message mentions upgrading Watchman.
   */
  @SuppressWarnings("unchecked")
  @Test
  public void watchRequestTest() throws IOException, ExecutionException, InterruptedException {
    String PATH = "/foo/bar";
    thrown.expect(ExecutionException.class);
    thrown.expectCause(Matchers.allOf(
        Matchers.isA(WatchmanException.class),
        Matchers.hasToString(
            Matchers.containsString("upgrade"))));

    withWatchProject = false;
    mClient.watch(Paths.get(PATH)).get(); // throws
  }

  /**
   * Test that the subscribe request sent by WatchmanClient respects the interface of Watchman
   */
  @SuppressWarnings("unchecked")
  @Test
  public void subscribeRequestTest() throws ExecutionException, InterruptedException, IOException {
    final String PATH = "/foo/bar";
    final String NAME = "sub-0";
    Callback mockListener = Mockito.mock(Callback.class);

    Map<String, Object> response = new HashMap<>();
    response.put("subscribe", "name");
    mObjectQueue.put(response); // response irrelevant
    mClient.subscribe(
        Paths.get(PATH),
        new HashMap<String, Object>(),
        mockListener).get();

    ByteArrayInputStream in = new ByteArrayInputStream(mOutgoingMessageStream.toByteArray());
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    List<Object> request = (List<Object>) deserializer.deserializeBserValue(in);
    deepObjectEquals(
        Arrays.<Object>asList("subscribe", PATH, NAME, new HashMap<String, Object>()),
        request);
  }

  /**
   * Test that the unsubscribe request sent by WatchmanClient respects the interface of Watchman
   */
  @SuppressWarnings("unchecked")
  @Test
  public void unsubscribeRequestTest()
      throws ExecutionException, InterruptedException, IOException {
    final String PATH = "/foo/bar";
    final String NAME = "sub-0";

    Map<String, Object> response = new HashMap<>();
    response.put("deleted", true);
    mObjectQueue.put(response); // response irrelevant
    mObjectQueue.put(response); // response irrelevant

    Callback mockListener = Mockito.mock(Callback.class);
    WatchmanClient.SubscriptionDescriptor descriptor = mClient.subscribe(
        Paths.get(PATH),
        new HashMap<String, Object>(),
        mockListener).get();
    mOutgoingMessageStream.reset(); // ignore the subscribe command
    mClient.unsubscribe(descriptor).get();

    ByteArrayInputStream in = new ByteArrayInputStream(mOutgoingMessageStream.toByteArray());
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    List<Object> request = (List<Object>) deserializer.deserializeBserValue(in);
    deepObjectEquals(
        Arrays.<Object>asList("unsubscribe", PATH, NAME),
        request);
  }

  /**
   * Test that the clock request sent by WatchmanClient respects the interface of Watchman
   */
  @SuppressWarnings("unchecked")
  @Test
  public void clockRequestWithoutTimeoutTest() throws ExecutionException, InterruptedException, IOException {
    final String PATH = "/foo/bar";

    Map<String, Object> response = new HashMap<>();
    response.put("clock", "some value");
    mObjectQueue.put(response); // response irrelevant

    mClient.clock(Paths.get(PATH)).get();

    ByteArrayInputStream in = new ByteArrayInputStream(mOutgoingMessageStream.toByteArray());
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    List<Object> request = (List<Object>) deserializer.deserializeBserValue(in);
    deepObjectEquals(
        Arrays.<Object>asList("clock", PATH),
        request);
  }

  /**
   * Test that the clock request sent by WatchmanClient respects the interface of Watchman, when
   * sync_timeout is also required
   */
  @SuppressWarnings("unchecked")
  @Test
  public void clockRequestWithTimeoutTest() throws ExecutionException, InterruptedException, IOException {
    final String PATH = "/foo/bar";
    final Short SYNC_TIMEOUT = 1500;

    Map<String, Object> response = new HashMap<>();
    response.put("clock", "some value");
    mObjectQueue.put(response); // response irrelevant

    mClient.clock(Paths.get(PATH), SYNC_TIMEOUT).get();

    ByteArrayInputStream in = new ByteArrayInputStream(mOutgoingMessageStream.toByteArray());
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    List<Object> request = (List<Object>) deserializer.deserializeBserValue(in);
    deepObjectEquals(
        Arrays.<Object>asList(
            "clock",
            PATH,
            ImmutableMap.<String, Object>of("sync_timeout", SYNC_TIMEOUT)),
        request);
  }


  /**
   * Test that the version request sent by WatchmanClient respects the interface of Watchman
   */
  @SuppressWarnings("unchecked")
  @Test
  public void versionRequestTestNoCapabilities()
      throws ExecutionException, InterruptedException, IOException {
    Map<String, Object> response = new HashMap<>();
    response.put("version", "1.2.3");
    mObjectQueue.put(response); // response irrelevant

    mClient.version().get();

    ByteArrayInputStream in = new ByteArrayInputStream(mOutgoingMessageStream.toByteArray());
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    List<Object> request = (List<Object>) deserializer.deserializeBserValue(in);
    deepObjectEquals(
        Arrays.<Object>asList("version"),
        request);
  }

  /**
   * Test that the version request sent by WatchmanClient respects the interface of Watchman
   */
  @SuppressWarnings("unchecked")
  @Test
  public void versionRequestTestWithCapabilities()
      throws ExecutionException, InterruptedException, IOException {
    Map<String, Object> response = new HashMap<>();
    response.put("version", "1.2.3");
    mObjectQueue.put(response); // response irrelevant

    List<String> optionalCapabilities = Collections.singletonList("optional1");
    List<String> requiredCapabilities = Arrays.asList("required1", "required2");
    mClient.version(optionalCapabilities, requiredCapabilities).get();

    Map<String, Object> expectedCapabilitiesMap = new HashMap<>();
    expectedCapabilitiesMap.put("optional", optionalCapabilities);
    expectedCapabilitiesMap.put("required", requiredCapabilities);

    ByteArrayInputStream in = new ByteArrayInputStream(mOutgoingMessageStream.toByteArray());
    BserDeserializer deserializer = new BserDeserializer(BserDeserializer.KeyOrdering.UNSORTED);
    List<Object> request = (List<Object>) deserializer.deserializeBserValue(in);
    deepObjectEquals(
        Arrays.<Object>asList("version", expectedCapabilitiesMap),
        request);
  }
}
