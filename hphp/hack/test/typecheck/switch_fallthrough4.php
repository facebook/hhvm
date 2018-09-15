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

function do_stuff(): void {}

function f(): void {
  switch (1) {
    case 1:
      break;
    case 2:
      do_stuff();
    default:
      break;
  }
}
