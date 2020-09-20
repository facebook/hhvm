<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function foo() {
  yield result(0);
  $x = function() {
    return 0;
  };
}

function result(int $x) {
  return $x;
}
