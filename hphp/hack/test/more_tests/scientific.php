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

function f1(): float {
  return 1.234e-10+1.234e-10;
}

function f2(): float {
  return 1.234E-10-1.234e-10;
}

function f3(): float {
  return 1.23e45-1.234e-10;
}

function f4(): float {
  return +1.23e45-1.234e-10;
}

function f5(): float {
  return -1.23e45-1.234e-10;
}

function f6(): float {
  return +1.23+1.234e-10;
}

function f7(): float {
  return -1.23-1.234e-10;
}

function f8(): int {
  return +23-1;
}

function f9(): int {
  return -23-1;
}
