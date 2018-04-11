<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function main(): void {
  $prng = null;
  if ($prng != null) {
    $rand = $prng->next(0, 1 << 31) / (1 << 31);
  }
}
