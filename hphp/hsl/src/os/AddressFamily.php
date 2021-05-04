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

enum AddressFamily: int {
  AF_UNIX = _OS\AF_UNIX;
  AF_INET = _OS\AF_INET;
  AF_INET6 = _OS\AF_INET6;
}
