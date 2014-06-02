<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function generic_fn<Tk>(Vector<?Tk> $x): Vector<Tk> {
  return Vector {};
}

function mixed_fn(Vector<mixed> $x): Vector<mixed> {
  return generic_fn($x);
}


function mixed_fn2(Map<string, mixed> $x): Map<string, mixed> {
  $lol = Map {};
  $lol['foo'] = 0;
  $lol['bar'] = 12;
  return compact_thing($lol);
}

function compact_thing<Tk, Tv>(
  KeyedTraversable<Tk, ?Tv> $m,
): Map<Tk, Tv> {
  $result = Map {};
  foreach ($m as $k => $v) {
    if ($v !== null) {
      $result[$k] = $v;
    }
  }
  return $result;
}
