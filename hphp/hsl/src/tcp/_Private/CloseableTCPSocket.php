<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\_Private\_TCP;

use namespace HH\Lib\{IO, Network, OS, TCP};
use namespace HH\Lib\_Private\{_IO, _Network};

final class CloseableTCPSocket
  extends _IO\FileDescriptorHandle
  implements TCP\CloseableSocket, IO\CloseableReadWriteHandle {
  use _IO\FileDescriptorReadHandleTrait;
  use _IO\FileDescriptorWriteHandleTrait;

  public function __construct(OS\FileDescriptor $impl) {
    parent::__construct($impl);
  }

  public function getLocalAddress(): (string, int) {
    $sa = OS\getsockname($this->impl) as OS\sockaddr_in;
    return tuple(
      OS\inet_ntop_inet($sa->getAddress()),
      $sa->getPort(),
    );
  }

  public function getPeerAddress(): (string, int) {
    $sa = OS\getpeername($this->impl) as OS\sockaddr_in;
    return tuple(
      OS\inet_ntop_inet($sa->getAddress()),
      $sa->getPort(),
    );
  }
}
