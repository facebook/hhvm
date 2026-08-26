<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// A concrete shape cannot satisfy every possible instantiation of `T`.
function only_super<<<__Explicit>> T as shape('a' => int, ...)>(
  shape('a' => int) $s,
): shape(...T) {
  return $s;
}

// `T` may require additional fields that the argument does not provide.
function only_super_inline<<<__Explicit>> T as shape(...)>(
  shape('a' => int) $s,
): shape(...T, 'a' => int) {
  return $s;
}

// `T = shape('b' => bool)` demonstrates the missing required field.
function witness(): void {
  $r = only_super_inline<shape('b' => bool)>(shape('a' => 1));
  hh_expect<bool>($r['b']);
}
