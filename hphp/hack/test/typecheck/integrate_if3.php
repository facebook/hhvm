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

function x(int $x): void {}

function y(): void {
  $x = 3;
  try {
    if (true) {
      $x = false;
      throw new Exception('x');
    } else {
      $x = 3;
      throw new Exception('x');
    }
  } catch (Exception $e) {
    x($x);
  }
}
