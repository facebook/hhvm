<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\OS;

use namespace HH\Lib\_Private\_OS;

/** Supported operations for `fcntl()` */
enum FcntlOp: int {
  F_GETFD = _OS\F_GETFD;
  F_GETFL = _OS\F_GETFL;
  F_GETOWN = _OS\F_GETOWN;
  F_SETFD = _OS\F_SETFD;
  F_SETFL = _OS\F_SETFL;
  F_SETOWN = _OS\F_SETOWN;
}

const int FD_CLOEXEC = _OS\FD_CLOEXEC;

/** Control operations for file descriptors.
 *
 * See `man 2 fcntl` for details. On error, an `ErrnoException` will be thrown.
 */
function fcntl(FileDescriptor $fd, FcntlOp $cmd, ?int $arg = null): mixed {
  return _OS\wrap_impl(() ==> _OS\fcntl($fd, $cmd as int, $arg));
}
