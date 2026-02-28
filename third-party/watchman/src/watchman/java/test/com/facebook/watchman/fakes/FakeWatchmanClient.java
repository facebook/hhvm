/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman.fakes;

import java.io.IOException;
import java.nio.file.Path;
import java.util.List;
import java.util.Map;

import com.facebook.watchman.Callback;
import com.facebook.watchman.WatchmanClient;

import com.google.common.util.concurrent.ListenableFuture;
import sun.reflect.generics.reflectiveObjects.NotImplementedException;

public class FakeWatchmanClient implements WatchmanClient {

  @Override
  public ListenableFuture<Map<String, Object>> clock(Path path) {
    throw new NotImplementedException();
  }

  @Override
  public ListenableFuture<Map<String, Object>> clock(Path path, Number syncTimeout) {
    throw new NotImplementedException();
  }

  @Override
  public ListenableFuture<Map<String, Object>> watch(Path path) {
    throw new NotImplementedException();
  }

  @Override
  public ListenableFuture<Map<String, Object>> watchDel(Path path) {
    throw new NotImplementedException();
  }

  @Override
  public ListenableFuture<Boolean> unsubscribe(SubscriptionDescriptor descriptor) {
    throw new NotImplementedException();
  }

  @Override
  public ListenableFuture<SubscriptionDescriptor> subscribe(
      Path path, Map<String, Object> query, Callback listener) {
    throw new NotImplementedException();
  }

  @Override
  public ListenableFuture<Map<String, Object>> version() {
    throw new NotImplementedException();
  }

  @Override
  public ListenableFuture<Map<String, Object>> version(
      List<String> optionalCapabilities, List<String> requiredCapabilities) {
    throw new NotImplementedException();
  }

  @Override
  public ListenableFuture<Map<String, Object>> run(List<Object> command) {
    throw new NotImplementedException();
  }

  @Override
  public ListenableFuture<Boolean> unsubscribeAll() {
    throw new NotImplementedException();
  }

  @Override
  public void close() throws IOException {
    throw new NotImplementedException();
  }

  @Override
  public void start() {
    throw new NotImplementedException();
  }
}
