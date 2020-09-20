<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function my_map<T1, T2>(
  Traversable<T1> $v,
  (function (T1): T2) $f,
): Traversable<T2> {
  return vec[];
}

function test($v): void {
  my_map(
    $v,
    $x ==> {
      $x->foo();
    }
  );
}
