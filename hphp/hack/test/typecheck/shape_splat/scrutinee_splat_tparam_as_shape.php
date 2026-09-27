<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// `as` a concrete shape guarantees the checked closed shape. The narrowed type
// also retains the original generic row constraint.
function f<T as shape(...)>(shape(...T, 'a' => int) $x): void {
  $y = $x as shape('a' => int);
  hh_expect<shape('a' => int)>($y);
}
