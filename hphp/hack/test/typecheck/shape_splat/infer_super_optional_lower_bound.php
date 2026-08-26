<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

function preserve_optional<T as shape(...)>(
  shape(...T) $s,
): shape(...T) {
  return $s;
}

function test(shape(?'id' => int) $s): void {
  $r = preserve_optional($s);
  hh_expect<shape(?'id' => int)>($r);
}
