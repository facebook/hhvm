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

use namespace HH\Lib\C;
use namespace HH\Lib\_Private\_OS;


/**
 * Class for exceptions reported via the C `h_errno` variable.
 */
final class HErrnoException extends \Exception {
  public function __construct(private HErrno $errno, string $message) {
    parent::__construct($message);
  }

  final public function getHErrno(): HErrno{
    return $this->errno;
  }
}
