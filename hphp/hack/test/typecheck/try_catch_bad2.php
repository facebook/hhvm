<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function foo(): int {
  $a = 23;
  try {
    $a = 'hello';
  } catch (Exception $e) {
    return $a;
  }
  return 123;
}
