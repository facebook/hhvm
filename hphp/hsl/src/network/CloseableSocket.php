<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Network;

use namespace HH\Lib\{IO, TCP, Unix};

<<
  __Sealed(
    TCP\CloseableSocket::class,
    Unix\CloseableSocket::class,
  ),
>>
interface CloseableSocket extends Socket, IO\CloseableReadWriteHandle {
}
