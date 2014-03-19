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

type X = (function(): int);

function useX(X $x): int {
  do {
    return $x();
    if (true) {
    }
  } while (true);
  return 0;
}
