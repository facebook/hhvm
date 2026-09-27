<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'union_intersection_type_hints',
)>>

function f(shape('a' => int, ...dynamic) $x): void {
  if ($x is shape('a' => string, ...)) {
    hh_expect_equivalent<(
      shape('a' => int, ...dynamic) & shape('a' => string, ...)
    )>($x);
  }
}
