<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// The splat type parameter has two disjoint *closed* shape bounds, so their
// intersection is `nothing`: `T`, and hence the scrutinee `shape(...T, …)`, is
// uninhabited. The refinement preserves that bottom, so both branches are
// `nothing` (rather than an open over-approximation).
function f<T as shape('a' => int) as shape('b' => string)>(
  shape(...T, 'c' => bool) $x,
): void {
  if ($x is shape('c' => bool)) {
    hh_expect_equivalent<nothing>($x);
  } else {
    hh_expect_equivalent<nothing>($x);
  }
}
