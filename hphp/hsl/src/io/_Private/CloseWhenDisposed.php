<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\_Private\_IO;

use namespace HH\Lib\IO;

final class CloseWhenDisposed implements \IDisposable {
  public function __construct(
    private IO\CloseableHandle $handle,
  ) {
  }

  public function __dispose(): void {
    $this->handle->close();
  }
}
