/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman;

public class WatchmanTransportUnavailableException extends Exception {

  public WatchmanTransportUnavailableException() {
    super();
  }

  public WatchmanTransportUnavailableException(String message) {
    super(message);
  }

  public WatchmanTransportUnavailableException(String message, Throwable cause) {
    super(message, cause);
  }
}
