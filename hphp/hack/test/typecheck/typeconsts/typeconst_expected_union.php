<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class SomeClass {
  abstract const type Tconst as arraykey;
}
class SomeClassChild extends SomeClass {
  const type Tconst = int;
}

function merge<T>(?T $x, T $y): T { return $x ?? $y; }

class Inv<T> { }

function f<T as SomeClass, Tc>(T $sc, Tc $c): (Inv<(T,Tc)>) where Tc = T::Tconst {
  throw new Exception("E");

}

  function test(SomeClass $sc, ?SomeClass $nullsc, nothing $n): void {
    f($sc, $n); // no error
    //hh_show_env();
    f<_,_>($nullsc ?? new SomeClassChild(), $n); // ERROR???
    //hh_show_env();
    $x = merge($nullsc, new SomeClassChild());
    $y = $nullsc ?? new SomeClassChild();
    f($sc, $n); // ERROR???
    $__tmp = $nullsc ?? new SomeClassChild();
    f($__tmp, $n); // no error???
}
