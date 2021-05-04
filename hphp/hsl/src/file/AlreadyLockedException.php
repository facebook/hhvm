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

/**
 * Indicates that a lock failed, because the file is already locked.
 *
 * This class does not extend `OS\ErrnoException` as an `EWOULDBLOCK` after
 * `flock($fd, LOCK_NB)` is expected rather than an error; this exception is
 * thrown when the caller has explicitly requested an exception for these cases.
 */
final class AlreadyLockedException extends \Exception {
}
