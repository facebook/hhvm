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

enum SocketDomain: int {
  PF_UNIX = _OS\PF_UNIX;
  PF_INET = _OS\PF_INET;
  PF_INET6 = _OS\PF_INET6;
}
