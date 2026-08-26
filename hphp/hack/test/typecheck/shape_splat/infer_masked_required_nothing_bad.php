<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Bad variant: `T`'s bound is open and does not prove `x` absent, so a caller
// can instantiate `T` with a shape that contains `x` (e.g. via an explicit type
// argument). Removing `x` inside would leave the returned `shape(...T)` claiming
// an `x` that is gone at runtime, so `removeKey` is correctly rejected.
// See infer_masked_required_nothing.php for the sound variant.
function producer<T as shape(...)>(shape(...T, 'x' => int) $s): shape(...T) {
  Shapes::removeKey(inout $s, 'x');
  return $s;
}
