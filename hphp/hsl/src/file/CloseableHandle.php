<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\File;

use namespace HH\Lib\IO;

interface CloseableHandle extends IO\CloseableSeekableHandle, Handle {
}

interface CloseableReadHandle
  extends IO\CloseableSeekableReadHandle, CloseableHandle, ReadHandle {
}

interface CloseableWriteHandle
  extends
    IO\CloseableSeekableWriteHandle,
    CloseableHandle,
    WriteHandle {
}

interface CloseableReadWriteHandle
  extends
    IO\CloseableSeekableReadWriteHandle,
    ReadWriteHandle,
    CloseableReadHandle,
    CloseableWriteHandle {
}
