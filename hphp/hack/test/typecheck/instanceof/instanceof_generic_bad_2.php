<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {}

class A implements I {}
class B implements I {}

function does_is_check<T>(T $t, I $i): T {
  if($t is I) {
    return $i;
  }

  return $t;
}

function breakit(): A {
  $a = new A();
  $b = new B();
  return does_is_check<A>($a, $b);
}
