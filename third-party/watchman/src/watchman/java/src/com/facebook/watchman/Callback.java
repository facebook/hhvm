/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman;

import java.util.Map;

public interface Callback {
  void call(Map<String, Object> message) throws Exception;
}
