<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

namespace NS1;

function f(): void {
  \NS2\f();
}

namespace NS2;

function f(): void {
  \NS1\f();
}
