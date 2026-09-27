<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// The splat type parameter has a *closed* shape upper bound, so every value of
// `T` has exactly the field 'x'. The scrutinee therefore always has both 'x' and
// 'a', and a closed hint that omits 'x' can never match: the true branch is
// `nothing`. This exercises the bound-aware splat view (the closed bound both
// surfaces 'x' as a known field and keeps the row closed).
function f<T as shape('x' => int)>(shape(...T, 'a' => int) $x): void {
  if ($x is shape('a' => int)) {
    hh_expect_equivalent<nothing>($x);
  }
}
