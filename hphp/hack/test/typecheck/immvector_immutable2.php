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
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>
// Test that we can't mutate a ImmVector array style.

function bad(bool $b):void {
  $fv = ImmVector {1, 2, 3};
  if ($b) { $fv = Vector { 4 }; }
  $fv[0] = 2.3;
  // But check that the result makes sense, in case we silenced the error
  hh_expect_equivalent<(ImmVector<int> | Vector<num>)>($fv);
}
