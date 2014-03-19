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


interface X {
}

interface Y extends X {
}

class U implements Y {
}

function f(X $x): void {
}

function main(): void {
  $u = new U();
  f($u);
}
