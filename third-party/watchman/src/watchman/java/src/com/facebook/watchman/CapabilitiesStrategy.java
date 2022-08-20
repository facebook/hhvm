/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman;

import java.util.Collections;
import java.util.Map;
import java.util.concurrent.ExecutionException;

import com.google.common.util.concurrent.ListenableFuture;

/**
 * Called a "strategy" because we might have different ways of testing for capabilities as versions
 * change. Should become an interface once we get different implementations available.
 */
public class CapabilitiesStrategy {

  private final static String CMD_WATCH_PROJECT = "cmd-watch-project";
  private final static String CAPABILITIES = "capabilities";

  /**
   * Tests if a client supports the "watch-project" command or not.
   */
  public static boolean checkWatchProjectCapability(WatchmanClient client) {

    ListenableFuture<Map<String, Object>> future = client.version(
        Collections.<String>emptyList(),
        Collections.singletonList(CMD_WATCH_PROJECT));
    try {
      Map<String, Object> response = future.get();
      if (response.containsKey(CAPABILITIES)) {
        Map<String, Object> capabilities = (Map<String, Object>) response.get(CAPABILITIES);
        return Boolean.TRUE.equals(capabilities.get(CMD_WATCH_PROJECT));
      }
      return false;
    } catch (InterruptedException | ExecutionException e) {
      return false;
    }
  }
}
