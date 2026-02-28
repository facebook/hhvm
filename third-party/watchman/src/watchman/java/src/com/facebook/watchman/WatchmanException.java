/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman;

import java.util.Map;

public class WatchmanException extends Exception {

  private final Map<String, Object> response;

  public WatchmanException() {
    super();
    response = null;
  }

  public WatchmanException(String reason) {
    super(reason);
    response = null;
  }

  public WatchmanException(String error, Map<String, Object> response) {
    super(error);
    this.response = response;
  }

  public Map<String, Object> getResponse() {
    return response;
  }
}
