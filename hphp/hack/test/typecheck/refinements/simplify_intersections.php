<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public function f(): void {}
}

function f(vec<?A> $v): void {
  mymap(
    $v,
    $x ==> {
      if ($x is null) {
        return null;
      }
      $x->f();
      return $x;
    }
  );
}

function mymap<T1, T2>(
  Traversable<T1> $x,
  (function (T1): T2) $f,
) : Traversable<T2> {
  return vec[];
}
