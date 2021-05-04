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

enum SocketType: int {
  SOCK_STREAM = _OS\SOCK_STREAM;
  SOCK_DGRAM = _OS\SOCK_DGRAM;
  SOCK_RAW = _OS\SOCK_RAW;
}
