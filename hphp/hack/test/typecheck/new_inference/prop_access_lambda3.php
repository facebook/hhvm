<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}

function foo(A $a): void {
  $f = $x ==> $x->x;
  $f($a);
}
