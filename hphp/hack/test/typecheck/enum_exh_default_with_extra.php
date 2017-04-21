<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

enum E : int {
  A = 1;
  B = 2;
}

function TestIt(E $e, arraykey $k, bool $b): void {
  if ($b) {
    $e = $k;
  }
  switch ($e) {
    case E::A:
      echo 'A';
      break;
    case E::B:
      echo 'B';
      break;
    default:
      echo 'other';
      break;
  }
}
