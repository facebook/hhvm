<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function dup<T>(T $x): (T, T) {
  return tuple($x, $x);
}

function expect(int $_, int $_): void {}

function test1(vec<int> $v): void {
  list($w, $_) = dup($v);
  $w[] = 42;
}

function test2((function(): void) $f): void {
  list($g, $_) = dup($f);
  $g();
}
