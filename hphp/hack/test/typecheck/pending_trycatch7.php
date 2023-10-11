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
function might_throw(): void {}

function f(): int {
  try {
    try {
      might_throw();
      $x = 1;
    } catch (Exception $e) {
    }
  } catch (Exception $e) {
    $x = 4;
  }

  return $x;
}
