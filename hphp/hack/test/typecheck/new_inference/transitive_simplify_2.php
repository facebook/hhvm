<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Co<+T> {}
class Inv<T> {}

function f<T>(): (Co<T>, (function(Inv<T>): void)) {
  return tuple(new Co(), $_ ==> {});
}

function g<T>(Co<T> $_): (Co<T>, Inv<T>) {
  return tuple(new Co(), new Inv());
}

function h<T>(Co<T> $x1, Co<T> $x2): Inv<T> {
  return new Inv();
}

function test(): void {
  $p = f();
  $x = $p[0];
  $f = $p[1];
  $y1 = g($x)[0];
  $y2 = g($x)[0];
  $z = h($y1, $y2);
  $f($z);
}
