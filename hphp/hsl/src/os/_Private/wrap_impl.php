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

use namespace HH\Lib\OS;

function wrap_impl<T>((function(): T) $impl): T {
  try {
    return $impl();
  } catch (namespace\ErrnoException $e) {
    throw_errno($e->getCode() as OS\Errno, '%s', $e->getMessage());
  }
}
