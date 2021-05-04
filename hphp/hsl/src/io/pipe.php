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

use namespace HH\Lib\{_Private\_IO, OS};

/** Create a pair of handles, where writes to the `WriteHandle` can be
 * read from the `ReadHandle`.
 *
 * @see `Network\Socket`
 */
function pipe(): (CloseableReadFDHandle, CloseableWriteFDHandle) {
  list($r, $w) = \HH\Lib\OS\pipe();
  return tuple(
    new _IO\PipeReadHandle($r),
    new _IO\PipeWriteHandle($w),
  );
}


<<__Deprecated("use pipe() instead")>>
function pipe_nd(): (CloseableReadFDHandle, CloseableWriteFDHandle) {
  return pipe();
}
