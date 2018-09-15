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

function foo(): void {}
function bar(): void {}

function hack_error(): void {
  switch (1) {
    case 1:
      // UNSAFE
      foo();
      // FALLTHROUGH
    case 2:
      bar();
      break;
  }
}
