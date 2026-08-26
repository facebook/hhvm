<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Passing `f`'s result to `g` should infer
// `T2 = shape('a' => bool, ?'y' => int)`. The optional `x` field belongs to
// `g`'s parameter type and is not included in `T2`.

interface I {
  public function f<T1 as shape(...)>(shape(...T1) $s): shape(...T1);
  public function g<T2 as shape(...)>(shape(...T2, ?'x' => int) $s): T2;
}

function test(I $i, shape('a' => bool, ?'y' => int) $s): void {
  $r = $i->f($s);
  hh_expect<shape('a' => bool, ?'y' => int)>($i->g($r));
}
