<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\_Private\_Unix;

use namespace HH\Lib\{IO, Network, OS, Unix};
use namespace HH\Lib\_Private\{_IO, _Network};

final class CloseableSocket
  extends _IO\FileDescriptorHandle
  implements Unix\CloseableSocket, IO\CloseableReadWriteHandle {
  use _IO\FileDescriptorReadHandleTrait;
  use _IO\FileDescriptorWriteHandleTrait;

  public function __construct(OS\FileDescriptor $impl) {
    parent::__construct($impl);
  }

  public function getLocalAddress(): ?string {
    return (OS\getsockname($this->impl) as OS\sockaddr_un)->getPath();
  }

  public function getPeerAddress(): ?string {
    return (OS\getpeername($this->impl) as OS\sockaddr_un)->getPath();
  }
}
