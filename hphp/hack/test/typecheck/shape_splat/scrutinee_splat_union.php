<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters', 'union_intersection_type_hints')>>

// Union of splat scrutinees: refinement distributes through the union. The true
// branch guarantees the checked shape while retaining the surviving generic
// row constraint; the false branch stays a subtype of the original union.
function f<T1 as shape(...), T2 as shape(...)>(
  shape(...T1, 'a' => int) $a,
  shape(...T2, 'b' => string) $b,
  bool $c,
): void {
  $u = $c ? $a : $b;
  if ($u is shape('a' => int)) {
    hh_expect<shape('a' => int)>($u);
  } else {
    hh_expect<(shape(...T1, 'a' => int) | shape(...T2, 'b' => string))>($u);
  }
}
