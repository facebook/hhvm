<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\_Private\_IO;

use namespace HH\Lib\{IO, OS};

final class StdioReadHandle
  extends FileDescriptorHandle
  implements IO\CloseableReadFDHandle {

  use FileDescriptorReadHandleTrait;
  public function __construct(OS\FileDescriptor $fd) {
    parent::__construct($fd);
  }
}
