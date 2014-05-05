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

function f(): int {
  if (true) {
    $x = 1;
  } else if (false) {
    $x = 2;
  } else {
    $x = 4;
  }

  return $x;
}
