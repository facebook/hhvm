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

// Test that FVs cannot be indexed with a non-int key.

function badkey<T>(ImmVector<T> $fv) : void {
  $x = $fv['sadpanda'];
}
