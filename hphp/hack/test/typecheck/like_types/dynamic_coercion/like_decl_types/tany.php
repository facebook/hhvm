<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function f(~int $x) /* TAny */ {
  return $x; // ok
}
