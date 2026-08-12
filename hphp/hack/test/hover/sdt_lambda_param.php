<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

// T247822323: under implicit pessimisation, a lambda passed to a generic mapper
// over a pessimised `vec<int>` gets parameter type `int` (the like is stripped at
// the call boundary), but the closure is a supportdyn closure, so under dynamic
// assumptions the parameter is `dynamic`. Because the enclosing `map_a_vec` has a
// (non-enforceable) `vec<int>` parameter, its dynamic-pass TAST is retained, so
// hover surfaces both as `int (dynamic when called dynamically)`.
function my_map<Tv1, Tv2>(
  Traversable<Tv1> $t,
  (function(Tv1): Tv2) $f,
): vec<Tv2> {
  return vec[];
}

function map_a_vec(vec<int> $v): void {
  my_map($v, $val ==> {
    return $val;
    //     ^ hover-at-caret
  });
}
