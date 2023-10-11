<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

interface X {}

interface Y extends X {}

class U implements Y {}

function f(X $x): void {}

function main(): void {
  $u = new U();
  f($u);
}
