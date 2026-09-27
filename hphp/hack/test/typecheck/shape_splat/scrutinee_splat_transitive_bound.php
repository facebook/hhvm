<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// The splat type parameter `T`'s upper bound is itself a splat over another type
// parameter `TU`, with a required field 'p'. Bounds are resolved transitively, so
// `T` (and the scrutinee) is known to always have 'p'. A closed hint that omits
// 'p' is therefore impossible: the true branch is `nothing`.
function f<TU as shape(...), T as shape(...TU, 'p' => int)>(
  shape(...T, 'a' => int) $x,
): void {
  if ($x is shape('a' => int)) {
    hh_expect_equivalent<nothing>($x);
  }
}
