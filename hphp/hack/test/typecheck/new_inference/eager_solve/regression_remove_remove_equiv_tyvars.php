<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function vec_map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $async_func,
): vec<Tv2> {
  return vec[];
}

class C {
  public function foo(): void {}
}

function foo(Vector<C> $objects): void {
  $m = Map {};
  foreach ($objects as $obj) {
    $m[''] = Vector {};
    $m[''][] = $obj;
  }
  vec_map(
    $m->values(),
    $objs ==> {
      /* We used to have $objs: Vector<#1> with
      #2, C <: #1 <: #2
      #1, C <: #2 <: #1
      so the early solve of #1 would fail to solve to C.
      */
      $objs[0]->foo();
      return 0;
    }
  );
}
