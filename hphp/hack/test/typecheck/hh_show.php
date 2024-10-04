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

function foo(int $x): void {
  hh_show($x);
  $v = Vector{};
  hh_show($v);
  hh_log_level('show', 1);
  hh_show($v);
}
