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

function f1(): void {
  invariant(true, 'error\
    on a new line');
}

function f2(): void {
  invariant(true, 'this\'should parse');
}

function g(): int {
  return false;
}
