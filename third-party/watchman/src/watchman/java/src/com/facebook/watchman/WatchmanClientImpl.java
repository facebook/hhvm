/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman;

import com.google.common.util.concurrent.MoreExecutors;
import javax.annotation.Nullable;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

import com.google.common.annotations.VisibleForTesting;
import com.google.common.base.Function;
import com.google.common.base.Optional;
import com.google.common.base.Predicates;
import com.google.common.base.Supplier;
import com.google.common.base.Suppliers;
import com.google.common.collect.Collections2;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;

import static com.google.common.base.Preconditions.checkNotNull;

public class WatchmanClientImpl implements WatchmanClient {
  private static final String SUBSCRIPTION_KEY = "subscription";
  private static final Collection<String> UNILATERAL_LABELS = Arrays.asList(
      SUBSCRIPTION_KEY);

  private final WatchmanConnection connection;
  private final ConcurrentHashMap<SubscriptionDescriptor, Callback> subscriptions =
      new ConcurrentHashMap<SubscriptionDescriptor, Callback>();
  private final AtomicInteger subscriptionIndex = new AtomicInteger(0);

  private final Supplier<Boolean> supportsWatchProject;

  public WatchmanClientImpl(WatchmanTransport transport) throws IOException {
    connection = new WatchmanConnection(
        transport,
        Optional.of(UNILATERAL_LABELS),
        Optional.<Callback>of(new UnilateralCallbackImpl()));

    supportsWatchProject = Suppliers.memoize(new Supplier<Boolean>() {
      @Override
      public Boolean get() {
        return CapabilitiesStrategy.checkWatchProjectCapability(WatchmanClientImpl.this);
      }
    });
  }

  @VisibleForTesting
  WatchmanClientImpl(
      Callable<Map<String, Object>> incomingMessageGetter,
      OutputStream outgoingMessageStream,
      Supplier<Boolean> supportsWatchProject) {
    connection = new WatchmanConnection(
        incomingMessageGetter,
        outgoingMessageStream,
        Optional.of(UNILATERAL_LABELS),
        Optional.<Callback>of(new UnilateralCallbackImpl()));

    this.supportsWatchProject = supportsWatchProject;
  }

  @Override
  public ListenableFuture<Map<String, Object>> clock(Path path) {
    List<String> request = ImmutableList.of(
        "clock",
        path.toAbsolutePath().toString());
    return connection.run(request);
  }

  @Override
  public ListenableFuture<Map<String, Object>> clock(Path path, Number syncTimeout) {
    List<Object> request = ImmutableList.<Object>of(
        "clock",
        path.toAbsolutePath().toString(),
        ImmutableMap.<String, Object>of("sync_timeout", syncTimeout));

    return connection.run(request);
  }

  @Override
  public ListenableFuture<Map<String, Object>> watch(Path path) {
    if (!supportsWatchProject.get()) {
      return Futures.immediateFailedFuture(
          new WatchmanException("Please upgrade Watchman to the latest version in order to use " +
              "the watching functionality"));
    }

    List<String> request = ImmutableList.of(
        "watch-project",
        path.toAbsolutePath().toString()
    );
    return connection.run(request);
  }

  @Override
  public ListenableFuture<Map<String, Object>> watchDel(Path path) {
    List<String> request = ImmutableList.of(
        "watch-del",
        path.toAbsolutePath().toString());
    return connection.run(request);
  }

  @Override
  public ListenableFuture<Boolean> unsubscribe(final SubscriptionDescriptor descriptor) {
    if (! subscriptions.containsKey(descriptor)) {
      return Futures.immediateFuture(false);
    }

    List<String> request = ImmutableList.of(
        "unsubscribe",
        descriptor.root(),
        descriptor.name());

    return Futures.transform(connection.run(request), new Function<Map<String, Object>, Boolean>() {
      @Nullable
      @Override
      public Boolean apply(@Nullable Map<String, Object> input) {
        checkNotNull(input);

        boolean wasDeleted = (Boolean) input.get("deleted");
        if (wasDeleted) {
          if (subscriptions.remove(descriptor) == null) {
            return false;
          }
        }
        return wasDeleted;
      }
    }, MoreExecutors.directExecutor());
  }

  @Override
  public ListenableFuture<SubscriptionDescriptor> subscribe(
      Path path,
      Map<String, Object> query,
      final Callback listener) {
    final String subscriptionId = "sub-" + subscriptionIndex.getAndAdd(1);
    final String root = path.toAbsolutePath().toString();

    final SubscriptionDescriptor result = new SubscriptionDescriptorBuilder()
        .name(subscriptionId)
        .root(root)
        .build();
    subscriptions.put(result, listener);

    List<Object> request = ImmutableList.of(
        "subscribe",
        root,
        subscriptionId,
        query == null ? Collections.emptyMap() : query);

    return Futures.transform(
        connection.run(request),
        new Function<Map<String, Object>, SubscriptionDescriptor>() {
          @Nullable
          @Override
          public SubscriptionDescriptor apply(@Nullable Map<String, Object> input) {
            // TODO remove subscription descriptor from `subscriptions` if we got an error from wman
            return result;
          }
        }, MoreExecutors.directExecutor());
  }

  @Override
  public ListenableFuture<Map<String, Object>> version() {
    List<String> request = ImmutableList.of("version");
    return connection.run(request);
  }

  @Override
  public ListenableFuture<Map<String, Object>> version(
      List<String> optionalCapabilities,
      List<String> requiredCapabilities) {
    Map<String, Object> capabilities = ImmutableMap.<String, Object>of(
        "optional", optionalCapabilities,
        "required", requiredCapabilities);
    List<Object> request = ImmutableList.of("version", capabilities);
    return connection.run(request);
  }

  @Override
  public ListenableFuture<Map<String, Object>> run(List<Object> command) {
    return connection.run(command);
  }

  /**
   * unsubscribes from all the subscriptions; convenience method
   */
  @Override
  public ListenableFuture<Boolean> unsubscribeAll() {
    Collection<ListenableFuture<Boolean>> unsubscribeAll = Collections2.transform(
        subscriptions.keySet(),
        new Function<SubscriptionDescriptor, ListenableFuture<Boolean>>() {
          @Nullable
          @Override
          public ListenableFuture<Boolean> apply(@Nullable SubscriptionDescriptor input) {
            return unsubscribe(input);
          }
        });
    return Futures.transform(
        Futures.allAsList(unsubscribeAll),
        new Function<List<Boolean>, Boolean>() {
          @Nullable
          @Override
          public Boolean apply(@Nullable List<Boolean> input) {
            return !Collections2.filter(input, Predicates.equalTo(false)).isEmpty();
          }
        }, MoreExecutors.directExecutor());
  }

  /**
   * closes the WatchmanConnection
   */
  @Override
  public void close() throws IOException {
    connection.close();
  }

  @Override
  public void start() {
    connection.start();
  }

  private class UnilateralCallbackImpl implements Callback {

    @Override
    public void call(Map<String, Object> message) throws Exception {
      if (message.containsKey(SUBSCRIPTION_KEY)) {
        String subscriptionId = (String) message.get("subscription");
        String root = (String) message.get("root");
        SubscriptionDescriptor subscription = new SubscriptionDescriptorBuilder()
            .name(subscriptionId)
            .root(root)
            .build();

        Callback listener = subscriptions.get(subscription);
        if (listener == null) {
          // TODO log error?!
          return;
        }
        listener.call(message);
      }
    }
  }

}
