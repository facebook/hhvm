/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman;

import java.io.Closeable;
import java.io.InputStream;
import java.io.OutputStream;

public interface WatchmanTransport extends Closeable {
  InputStream getInputStream();
  OutputStream getOutputStream();
}
