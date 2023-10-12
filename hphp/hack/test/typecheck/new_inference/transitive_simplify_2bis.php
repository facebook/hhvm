<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Co<+T> {}
class Inv<T> {
  public function getCo(): Co<T> {
    return new Co();
  }
}

function f<T>(): (Co<T>, (function(Co<T>): void)) {
  return tuple(new Co(), $_ ==> {});
}

function g<T>(Co<T> $_): (Co<T>, Inv<T>) {
  return tuple(new Co(), new Inv());
}

function h<T>(Co<T> $x1, Co<T> $x2): Inv<T> {
  return new Inv();
}

function test(): void {
  $p = f(); // p : (Co<#1>, Inv<#1> => void)
  $x = $p[0]; // x: Co<#1>
  $f = $p[1]; // f: Inv<#1> => void
  $y1 = g($x)[0]; // y1: Co<#2>, #1 <: #+-2
  $y2 = g($x)[0]; // y2: Co<#3>, #1 <: #+-3
  $z = h($y1, $y2); // z: Inv<#4>, #2, #3 <: #4
  /*
  Basically, we have a diamond:
    4
   / \
  2  3
  \ /
   1
  1 <: 2, 3, 4
  1 <: 2 <: 4
  1 <: 3 <: 4
  1, 2, 3 <: 4
  And now we do 4 <: 1
  */
  $f($z->getCo()); // #4 < #1
}
