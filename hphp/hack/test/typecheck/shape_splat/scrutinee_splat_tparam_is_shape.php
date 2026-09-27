<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// `is` against a concrete (closed) shape hint. The true branch guarantees the
// checked shape while retaining the original generic row constraint. The false
// branch stays a subtype of the original splat shape.
function f<T as shape(...)>(shape(...T, 'a' => int) $x): void {
  if ($x is shape('a' => int)) {
    hh_expect<shape('a' => int)>($x);
  } else {
    hh_expect<shape(...T, 'a' => int)>($x);
  }
}
