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

// Test that we can't mutate a ImmVector array style.

function bad() {
  $fv = ImmVector {1, 2, 3};
  $fv[0] = 42;
}



