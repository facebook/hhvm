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

function getint(): ?int {
  return 1;
}

function f(): ?int {
  return ($x = getint()) ? $x : 0;
}

function g(): bool {
  if (true) {
    $y = ($x = getint()) ? 1 : 0;
  } else {
    $x = 0;
    $y = 0;
  }

  return $x && $y;
}
