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

function show<T>(T $x): void {}

function test(): Vector<(function(int): int)> {
  $v = Vector {
    function($x) {
      return $x;
    },
  };
  $v[] = function($x) use ($v) {
    return $v[0]($x);
  };
  return $v;
}
