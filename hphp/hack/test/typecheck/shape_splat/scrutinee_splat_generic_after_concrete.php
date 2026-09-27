<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

function f<T as shape(...)>(shape('a' => int, ...T) $x): void {
  if ($x is shape('a' => string, ...)) {
    hh_expect_equivalent<(
      shape('a' => int, ...T) & supportdyn<shape('a' => string, ...)>
    )>($x);
  }
}
