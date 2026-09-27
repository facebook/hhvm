<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

function f<T as shape(?'t' => bool, ...)>(
  shape(...T, 'a' => int) $x,
): void {
  if ($x is shape('a' => int)) {
    hh_expect<shape('a' => int)>($x);
  }
}
