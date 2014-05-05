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

function foo2(Traversable $p) {
  foreach ($p as $k => $v) {
    f1($k);
    f2($v);
  }
}

function f1(int $k) {}

function f2(string $v) {}


