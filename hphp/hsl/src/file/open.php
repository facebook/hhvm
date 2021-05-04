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
use namespace HH\Lib\{OS, _Private\_File};

function open_read_only(string $path): CloseableReadHandle {
  return OS\open($path, OS\O_RDONLY)
    |> new _File\CloseableReadHandle($$, $path);
}

function open_write_only(
  string $path,
  WriteMode $mode = WriteMode::OPEN_OR_CREATE,
  int $create_file_permissions = 0644,
): CloseableWriteHandle {
  return OS\open(
    $path,
    OS\O_WRONLY | $mode as int,
    ($mode & OS\O_CREAT) ? $create_file_permissions : null,
  )
    |> new _File\CloseableWriteHandle($$, $path);
}

function open_read_write(
  string $path,
  WriteMode $mode = WriteMode::OPEN_OR_CREATE,
  int $create_file_permissions = 0644,
): CloseableReadWriteHandle {
  return OS\open(
    $path,
    OS\O_RDWR | $mode as int,
    ($mode & OS\O_CREAT) ? $create_file_permissions : null,
  )
    |> new _File\CloseableReadWriteHandle($$, $path);
}

<<__Deprecated("Use open_read_only() instead")>>
function open_read_only_nd(string $path): CloseableReadHandle {
  return open_read_only($path);
}

<<__Deprecated("Use open_write_only() instead")>>
function open_write_only_nd(
  string $path,
  WriteMode $mode = WriteMode::OPEN_OR_CREATE,
  int $create_file_permissions = 0644,
): CloseableWriteHandle {
  return open_write_only($path, $mode, $create_file_permissions);
}

<<__ReturnDisposable, __Deprecated("Use open_read_write() instead")>>
function open_read_write_nd(
  string $path,
  WriteMode $mode = WriteMode::OPEN_OR_CREATE,
  int $create_file_permissions = 0644,
): CloseableReadWriteHandle {
  return open_read_write($path, $mode, $create_file_permissions);
}
