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

use namespace HH\Lib\OS;

enum LockType: int as int {
  /**
   * Any number of processes may have a shared lock simultaneously. It is
   * commonly called a reader lock. The creation of a Lock will block until
   * the lock is acquired.
   */
  SHARED = OS\LOCK_SH;

  /**
   * Only a single process may possess an exclusive lock to a given file at a
   * time. The creation of a Lock will block until the lock is acquired.
   */
  EXCLUSIVE = OS\LOCK_EX;
}
