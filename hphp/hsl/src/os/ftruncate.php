<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\OS;

use namespace HH\Lib\_Private\_OS;

function ftruncate(FileDescriptor $fd, int $length): void {
  _OS\arg_assert($length >= 0, '$length must be >= 0, got %d', $length);
  _OS\wrap_impl(() ==> _OS\ftruncate($fd, $length));
}
