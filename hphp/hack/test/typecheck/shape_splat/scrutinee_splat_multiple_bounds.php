<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// The splat type parameter has *two* shape upper bounds, so `T` must have both
// 'a' and 'b'. All shape bounds are combined (intersected), so a closed hint
// that omits 'b' is impossible even though 'a' is present — the true branch is
// `nothing`. This must not depend on which bound happens to be enumerated first.
function f<T as shape('a' => int, ...) as shape('b' => string, ...)>(
  shape(...T, 'c' => bool) $x,
): void {
  if ($x is shape('a' => int, 'c' => bool)) {
    hh_expect_equivalent<nothing>($x);
  }
}
