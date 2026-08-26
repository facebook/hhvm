<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// The `absent 'b'` bound lets `prefixed` infer the remaining row after `b` is
// removed from the shape produced by `grow`.
interface I {
  public function grow<T as shape(...)>(shape(...T) $s): shape(...T, 'a' => int);
  public function prefixed<T as shape(absent 'b', ...)>(
    shape(...T, 'b' => bool) $s,
  ): T;
}

function test(I $i, shape('b' => bool, 'c' => string) $s): void {
  $r = $i->grow($s);
  $tail = $i->prefixed($r);
  hh_expect<shape('a' => int, absent 'b', 'c' => string)>($tail);
}
