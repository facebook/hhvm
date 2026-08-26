<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Merging these concrete inputs should preserve both `k` and `j`, and the
// result must satisfy a parameter that requires `k`.

interface I {
  public function two<TA as shape(...), TB as shape(...)>(
    shape(...TA) $a,
    shape(...TB) $b,
  ): shape(...TA, ...TB);
}

function want_k<T as shape(...)>(shape(...T, 'k' => int) $s): void {}

function test(I $i, shape('k' => int) $x, shape('j' => bool) $y): void {
  $r = $i->two($x, $y);
  hh_expect<shape('k' => int, 'j' => bool)>($r);
  want_k($r);
}
