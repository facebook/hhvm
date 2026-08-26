<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Merging two open shapes satisfies another open shape, but cannot guarantee a
// required `k` field.

interface I {
  public function two<TA as shape(...), TB as shape(...)>(
    shape(...TA) $a,
    shape(...TB) $b,
  ): shape(...TA, ...TB);
}

function want_open<T as shape(...)>(shape(...T) $s): void {}

function want_k<T as shape(...)>(shape(...T, 'k' => int) $s): void {}

function test(I $i, shape(...) $x, shape(...) $y): void {
  want_open($i->two($x, $y)); // OK
  want_k($i->two($x, $y)); // ERROR: 'k' not guaranteed by the open args
}
