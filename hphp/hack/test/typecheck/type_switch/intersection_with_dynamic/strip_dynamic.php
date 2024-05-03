<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

/**
 * Here we want to make sure a bare dynamic is intersected w/
 */
function foo(dynamic $x): int {
  if ($x is int) {
    hh_expect_equivalent<(int & dynamic)>($x);
    return $x;
  }
  throw new Exception();
}

/**
 * Here we want to see that the dynamic as part of a union (like type)
 * is treated a bit differently; namely that it is extracted out before the
 * type switching is broken into bounds on the true and false types
 * and then added back.
 * This is because some places in the typechecker currently will try to strip
 * out the dynamic before applying a transform and the stripping will trip up
 * on forms like:
 *   (dynamic & nonnull) | vec<int>
 * vs:
 *   dynamic | vec<int>
 */
function bar(~?vec<int> $dyn_opt_vec): ~vec<int> {
  if ($dyn_opt_vec is null) {
    throw new Exception();
  }
  hh_expect_equivalent<~vec<int>>($dyn_opt_vec);
  return $dyn_opt_vec;
}
