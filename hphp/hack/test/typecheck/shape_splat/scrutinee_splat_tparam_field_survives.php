<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// The splat type parameter carries a concrete field ('t'). A preserving
// refinement (`is nonnull`) keeps T's contribution, so the T-derived field
// remains accessible with its declared type.
function f<T as shape('t' => bool, ...)>(shape(...T, 'a' => int) $x): void {
  if ($x is nonnull) {
    hh_expect<int>($x['a']);
    hh_expect<bool>($x['t']);
  }
}
