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

function f(arraykey $k1, arraykey $k2): arraykey {
  return $k1;
}

function generic<T as arraykey>(T $k1, T $k2): T {
  return $k2;
}

function cast(arraykey $k1, arraykey $k2): (string, int) {
  return tuple((string)$k1, (int)$k2);
}

function f_opt(?arraykey $k1, arraykey $k2): arraykey {
  if (null === $k1) {
    return 0;
  }
  return f($k1, $k2);
}

abstract class C {}
function get_classname(): classname<C> {
  return C::class;
}

function test(): void {
  f(1, 1);
  f('a', 'a');
  f(1, 'a');
  f('a', 1);
  f(get_classname(), C::class);

  generic(1, 1);
  generic('a', 'a');
  generic(get_classname(), get_classname());

  f_opt(1, 1);
  f_opt('a', 'a');
  f_opt('a', 1);
  f_opt(1, 'a');
  f_opt(null, 1);
  f_opt(null, 'a');
}

function test_switch(arraykey $x): bool {
  $c = 1;
  if ($c == $x) {
    return true;
  }

  if (0 === $x) {
    return true;
  }

  if ('' === $x) {
    return true;
  }

  switch ($x) {
    case $c:
      return true;
    case 'a':
      $res = false;
      break;
    case 3:
      $res = true;
      break;
    default:
      $res = true;
  }
  return $res;
}
