<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\_Private\_OS;

use namespace HH\Lib\{C, OS};

trait ErrnoExceptionWithMultipleErrnosTrait {
  require extends OS\ErrnoException;

  final public function __construct(OS\Errno $code, string $message) {
    invariant(
      C\contains(static::_getValidErrnos(), $code),
      'Exception %s constructed with invalid code %s',
      static::class,
      $code,
    );
    parent::__construct($code, $message);
  }

  abstract public static function _getValidErrnos(): keyset<OS\Errno>;
}
