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

/* HH_FIXME[4101] */
function f1(Map $m) {
  f1($m);
  f2($m);
  f($m);
}

function f2(Map<string, int> $m) {
  f1($m);
  f2($m);
  f($m);
}

function f(KeyedTraversable<string, int> $m) {
}
