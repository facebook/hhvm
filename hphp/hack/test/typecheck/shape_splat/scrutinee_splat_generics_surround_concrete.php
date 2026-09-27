<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

function f<T1 as shape(...), T2 as shape(...)>(
  shape(...T1, 'a' => int, ...T2) $x,
  shape(...T1) $_t1,
  shape(...T2) $_t2,
): void {
  if ($x is shape('a' => string, ...)) {
    hh_expect_equivalent<(
      shape(...T1, 'a' => int, ...T2) &
      supportdyn<shape('a' => string, ...)>
    )>($x);
  }
}
