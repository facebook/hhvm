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

// testing loops scoping

function stmt(): void {
  try {
    $x = 0;
    throw new Exception('this is a test');
  } catch (Exception $e) {
    $x = 1;
  }
  $x += 1;
}
