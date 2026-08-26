<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// The optional `x` suffix satisfies the input's `x` field, so `T` is inferred
// as `shape(absent 'x', ?'y' => int)`.

interface I {
  public function f<T as shape(...)>(shape(...T, ?'x' => int) $s): T;
}

function test(I $i, shape('x' => int, ?'y' => int) $s): void {
  $x = $i->f($s);
  hh_expect_equivalent<shape(absent 'x', ?'y' => int)>($x);
}
