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

// Test that we can't mutate a ImmVector array style.

function bad() {
  $fv = ImmVector {1, 2, 3};
  $fv[0] = 42;
}



