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

function foo(bool $x): void {
  if ($x) {
    $y = "asd";
  }
  $z = $l ==> $y; // $y bad_style, even in a capture
}
