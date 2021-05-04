<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\IO;

/**
 * A non-disposable handle that is explicitly closeable.
 *
 * Some handles, such as those returned by `IO\server_error()` may
 * be neither disposable nor closeable.
 */
interface CloseableHandle extends Handle {
  /** Close the handle */
  public function close(): void;

  /** Close the handle when the returned disposable is disposed.
   *
   * Usage: `using $handle->closeWhenDisposed();`
   */
  <<__ReturnDisposable>>
  public function closeWhenDisposed(): \IDisposable;
}
