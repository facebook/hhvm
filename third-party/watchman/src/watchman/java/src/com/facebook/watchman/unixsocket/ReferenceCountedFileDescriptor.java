/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

 /*

 Copyright 2004-2015, Martian Software, Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 */

package com.facebook.watchman.unixsocket;

import com.sun.jna.LastErrorException;

import java.io.IOException;

/**
 * Encapsulates a file descriptor plus a reference count to ensure close requests
 * only close the file descriptor once the last reference to the file descriptor
 * is released.
 * <p>
 * If not explicitly closed, the file descriptor will be closed when
 * this object is finalized.
 */
class ReferenceCountedFileDescriptor {
  private int fd;
  private int fdRefCount;
  private boolean closePending;

  public ReferenceCountedFileDescriptor(int fd) {
    this.fd = fd;
    this.fdRefCount = 0;
    this.closePending = false;
  }

  @Override
  protected void finalize() throws IOException {
    close();
  }

  public synchronized int acquire() {
    fdRefCount++;
    return fd;
  }

  public synchronized void release() throws IOException {
    fdRefCount--;
    if (fdRefCount == 0 && closePending && fd != -1) {
      doClose();
    }
  }

  public synchronized void close() throws IOException {
    if (fd == -1 || closePending) {
      return;
    }

    if (fdRefCount == 0) {
      doClose();
    } else {
      // Another thread has the FD. We'll close it when they release the reference.
      closePending = true;
    }
  }

  private void doClose() throws IOException {
    try {
      UnixDomainSocketLibrary.close(fd);
      fd = -1;
    } catch (LastErrorException e) {
      throw new IOException(e);
    }
  }
}
