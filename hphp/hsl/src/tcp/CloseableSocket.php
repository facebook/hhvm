<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\TCP;

use namespace HH\Lib\Network;
use namespace HH\Lib\_Private\_TCP;

<<__Sealed(_TCP\CloseableTCPSocket::class)>>
interface CloseableSocket
  extends Socket, Network\CloseableSocket {
}
