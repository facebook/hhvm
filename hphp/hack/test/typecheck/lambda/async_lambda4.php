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

async function f(): Awaitable<void> {
  $f = async $v ==> $v;
  $g = $w ==> $w;
  $x = await $f(3);
  $y = $g(3);
  hh_force_solve();
  hh_show($x);
  hh_show($y);
}
