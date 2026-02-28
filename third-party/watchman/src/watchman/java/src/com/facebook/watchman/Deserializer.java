/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman;

import java.io.IOException;
import java.io.InputStream;
import java.util.Map;

public interface Deserializer {

  /**
   * Reads the next object from the InputSteram, blocking until it becomes available.
   * @param stream the stream to read from
   * @return a deserialized object, read from the stream
   */
  Map<String, Object> deserialize(InputStream stream) throws IOException;
}
